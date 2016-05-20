#ifndef ROVER2PROCESS_H
#define ROVER2PROCESS_H

#include <QtCore>
#include <QCoreApplication>
#include <QTimerEvent>

#include "channel.h"
#include "logger.h"
#include "soro_global.h"
#include "soroini.h"
#include "socketaddress.h"
#include "videoserver.h"
#include "videoencoding.h"
#include "videoserverarray.h"

using namespace Soro;

namespace Soro {
namespace Rover {

class Rover2Process : QObject {
    Q_OBJECT

public:
    explicit Rover2Process(QObject *parent = 0);
    ~Rover2Process();

private:
    Logger *_log = NULL;

    Channel *_masterComputerChannel = NULL;
    QUdpSocket *_masterComputerBroadcastSocket = NULL;

    VideoServerArray *_videoServers = NULL;

    SoroIniLoader _config;

    int _initTimerId = TIMER_INACTIVE;
    int _broadcastTimerId = TIMER_INACTIVE;

    void beginBroadcast();

private slots:
    void masterChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void masterChannelStateChanged(Channel* channel, Channel::State state);
    void masterComputerBroadcastSocketReadyRead();
protected:
    void timerEvent(QTimerEvent *e);
};

} // namespace Rover
} // namespace Soro

#endif // ROVER2PROCESS_H
