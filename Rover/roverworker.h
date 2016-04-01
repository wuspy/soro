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
#include "serialchannel3.h"
#include "commonini.h"
#include "drivemessage.h"
#include "gimbalmessage.h"
#include "gpsserver.h"

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
        SerialChannel3 *_armControllerSerial = NULL;
        SerialChannel3 *_driveControllerSerial = NULL;
        SerialChannel3 *_gimbalControllerSerial = NULL;
        GpsServer *_gpsServer = NULL;

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

        void armControllerChannelStateChanged(SerialChannel3::State state);
        void driveControllerChannelStateChanged(SerialChannel3::State state);
        void gimbalControllerChannelStateChanged(SerialChannel3::State state);

    protected:
        void timerEvent(QTimerEvent *e);
    };

}
}

#endif // ARMNETWORKINTERFACE_H
