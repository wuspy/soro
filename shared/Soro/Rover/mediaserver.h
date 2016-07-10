#ifndef SORO_ROVER_MEDIASERVER_H
#define SORO_ROVER_MEDIASERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QProcess>

#include "socketaddress.h"
#include "soro_global.h"
#include "logger.h"
#include "channel.h"

namespace Soro {
namespace Rover {

/* Abstract class implementing the base functionality for sending a UDP
 * stream to a MediaClient.
 */
class MediaServer: public QObject {
    Q_OBJECT
public:
    enum State {
        IdleState,
        /* The server is configured for a video stream and is waiting for a client
         * to connect so it can receive the stream. No pipeline or stream will
         * be created until after a client is successfully connected.
         */
        WaitingState,

        /* The server is configured and currently streaming video to a client.
         */
        StreamingState
    };

    ~MediaServer();

    void stop();
    int getMediaId();
    MediaServer::State getState() const;

private:
    int _mediaId;
    SocketAddress _host;
    Channel *_controlChannel = NULL;
    QUdpSocket *_mediaSocket = NULL;
    State _state = IdleState;
    QProcess _child;
    QTcpServer *_ipcServer = NULL;
    QTcpSocket *_ipcSocket = NULL;
    int _startInternalTimerId = TIMER_INACTIVE;
    QString LOG_TAG;

    void beginStream(SocketAddress address);

    /* Internal state change method
     */
    void setState(MediaServer::State state);

private slots:
    void mediaSocketReadyRead();
    void controlChannelStateChanged(Channel *channel, Channel::State state);
    void beginClientHandshake();
    void childStateChanged(QProcess::ProcessState state);
    void ipcServerClientAvailable();

signals:
    void stateChanged(MediaServer *server, MediaServer::State state);
    void eos(MediaServer *server);
    void error(MediaServer *server, QString message);

protected:
    MediaServer(QString logTag, int mediaId, QString childProcessPath, SocketAddress host, QObject *parent);

    void initStream();

    virtual void onStreamStoppedInternal() = 0;
    virtual void constructChildArguments(QStringList& outArgs, SocketAddress host, SocketAddress address, quint16 ipcPort)=0;
    virtual void constructStreamingMessage(QDataStream& stream)=0;

};

} // namespace Rover
} // namespace Soro

#endif // SORO_ROVER_MEDIASERVER_H
