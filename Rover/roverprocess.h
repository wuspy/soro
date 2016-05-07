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

    /*inline StreamFormat streamFormatLowQuality() {
        StreamFormat format;
        format.Encoding = Mpeg2Encoding;
        format.Width = 480;
        format.Height = 360;
        format.Framerate = 15;
        format.Mpeg2_Bitrate = 2000000;
        return format;
    }

    inline StreamFormat streamFormatNormalQuality() {
        StreamFormat format;
        format.Encoding = Mpeg2Encoding;
        format.Width = 640;
        format.Height = 480;
        format.Framerate = 25;
        format.Mpeg2_Bitrate = 3000000;
        return format;
    }

    inline StreamFormat streamFormatHighQuality() {
        StreamFormat format;
        format.Encoding = Mpeg2Encoding;
        format.Width = 960;
        format.Height = 720;
        format.Framerate = 25;
        format.Mpeg2_Bitrate = 5000000;
        return format;
    }

    inline StreamFormat streamFormatUltraQuality() {
        StreamFormat format;
        format.Encoding = Mpeg2Encoding;
        format.Width = 1440;
        format.Height = 1080;
        format.Framerate = 30;
        format.Mpeg2_Bitrate = 10000000;
        return format;
    }*/

    inline StreamFormat streamFormatLowQuality() {
        StreamFormat format;
        format.Encoding = MjpegEncoding;
        format.Width = 480;
        format.Height = 360;
        format.Framerate = 15;
        format.Mjpeg_Quality = 30;
        return format;
    }

    inline StreamFormat streamFormatNormalQuality() {
        StreamFormat format;
        format.Encoding = MjpegEncoding;
        format.Width = 640;
        format.Height = 480;
        format.Framerate = 15;
        format.Mjpeg_Quality = 40;
        return format;
    }

    inline StreamFormat streamFormatHighQuality() {
        StreamFormat format;
        format.Encoding = MjpegEncoding;
        format.Width = 960;
        format.Height = 720;
        format.Framerate = 15;
        format.Mjpeg_Quality = 30;
        return format;
    }

    inline StreamFormat streamFormatUltraQuality() {
        StreamFormat format;
        format.Encoding = MjpegEncoding;
        format.Width = 960;
        format.Height = 720;
        format.Framerate = 20;
        format.Mjpeg_Quality = 50;
        return format;
    }


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
