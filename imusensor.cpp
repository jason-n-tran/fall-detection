#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "imusensor.h"
#include <QDebug>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cmath>
#include <chrono>

#ifdef __linux__
    #include <linux/i2c-dev.h>
#else
    #define I2C_SLAVE 0x0703
#endif

ImuSensor::ImuSensor(QObject *parent) 
    : QObject(parent), i2c_file(-1), m_running(false), m_workerThread(nullptr), m_history(50)
{
}

ImuSensor::~ImuSensor() {
    stopSensing();
    if (i2c_file != -1) ::close(i2c_file);
}

bool ImuSensor::initI2C() {
    const char *i2c_device = "/dev/i2c-1";
    i2c_file = ::open(i2c_device, O_RDWR);
    if (i2c_file < 0) return false;

    if (ioctl(i2c_file, I2C_SLAVE, LSM_ADDR) < 0) return false;

    char reg = LSM_WHO_AM_I;
    write(i2c_file, &reg, 1);
    char id;
    if (read(i2c_file, &id, 1) != 1 || id != 0x6C) return false;

    char configXL[] = {LSM_CTRL1_XL, 0x48}; 
    write(i2c_file, configXL, 2);

    char configG[] = {LSM_CTRL2_G, 0x4C}; 
    write(i2c_file, configG, 2);

    return true;
}

void ImuSensor::startSensing() {
    if (m_running) return;

    if (!initI2C()) {
        emit statusUpdated("Error: I2C Init Failed");
        return;
    }

    m_running = true;
    m_workerThread = QThread::create([this]{ processSensorLoop(); });
    m_workerThread->start();
    emit statusUpdated("Monitoring (Advanced)...");
}

void ImuSensor::stopSensing() {
    m_running = false;
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
        m_workerThread = nullptr;
    }
}

void ImuSensor::processSensorLoop() {
    while (m_running) {
        auto start = std::chrono::steady_clock::now();

        char reg = LSM_OUTX_L_G;
        write(i2c_file, &reg, 1);
        char data[12];
        if (read(i2c_file, data, 12) == 12) {
            int16_t gx_raw = (data[1] << 8) | (uint8_t)data[0];
            int16_t gy_raw = (data[3] << 8) | (uint8_t)data[2];
            int16_t gz_raw = (data[5] << 8) | (uint8_t)data[4];

            int16_t ax_raw = (data[7] << 8) | (uint8_t)data[6];
            int16_t ay_raw = (data[9] << 8) | (uint8_t)data[8];
            int16_t az_raw = (data[11] << 8) | (uint8_t)data[10];

            SensorData cur;
            cur.gx = gx_raw * 0.070 * (M_PI / 180.0); // rad/s
            cur.gy = gy_raw * 0.070 * (M_PI / 180.0);
            cur.gz = gz_raw * 0.070 * (M_PI / 180.0);
            cur.ax = ax_raw * 0.000122; // G
            cur.ay = ay_raw * 0.000122;
            cur.az = az_raw * 0.000122;
            cur.total_accel = sqrt(cur.ax*cur.ax + cur.ay*cur.ay + cur.az*cur.az);

            double accel_roll = atan2(cur.ay, cur.az);
            double accel_pitch = atan2(-cur.ax, sqrt(cur.ay*cur.ay + cur.az*cur.az));

            m_rollFilter.update(accel_roll, cur.gx, DT);
            m_pitchFilter.update(accel_pitch, cur.gy, DT);

            cur.roll = m_rollFilter.getAngle();
            cur.pitch = m_pitchFilter.getAngle();

            double gravity_z = -cur.ax * sin(cur.pitch) + 
                               cur.ay * sin(cur.roll) * cos(cur.pitch) + 
                               cur.az * cos(cur.roll) * cos(cur.pitch);
            
            double vert_accel_g = gravity_z - 1.0;
            m_vertical_vel += vert_accel_g * G * DT;
            
            m_vertical_vel *= 0.98; 
            cur.vertical_vel = m_vertical_vel;
            cur.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                start.time_since_epoch()).count();

            detectFall(cur);

            m_history.push(cur);
        }

        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        int sleepTime = 10 - elapsed;
        if (sleepTime > 0) QThread::msleep(sleepTime);
    }
}

void ImuSensor::detectFall(const SensorData &cur) {
    if (cur.total_accel < 0.4) {
        m_potentialFall = true;
    }

    if (m_potentialFall && cur.total_accel > 3.0) {
        if (m_history.count() < 50) {
            m_potentialFall = false;
            return;
        }

        SensorData past = m_history.get_past(49);
        
        bool high_downward_vel = (cur.vertical_vel < -1.5);
        
        double angle_change = sqrt(pow(cur.roll - past.roll, 2) + pow(cur.pitch - past.pitch, 2));
        bool posture_change = (std::abs(cur.pitch) > M_PI/3.0 || std::abs(cur.roll) > M_PI/3.0);

        if (posture_change && high_downward_vel) {
            emit fallDetected();
            emit statusUpdated("HARD FALL DETECTED!");
        } else if (posture_change) {
            emit statusUpdated("Stumble / Sit down detected");
        } else {
            emit statusUpdated("Impact detected, still upright");
        }
        
        m_potentialFall = false;
        m_vertical_vel = 0;
    }
}