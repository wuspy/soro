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

class VideoClient : public QObject {
    Q_OBJECT
public:
    enum State {
        ConnectingState,
        ConnectedState,
        StreamingState
    };

    explicit VideoClient(int cameraId, SocketAddress server, QHostAddress host, Logger *log = 0, QObject *parent = 0);

    ~VideoClient();

    void addForwardingAddress(SocketAddress address);
    void removeForwardingAddress(SocketAddress address);

    /* Gets the format of the stream currently being received
     */
    StreamFormat getStreamFormat() const;
    SocketAddress getServerAddress() const;
    SocketAddress getHostAddress() const;
    VideoClient::State getState() const;
    int getCameraId() const;
    QString getErrorString() const;
    quint64 getVideoBitrate() const;

signals:
    void stateChanged(VideoClient *client, VideoClient::State state);
    void nameChanged(VideoClient *client, QString name);

private:
    char *_buffer;
    int _cameraId;
    bool _needsData = true;
    SocketAddress _server;
    Logger *_log;
    State _state = ConnectingState;
    StreamFormat _format;
    QUdpSocket *_videoSocket;
    Channel *_controlChannel;
    int _punchTimerId = TIMER_INACTIVE;
    int _calculateBitrateTimerId = TIMER_INACTIVE;
    QList<SocketAddress> _forwardAddresses;
    long _bitCount = 0;
    int _lastBitrate = 0;
    QString _errorString = "";

    void setState(State state);
    void setCameraName(QString name);

private slots:
    void controlMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void videoSocketReadyRead();
    void controlChannelStateChanged(Channel *channel, Channel::State state);

protected:
    void timerEvent(QTimerEvent *e);
};

} // namespace MissionControl
} // namespace Soro

#endif // VIDEOCLIENT_H
