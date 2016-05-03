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
#include "flycapsource.h"

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
    const MjpegStreamFormat* STREAMFORMAT_720_MJPEG_Q30 = new MjpegStreamFormat(1080, 720, 0, 30);

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

    // these hold the gst elements for the cameras that are flycapture sources
    FlycapSource *_armFlycaptureSource = NULL;
    FlycapSource *_driveFlycaptureSource = NULL;
    FlycapSource *_gimbalFlycaptureSource = NULL;
    FlycapSource *_fisheyeFlycaptureSource = NULL;

    // these hold the gst elements for the cameras that are V4L2 sources
    QGst::ElementPtr _armV4L2Source;
    QGst::ElementPtr _driveV4L2Source;
    QGst::ElementPtr _gimbalV4L2Source;
    QGst::ElementPtr _fisheyeV4L2Source;

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
