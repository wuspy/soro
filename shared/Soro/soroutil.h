#ifndef SOROUTIL_H
#define SOROUTIL_H

#include <QSerialPortInfo>
#include <QSerialPort>

//QObject timer macros to make shit easier
#define TIMER_INACTIVE -1
#define START_TIMER(X,Y) if (X == TIMER_INACTIVE) X = startTimer(Y)
#define KILL_TIMER(X) if (X != TIMER_INACTIVE) { killTimer(X); X = TIMER_INACTIVE; }

//shared channel names, must be the same on the rover and mission control builds
#define CHANNEL_NAME_ARM "Soro_ArmChannel"
#define CHANNEL_NAME_DRIVE "Soro_DriveChannel"
#define CHANNEL_NAME_GIMBAL "Soro_GimbalChannel"
#define CHANNEL_NAME_SHARED "Soro_SharedTcpChannel"

namespace Soro {

static inline signed char joyAxisToByte(float val) {
    return (signed char) (val * 100);
}

static void configureMbedSerial(QSerialPort* port) {
    port->setBaudRate(QSerialPort::Baud115200);
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
}

}

#endif // SOROUTIL_H
