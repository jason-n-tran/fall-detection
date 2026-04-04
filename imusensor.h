#ifndef IMUSENSOR_H
#define IMUSENSOR_H

#include <QObject>
#include <QThread>
#include <QTimer>

class ImuSensor : public QObject
{
    Q_OBJECT
public:
    explicit ImuSensor(QObject *parent = nullptr);
    ~ImuSensor();

public slots:
    void startSensing();

signals:
    void fallDetected();
    void statusUpdated(const QString &status);

private slots:
    void readSensorData();

private:
    int i2c_file;
    QTimer *m_timer;
    bool potentialFreefall = false;
    const double FREEFALL_THRESHOLD = 0.5;
    const double IMPACT_THRESHOLD = 3.0;
};

#endif