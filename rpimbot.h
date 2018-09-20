#ifndef RPIMBOT_H
#define RPIMBOT_H

#include <QObject>
#include <QProcess>
#include <QSerialPort>
#include <QDebug>
#include <QThread>

#define GPIO_BCM_TILT   12
#define GPIO_BCM_PAN    13
#define ANGLE_STEP      20

class Rpimbot : public QObject
{
    Q_OBJECT
public:
    explicit Rpimbot(QObject *parent = nullptr);
    void init();

signals:
    void servoCommanderReadyMsg(QByteArray msg);

public slots:

private:
    QProcess *servoController;
    QProcess *camStreamer;
    QSerialPort *mbotMessenger;
    uint angleTilt, anglePan;
    QByteArray baCmdMsg;

private slots:

    void onProcessErrorStream(QProcess::ProcessError err);
    void onProcessErrorServo(QProcess::ProcessError err);
    void onSerialReadyRead();
    void onServoCommanderReadyMsg(QByteArray cmd);

};

#endif // RPIMBOT_H
