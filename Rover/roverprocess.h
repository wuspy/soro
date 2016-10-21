#ifndef SORO_ROVER_ROVERPROCESS_H
#define SORO_ROVER_ROVERPROCESS_H

#include <QtCore>
#include <QCoreApplication>
#include <QTimerEvent>

#include "channel.h"
#include "logger.h"
#include "armmessage.h"
#include "soro_global.h"
#include "mbedchannel.h"
#include "roverconfigloader.h"
#include "drivemessage.h"
#include "gimbalmessage.h"
#include "socketaddress.h"
#include "videoserver.h"
#include "videoserverarray.h"
#include "audioserver.h"
#include "gpsserver.h"

using namespace Soro;

namespace Soro {
namespace Rover {

class RoverProcess : QObject {
    Q_OBJECT

public:
    explicit RoverProcess(QObject *parent = 0);
    ~RoverProcess();

private:

    Channel *_armChannel = NULL;
    Channel *_driveChannel = NULL;
    Channel *_gimbalChannel = NULL;
    Channel *_sharedChannel = NULL;
    Channel *_secondaryComputerChannel = NULL;

    QUdpSocket *_secondaryComputerBroadcastSocket = NULL;

    MbedChannel *_armControllerMbed = NULL;
    MbedChannel *_driveGimbalControllerMbed = NULL;

    VideoServerArray *_videoServers = NULL;

    AudioServer *_audioServer = NULL;

    // These hold the current stream formats for each camera.
    // If a camera currently isn't being streamed, the format will have an
    // encoding value of UnknownEncoding.
    GpsServer *_gpsServer = NULL;

    RoverConfigLoader *_config;

    int _initTimerId = TIMER_INACTIVE;

private slots:
    void init();

    // slots for received network messages
    void armChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void driveChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void gimbalChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void sharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void sharedChannelStateChanged(Channel *channel, Channel::State state);

    void mbedChannelStateChanged(MbedChannel *channel, MbedChannel::State state);

    void secondaryComputerBroadcastSocketReadyRead();
    void secondaryComputerBroadcastSocketError(QAbstractSocket::SocketError err);
    void secondaryComputerStateChanged(Channel *channel, Channel::State state);
    void beginSecondaryComputerListening();

    void sendSystemStatusMessage();

    void videoServerError(int cameraId, QString message);

    void gpsUpdate(NmeaMessage message);

};

} // namespace Rover
} // namespace Soro

#endif // SORO_ROVER_ROVERPROCESS_H
