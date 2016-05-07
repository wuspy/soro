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

class VideoClient : public QObject, public QGst::Utils::ApplicationSource {
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
    SocketAddress getServerAddress() const;
    SocketAddress getHostAddress() const;
    VideoClient::State getState() const;
    QString getCameraName() const;

signals:
    void stateChanged(VideoClient *client, VideoClient::State state);
    void serverError(VideoClient *client, QString message);
    void statisticsUpdate(VideoClient *client, long bitrate);
    void nameChanged(VideoClient *client, QString name);

private:
    char *_buffer;
    QString _name = "";
    bool _needsData = false;
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

    void setState(State state);
    void setCameraName(QString name);

private slots:
    void controlMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void videoSocketReadyRead();
    void controlChannelStateChanged(Channel *channel, Channel::State state);

protected:
    void timerEvent(QTimerEvent *e);

    /*! Called when the appsrc needs more data. A buffer or EOS should be pushed
     * to appsrc from this thread or another thread. length is just a hint and when
     * it is set to -1, any number of bytes can be pushed into appsrc. */
    void needData(uint length) Q_DECL_OVERRIDE;

    /*! Called when appsrc has enough data. It is recommended that the application
     * stops calling pushBuffer() until the needData() method is called again to
     * avoid excessive buffer queueing. */
    void enoughData() Q_DECL_OVERRIDE;
};

} // namespace MissionControl
} // namespace Soro

#endif // VIDEOCLIENT_H
