#ifndef ARMNETWORKINTERFACE_H
#define ARMNETWORKINTERFACE_H

#include <QtCore>
#include <QCoreApplication>
#include <QSerialPort>
#include <QTimerEvent>

#include "channel.h"
#include "logger.h"
#include "armmessage.h"
#include "soroutil.h"
#include "serialinterop.h"
#include "commonini.h"

#include <stdio.h>

using namespace Soro;

namespace Soro {
namespace Rover {

    class RoverWorker : QObject {
        Q_OBJECT

    public:
        explicit RoverWorker(QObject *parent = 0);
        ~RoverWorker();

    private:
        Logger *_log = NULL;
        Channel *_armChannel = NULL;
        Channel *_driveChannel = NULL;
        Channel *_gimbalChannel = NULL;
        Channel *_sharedChannel = NULL;
        SerialChannel *_armControllerSerial = NULL;
        SerialChannel *_driveControllerSerial = NULL;
        SerialChannel *_gimbalControllerSerial = NULL;

        SoroIniConfig _soroIniConfig;

        int _initTimerId = TIMER_INACTIVE;

    private slots:
        void armChannelMessageReceived(const QByteArray &message);
        void driveChannelMessageReceived(const QByteArray &message);
        void gimbalChannelMessageReceived(const QByteArray &message);
        void sharedChannelMessageReceived(const QByteArray &message);

        void armChannelStateChanged(Channel::State state);
        void driveChannelStateChanged(Channel::State state);
        void gimbalChannelStateChanged(Channel::State state);
        void sharedChannelStateChanged(Channel::State state);

        void armControllerMessageReceived(const char *message, int size);
        void driveControllerMessageReceived(const char *message, int size);
        void gimbalControllerMessageReceived(const char *message, int size);

        void armControllerChannelStateChanged(SerialChannel::State state);
        void driveControllerChannelStateChanged(SerialChannel::State state);
        void gimbalControllerChannelStateChanged(SerialChannel::State state);

    protected:
        void timerEvent(QTimerEvent *e);
    };

}
}

#endif // ARMNETWORKINTERFACE_H
