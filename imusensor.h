#ifndef IMUSENSOR_H
#define IMUSENSOR_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <atomic>
#include "orientation_filter.h"
#include "circular_buffer.h"
#include "gait_analyzer.h"

struct SensorData {
    double ax, ay, az;
    double gx, gy, gz;
    double roll, pitch;
    double total_accel;
    double vertical_vel;
    long long timestamp;
};

class ImuSensor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double pitch READ pitch NOTIFY pitchChanged)
    Q_PROPERTY(double roll READ roll NOTIFY rollChanged)
    Q_PROPERTY(double totalAccel READ totalAccel NOTIFY sensorUpdated)
    Q_PROPERTY(double riskScore READ riskScore NOTIFY riskScoreChanged)

public:
    explicit ImuSensor(QObject *parent = nullptr);
    ~ImuSensor();

    double pitch() const { return m_pitch; }
    double roll() const { return m_roll; }
    double totalAccel() const { return m_totalAccel; }
    double riskScore() const { return m_riskScore; }

public slots:
    void startSensing();
    void stopSensing();

signals:
    void fallDetected();
    void fallRiskWarning(double riskScore);
    void statusUpdated(const QString &status);
    void pitchChanged();
    void rollChanged();
    void riskScoreChanged();
    void sensorUpdated();

private slots:
    void processSensorLoop();

private:
    int i2c_file;
    std::atomic<bool> m_running;
    QThread *m_workerThread;

    OrientationFilter m_rollFilter;
    OrientationFilter m_pitchFilter;
    CircularBuffer<SensorData> m_history;
    GaitAnalyzer m_gaitAnalyzer;
    std::vector<double> m_gaitWindow;
    int m_analysisCounter = 0;

    double m_pitch = 0.0;
    double m_roll = 0.0;
    double m_totalAccel = 1.0;
    double m_riskScore = 0.0;
    double m_vertical_vel = 0.0;
    bool m_potentialFall = false;
    
    const double G = 9.80665;
    const int SAMPLE_RATE_HZ = 100;
    const double DT = 1.0 / SAMPLE_RATE_HZ;

    static constexpr int LSM_ADDR = 0x6A;
    static constexpr int LSM_WHO_AM_I = 0x0F;
    static constexpr int LSM_CTRL1_XL = 0x10;
    static constexpr int LSM_CTRL2_G = 0x11;
    static constexpr int LSM_OUTX_L_G = 0x22;
    static constexpr int LSM_OUTX_L_A = 0x28;

    bool initI2C();
    void detectFall(const SensorData &current);
};

#endif