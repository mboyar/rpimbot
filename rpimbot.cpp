#include "rpimbot.h"

Rpimbot::Rpimbot(QObject *parent) : QObject(parent)
{
    this->servoController = new QProcess(this);
    this->camStreamer = new QProcess(this);
    this->mbotMessenger = new QSerialPort(this);

    connect(camStreamer, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onProcessErrorStream(QProcess::ProcessError)));
    connect(servoController, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onProcessErrorServo(QProcess::ProcessError)));
    connect(mbotMessenger, SIGNAL(readyRead()), this, SLOT(onSerialReadyRead()));
    connect(this, SIGNAL(servoCommanderReadyMsg(QByteArray)), this, SLOT(onServoCommanderReadyMsg(QByteArray)));
}

void Rpimbot::init()
{
    /*
     * Thx to http://www.raspberry-projects.com/pi/pi-hardware/raspberry-pi-camera/streaming-video-using-vlc-player
     * The correction at https://stackoverflow.com/questions/45969873/trying-to-stream-raspberry-pi-camera-with-vlc-cannot-get-vlc-on-windows-to-play
     */
    camStreamer->start("bash", QStringList() << "-c" << \
                       "raspivid -n -ih -t 0 -b 1000000 -fps 30 -o - | " \
                       "cvlc -vvv stream:///dev/stdin --sout '#rtp{sdp=rtsp://:8554/cam1}' :demux=h264");

    /*
     * Thx to https://learn.adafruit.com/adafruits-raspberry-pi-lesson-8-using-a-servo-motor/software
     */
    servoController->start("gpio -g mode 12 pwm"); //will be configurable
    servoController->waitForFinished();
    servoController->start("gpio -g mode 13 pwm");
    servoController->waitForFinished();
    servoController->start("gpio pwm-ms");
    servoController->waitForFinished();
    servoController->start("gpio pwmc 192");
    servoController->waitForFinished();
    servoController->start("gpio pwmr 2000");
    servoController->waitForFinished();

    angleTilt = 50;
    anglePan = 50;

    /*
     *
     */
    mbotMessenger->setPortName("/dev/ttyS0");
    mbotMessenger->setBaudRate(QSerialPort::Baud9600);
    if(mbotMessenger->open(QIODevice::ReadOnly) == false){

        qDebug() << __FUNCTION__ << "mbotMessenger->open(QIODevice::ReadOnly)" << mbotMessenger->errorString();
    }
}

void Rpimbot::onServoCommanderReadyMsg(QByteArray msg)
{
    QString cmd1, cmd2, cmdBase("gpio -g pwm");
    bool flag = true;

    QString gpioBcmNoTilt = QString::number(GPIO_BCM_TILT);
    QString gpioBcmNoPan = QString::number(GPIO_BCM_PAN);

    if(msg.contains("tilt-up")){

        angleTilt += ANGLE_STEP;
        if(angleTilt >= 250) angleTilt = 250;

        cmd1 = QString("%1 %2 %3").arg(cmdBase).arg(gpioBcmNoTilt).arg(QString::number(angleTilt));
        cmd2 = QString("%1 %2 0").arg(cmdBase).arg(gpioBcmNoTilt);
    }
    else if(msg.contains("tilt-down")){

        angleTilt -= ANGLE_STEP;
        if(angleTilt <= 50) angleTilt = 50;

        cmd1 = QString("%1 %2 %3").arg(cmdBase).arg(gpioBcmNoTilt).arg(QString::number(angleTilt));
        cmd2 = QString("%1 %2 0").arg(cmdBase).arg(gpioBcmNoTilt);
    }
    else if(msg.contains("pan-right")){

        anglePan += ANGLE_STEP;
        if(anglePan >= 250) anglePan = 250;

        cmd1 = QString("%1 %2 %3").arg(cmdBase).arg(gpioBcmNoPan).arg(QString::number(anglePan));
        cmd2 = QString("%1 %2 0").arg(cmdBase).arg(gpioBcmNoPan);
    }
    else if(msg.contains("pan-left")){

        anglePan -= ANGLE_STEP;
        if(anglePan <= 50) anglePan = 50;

        cmd1 = QString("%1 %2 %3").arg(cmdBase).arg(gpioBcmNoPan).arg(QString::number(anglePan));
        cmd2 = QString("%1 %2 0").arg(cmdBase).arg(gpioBcmNoPan);
    }
    else{

        flag = false;
        qDebug() << "Command format error!";
    }

    if(flag){

        servoController->start(cmd1);
        servoController->waitForFinished();

        QThread::sleep(1); //need due to "gpio" command's behavior

        servoController->start(cmd2);
        servoController->waitForFinished();
    }

    qDebug() << __FUNCTION__ << "msg" << msg;
    qDebug() << __FUNCTION__ << "cmd1" << cmd1;
    qDebug() << __FUNCTION__ << "cmd2" << cmd2;
}

void Rpimbot::onProcessErrorStream(QProcess::ProcessError err)
{
    qDebug() << __FUNCTION__ << "QProcess::ProcessError" << err;
}

void Rpimbot::onProcessErrorServo(QProcess::ProcessError err)
{
    qDebug() << __FUNCTION__ << "QProcess::ProcessError" << err;
}

void Rpimbot::onSerialReadyRead()
{
    baCmdMsg.append(mbotMessenger->readAll());

    if(baCmdMsg.contains('\n')){

        emit servoCommanderReadyMsg(baCmdMsg);
        baCmdMsg.clear();
    }
}


