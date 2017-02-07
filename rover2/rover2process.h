#ifndef SORO_ROVER_ROVER2PROCESS_H
#define SORO_ROVER_ROVER2PROCESS_H

#include <QtCore>
#include <QCoreApplication>
#include <QTimerEvent>

#include "libsoro/channel.h"
#include "libsoro/logger.h"
#include "libsoro/constants.h"
#include "libsoro/socketaddress.h"
#include "libsoro/videoserver.h"
#include "libsoro/videoserverarray.h"
#include "libsoro/roverconfigloader.h"
#include "libsoro/videoformat.h"
#include "libsoro/enums.h"

using namespace Soro;

namespace Soro {
namespace Rover {

class Rover2Process : QObject {
    Q_OBJECT

public:
    explicit Rover2Process(QObject *parent = 0);
    ~Rover2Process();

private:
    Channel *_masterComputerChannel = NULL;
    QUdpSocket *_masterComputerBroadcastSocket = NULL;

    VideoServerArray *_videoServers = NULL;

    RoverConfigLoader _config;

    int _initTimerId = TIMER_INACTIVE;
    int _broadcastTimerId = TIMER_INACTIVE;

    void beginBroadcast();

private slots:
    void init();
    void masterChannelMessageReceived(const char *message, Channel::MessageSize size);
    void masterChannelStateChanged(Channel::State state);
    void masterComputerBroadcastSocketReadyRead();
    void mediaServerError(MediaServer *server, QString error);

protected:
    void timerEvent(QTimerEvent *e);
};

} // namespace Rover
} // namespace Soro

#endif // SORO_ROVER_ROVER2PROCESS_H
