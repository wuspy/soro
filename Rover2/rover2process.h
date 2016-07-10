#ifndef SORO_ROVER_ROVER2PROCESS_H
#define SORO_ROVER_ROVER2PROCESS_H

#include <QtCore>
#include <QCoreApplication>
#include <QTimerEvent>

#include "channel.h"
#include "logger.h"
#include "soro_global.h"
#include "configuration.h"
#include "socketaddress.h"
#include "videoserver.h"
#include "videoserverarray.h"

using namespace Soro;

namespace Soro {
namespace Rover {

class Rover2Process : QObject {
    Q_OBJECT

public:
    explicit Rover2Process(const Configuration *config, QObject *parent = 0);
    ~Rover2Process();

private:
    Channel *_masterComputerChannel = NULL;
    QUdpSocket *_masterComputerBroadcastSocket = NULL;

    VideoServerArray *_videoServers = NULL;

    const Configuration *_config;

    int _initTimerId = TIMER_INACTIVE;
    int _broadcastTimerId = TIMER_INACTIVE;

    void beginBroadcast();

private slots:
    void init();
    void masterChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void masterChannelStateChanged(Channel* channel, Channel::State state);
    void masterComputerBroadcastSocketReadyRead();

protected:
    void timerEvent(QTimerEvent *e);
};

} // namespace Rover
} // namespace Soro

#endif // SORO_ROVER_ROVER2PROCESS_H
