#ifndef ARMNETWORKINTERFACE_H
#define ARMNETWORKINTERFACE_H

#include <QtCore>
#include <QCoreApplication>
#include <QSerialPort>
#include <QTimerEvent>

#include "channel.h"
#include "logger.h"
#include "armmessage.h"
#include "soro_global.h"
#include "mbedchannel.h"
#include "soroini.h"
#include "drivemessage.h"
#include "gimbalmessage.h"
#include "socketaddress.h"
#include "gpsserver.h"

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
        MbedChannel *_armControllerMbed = NULL;
        MbedChannel *_driveControllerMbed = NULL;
        MbedChannel *_gimbalControllerMbed = NULL;
        GpsServer *_gpsServer = NULL;

        SoroIniLoader _soroIniConfig;

        int _initTimerId = TIMER_INACTIVE;

    private slots:
        void armChannelMessageReceived(const char *message, Channel::MessageSize size);
        void driveChannelMessageReceived(const char *message, Channel::MessageSize size);
        void gimbalChannelMessageReceived(const char *message, Channel::MessageSize size);
        void sharedChannelMessageReceived(const char *message, Channel::MessageSize size);

        void armChannelStateChanged(Channel::State state);
        void driveChannelStateChanged(Channel::State state);
        void gimbalChannelStateChanged(Channel::State state);
        void sharedChannelStateChanged(Channel::State state);

    protected:
        void timerEvent(QTimerEvent *e);
    };

}
}

#endif // ARMNETWORKINTERFACE_H
