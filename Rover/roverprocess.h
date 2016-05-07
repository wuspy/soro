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
#include "flycapenumerator.h"
#include "uvdcameraenumerator.h"

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

    MbedChannel *_armControllerMbed = NULL;
    MbedChannel *_driveGimbalControllerMbed = NULL;

    // These hold the gst elements for the cameras that are flycapture sources
    QMap<int, FlyCapture2::PGRGuid> _flycapCameras; // camera ID is by key

    // These hold the gst elements for the cameras that not flycapture
    QMap<int, QString> _uvdCameras; // camera ID is by key

    // These hold the video server objects which spawn child processes to
    // stream the data to mission control
    QList<VideoServer*> _videoServers; // camera ID is by index

    // These hold the current stream formats for each camera.
    // If a camera currently isn't being streamed, the format will have an
    // encoding value of UnknownEncoding.
    GpsServer *_gpsServer = NULL;

    SoroIniLoader _soroIniConfig;

    int _initTimerId = TIMER_INACTIVE;

    void sendSystemStatusMessage();

private slots:
    void armChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void driveChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void gimbalChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void sharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void sharedChannelStateChanged(Channel *channel, Channel::State state);

    void mbedChannelStateChanged(MbedChannel *channel, MbedChannel::State state);

protected:
    void timerEvent(QTimerEvent *e);
};

} // namespace Rover
} // namespace Soro

#endif // ARMNETWORKINTERFACE_H
