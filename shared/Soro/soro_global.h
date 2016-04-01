#ifndef SORO_GLOBAL_H
#define SORO_GLOBAL_H

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

#ifdef QT_WIDGETS_LIB

#include <QWidget>
#include <QGraphicsDropShadowEffect>

static inline void addWidgetShadow(QWidget *target, int radius, int offset) {
    QGraphicsDropShadowEffect* ef = new QGraphicsDropShadowEffect;
    ef->setBlurRadius(radius);
    ef->setOffset(offset);
    target->setGraphicsEffect(ef);
}

#endif

#endif // SORO_GLOBAL_H
