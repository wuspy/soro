#ifndef VIDEOCLIENT_H
#define VIDEOCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QList>

#include "soro_global.h"
#include "channel.h"
#include "socketaddress.h"
#include "videoencoding.h"

namespace Soro {
namespace MissionControl {

class VideoClient : public QObject {
    Q_OBJECT
public:
    enum State {
        ConnectingState,
        ConnectedState,
        StreamingState
    };

    explicit VideoClient(QString name, SocketAddress server, QHostAddress host, Logger *log = 0, QObject *parent = 0);

    void addForwardingAddress(SocketAddress address);
    void removeForwardingAddress(SocketAddress address);

    /* Gets the format of the stream currently being received
     */
    VideoEncoding getEncoding();

    VideoClient::State getState();

signals:
    void stateChanged(VideoClient::State state);
    void serverEos();
    void serverError();
    void statisticsUpdate(long bitrate);
    void stopped();

private:
    char *_buffer;
    QString _name;
    SocketAddress _server;
    Logger *_log;
    State _state = ConnectingState;
    VideoEncoding _encoding = UNKNOWN;
    QUdpSocket *_videoSocket;
    Channel *_controlChannel;
    int _punchTimerId = TIMER_INACTIVE;
    int _calculateBitrateTimerId = TIMER_INACTIVE;
    QList<SocketAddress> _forwardAddresses;
    long _bitCount = 0;
    int _lastBitrate = 0;

    void setState(State state);

private slots:
    void controlMessageReceived(const char *message, Channel::MessageSize size);
    void videoSocketReadyRead();
    void controlChannelStateChanged(Channel::State state);

protected:
    void timerEvent(QTimerEvent *e);
};

} // namespace MissionControl
} // namespace Soro

#endif // VIDEOCLIENT_H
