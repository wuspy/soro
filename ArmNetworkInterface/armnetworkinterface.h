#ifndef ARMNETWORKINTERFACE_H
#define ARMNETWORKINTERFACE_H

#include <QtCore/qglobal.h>
#include <QCoreApplication>
#include <QSerialPort>
#include <QTimerEvent>

#include "soro.h"

class ArmNetworkInterface : QObject
{
    Q_OBJECT

public:
    explicit ArmNetworkInterface(QObject *parent = 0);
    ~ArmNetworkInterface();

private:
    Logger *_log;
    Channel *_armChannel;
    QSerialPort *_serial;

private slots:
    void channelMessageReceived(Channel *channel, const QByteArray &message);
};

#endif // ARMNETWORKINTERFACE_H
