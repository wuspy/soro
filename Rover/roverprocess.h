#ifndef ARMNETWORKINTERFACE_H
#define ARMNETWORKINTERFACE_H

#include <QtCore>
#include <QCoreApplication>
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
#include "videoserver.h"
#include "videoencoding.h"

#include <Qt5GStreamer/QGst/Element>
#include <Qt5GStreamer/QGst/ElementFactory>

using namespace Soro;

namespace Soro {
namespace Rover {

class RoverProcess : QObject {
    Q_OBJECT

public:
    explicit RoverProcess(QObject *parent = 0);
    ~RoverProcess();

private:
    Logger *_log = NULL;
    Channel *_armChannel = NULL;
    Channel *_driveChannel = NULL;
    Channel *_gimbalChannel = NULL;
    Channel *_sharedChannel = NULL;
    VideoServer *_video1Server = NULL;
    VideoServer *_video2Server = NULL;

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

protected:
    void timerEvent(QTimerEvent *e);
};

} // namespace Rover
} // namespace Soro

#endif // ARMNETWORKINTERFACE_H
