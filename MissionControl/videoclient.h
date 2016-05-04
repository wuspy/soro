#ifndef VIDEOCLIENT_H
#define VIDEOCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QDataStream>
#include <QByteArray>
#include <QList>

#include <Qt5GStreamer/QGst/Buffer>
#include <Qt5GStreamer/QGst/Caps>
#include <Qt5GStreamer/QGst/Utils/ApplicationSource>

#include "soro_global.h"
#include "channel.h"
#include "socketaddress.h"
#include "videoencoding.h"

namespace Soro {
namespace MissionControl {

class VideoClientSource : public QGst::Utils::ApplicationSource {
    friend class VideoClient;
};

class VideoClient : public QObject {
    Q_OBJECT
public:
    enum State {
        ConnectingState,
        ConnectedState,
        StreamingState
    };

    explicit VideoClient(QString name, SocketAddress server, QHostAddress host, Logger *log = 0, QObject *parent = 0);

    ~VideoClient();

    void addForwardingAddress(SocketAddress address);
    void removeForwardingAddress(SocketAddress address);

    /* Gets the format of the stream currently being received
     */
    StreamFormat getStreamFormat() const;
    VideoClient::State getState() const;
    QGst::ElementPtr createSource();
    void clearSource();

signals:
    void stateChanged(VideoClient::State state);
    void serverError(QString message);
    void statisticsUpdate(long bitrate);

private:
    char *_buffer;
    QString _name;
    SocketAddress _server;
    Logger *_log;
    State _state = ConnectingState;
    StreamFormat _format;
    QUdpSocket *_videoSocket;
    Channel *_controlChannel;
    VideoClientSource *_source = NULL;
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
