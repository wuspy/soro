#ifndef SORO_ROVER_ROVERPROCESS_H
#define SORO_ROVER_ROVERPROCESS_H

#include <QtCore>
#include <QCoreApplication>
#include <QTimerEvent>

#include "libsoro/channel.h"
#include "libsoro/logger.h"
#include "libsoro/constants.h"
#include "libsoro/armmessage.h"
#include "libsoro/mbedchannel.h"
#include "libsoro/roverconfigloader.h"
#include "libsoro/drivemessage.h"
#include "libsoro/gimbalmessage.h"
#include "libsoro/socketaddress.h"
#include "libsoro/videoserver.h"
#include "libsoro/videoserverarray.h"
#include "libsoro/audioserver.h"
#include "libsoro/gpsserver.h"
#include "libsoro/videoformat.h"
#include "libsoro/enums.h"

namespace Soro {
namespace Rover {

class RoverProcess : public QObject {
    Q_OBJECT

public:
    explicit RoverProcess(QObject *parent = 0);
    ~RoverProcess();

private:

    Channel *_armChannel = nullptr;
    Channel *_driveChannel = nullptr;
    Channel *_gimbalChannel = nullptr;
    Channel *_sharedChannel = nullptr;
    Channel *_secondaryComputerChannel = nullptr;

    QUdpSocket *_secondaryComputerBroadcastSocket = nullptr;

    MbedChannel *_armControllerMbed = nullptr;
    MbedChannel *_driveGimbalControllerMbed = nullptr;

    VideoServerArray *_videoServers = nullptr;

    AudioServer *_audioServer = nullptr;

    // These hold the current stream formats for each camera.
    // If a camera currently isn't being streamed, the format will have an
    // encoding value of UnknownEncoding.
    GpsServer *_gpsServer = nullptr;

    RoverConfigLoader _config;

    int _initTimerId = TIMER_INACTIVE;

private slots:
    void init();

    // slots for received network messages
    void armChannelMessageReceived(const char *message, Channel::MessageSize size);
    void driveChannelMessageReceived(const char *message, Channel::MessageSize size);
    void gimbalChannelMessageReceived( const char *message, Channel::MessageSize size);
    void sharedChannelMessageReceived(const char *message, Channel::MessageSize size);
    void sharedChannelStateChanged(Channel::State state);

    void mbedChannelStateChanged(MbedChannel::State state);

    void secondaryComputerBroadcastSocketReadyRead();
    void secondaryComputerBroadcastSocketError(QAbstractSocket::SocketError err);
    void secondaryComputerStateChanged(Channel::State state);
    void beginSecondaryComputerListening();

    void sendSystemStatusMessage();

    void mediaServerError(MediaServer *server, QString message);

    void gpsUpdate(NmeaMessage message);

};

} // namespace Rover
} // namespace Soro

#endif // SORO_ROVER_ROVERPROCESS_H
