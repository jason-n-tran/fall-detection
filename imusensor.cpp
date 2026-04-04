#include "imusensor.h"
#include <QDebug>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cmath>

#ifdef __linux__
    #include <linux/i2c-dev.h>
#else
    #define I2C_SLAVE 0x0703
#endif

// --- LSM6DSO32 Registers ---
#define LSM_ADDR          0x6A
#define LSM_WHO_AM_I      0x0F
#define LSM_CTRL1_XL      0x10
#define LSM_OUTX_L_A      0x28

ImuSensor::ImuSensor(QObject *parent) : QObject(parent), i2c_file(-1) {}

ImuSensor::~ImuSensor() {
    if (i2c_file != -1) close(i2c_file);
}

void ImuSensor::startSensing() {
    const char *i2c_device = "/dev/i2c-1";
    i2c_file = open(i2c_device, O_RDWR);
    if (i2c_file < 0) {
        emit statusUpdated("Error: Cannot open I2C bus");
        return;
    }

    if (ioctl(i2c_file, I2C_SLAVE, LSM_ADDR) < 0) {
        emit statusUpdated("Error: Cannot connect to Sensor Address");
        return;
    }

    char whoAmI_Reg = LSM_WHO_AM_I;
    write(i2c_file, &whoAmI_Reg, 1);
    char whoAmI_Val;
    if (read(i2c_file, &whoAmI_Val, 1) != 1 || whoAmI_Val != 0x6C) {
        emit statusUpdated("Error: Wrong Sensor ID (Expect 0x6C)");
        qDebug() << "Read ID:" << Qt::hex << (int)whoAmI_Val;
        return;
    }

    char config[] = {LSM_CTRL1_XL, 0x38}; 
    write(i2c_file, config, 2);
    
    emit statusUpdated("Monitoring (LSM6DSO32)...");

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ImuSensor::readSensorData);
    m_timer->start(50);
}

void ImuSensor::readSensorData() {
    if (i2c_file < 0) return;

    char reg = LSM_OUTX_L_A;
    if (write(i2c_file, &reg, 1) != 1) return;

    char data[6];
    if (read(i2c_file, data, 6) != 6) return;

    int16_t ax_raw = (data[1] << 8) | (uint8_t)data[0];
    int16_t ay_raw = (data[3] << 8) | (uint8_t)data[2];
    int16_t az_raw = (data[5] << 8) | (uint8_t)data[4];

    const double scale = 0.000122; 

    double ax = ax_raw * scale;
    double ay = ay_raw * scale;
    double az = az_raw * scale;
    
    double total_accel = sqrt(ax*ax + ay*ay + az*az);

    if (potentialFreefall) {
        if (total_accel > 2.5) {
            emit fallDetected();
            emit statusUpdated("FALL DETECTED!");
            potentialFreefall = false;
        }
    } else {
        if (total_accel < 0.3) {
            potentialFreefall = true;
            qDebug() << "Freefall detected...";
        }
    }
}