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
#include "flycapcamera.h"
#include "uvdcameraenumerator.h"

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

    inline StreamFormat streamFormat_Mjpeg_960x720_15FPS_Q50() {
        StreamFormat format;
        format.Encoding = MjpegEncoding;
        format.Width = 960;
        format.Height = 720;
        format.Framerate = 15;
        format.Mjpeg_Quality = 50;
        return format;
    }

    inline StreamFormat streamFormat_Mjpeg_Original_15FPS_Q30() {
        StreamFormat format;
        format.Encoding = MjpegEncoding;
        format.Width = 0;
        format.Height = 0;
        format.Framerate = 15;
        format.Mjpeg_Quality = 50;
        return format;
    }

    Logger *_log = NULL;

    Channel *_armChannel = NULL;
    Channel *_driveChannel = NULL;
    Channel *_gimbalChannel = NULL;
    Channel *_sharedChannel = NULL;

    VideoServer *_armVideoServer = NULL;
    VideoServer *_driveVideoServer = NULL;
    VideoServer *_fisheyeVideoServer = NULL;
    VideoServer *_gimbalVideoServer = NULL;

    MbedChannel *_armControllerMbed = NULL;
    MbedChannel *_driveControllerMbed = NULL;
    MbedChannel *_gimbalControllerMbed = NULL;

    // These hold the gst elements for the cameras that are flycapture sources
    QMap<int, FlycapCamera*> _flycapCameras;

    // These hold the gst elements for the cameras that not flycapture
    QMap<int, QGst::ElementPtr> _uvdCameras;

    QMap<int, VideoServer*> _videoServers;

    QMap<int, StreamFormat> _videoFormats;

    // These hold the current stream formats for each camera.
    // If a camera currently isn't being streamed, the format will have an
    // encoding value of UnknownEncoding.
    GpsServer *_gpsServer = NULL;

    SoroIniLoader _soroIniConfig;

    int _initTimerId = TIMER_INACTIVE;

    /* Trys to reconfigure the current streams to match what they
     * are configured to be. This should be called any time there is
     * a configuration change or a stream error.
     */
    void syncVideoStreams();

private slots:
    void armChannelMessageReceived(const char *message, Channel::MessageSize size);
    void driveChannelMessageReceived(const char *message, Channel::MessageSize size);
    void gimbalChannelMessageReceived(const char *message, Channel::MessageSize size);
    void sharedChannelMessageReceived(const char *message, Channel::MessageSize size);

    void videoServerStateChanged(VideoServer::State state);

protected:
    void timerEvent(QTimerEvent *e);
};

} // namespace Rover
} // namespace Soro

#endif // ARMNETWORKINTERFACE_H
