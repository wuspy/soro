#ifndef VIDEOSERVER_H
#define VIDEOSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QProcess>

#include "socketaddress.h"
#include "soro_global.h"
#include "videoencoding.h"
#include "logger.h"
#include "channel.h"

#include <flycapture/FlyCapture2.h>

namespace Soro {
namespace Rover {

class VideoServer: public QObject {
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

    explicit VideoServer(QString name, SocketAddress host, Logger *log = 0, QObject *parent = 0);
    ~VideoServer();

    void stop();
    void start(QString deviceName, StreamFormat format);
    void start(FlyCapture2::PGRGuid camera, StreamFormat format);
    QString getCameraName();
    VideoServer::State getState();
    const StreamFormat& getCurrentStreamFormat() const;

private:

    Logger *_log = NULL;
    QString _name;
    SocketAddress _host;
    Channel *_controlChannel = NULL;
    QUdpSocket *_videoSocket = NULL;
    StreamFormat _format;
    State _state = IdleState;
    QProcess _child;
    QString _deviceDescription;
    QTcpServer *_ipcServer = NULL;
    QTcpSocket *_ipcSocket = NULL;
    int _startInternalTimerId = TIMER_INACTIVE;

    /* Internal state change method
     */
    void setState(VideoServer::State state);

private slots:
    void videoSocketReadyRead();
    void controlChannelStateChanged(Channel *channel, Channel::State state);
    void startInternal();
    void childStateChanged(QProcess::ProcessState state);

    /* Begins streaming video to the provided address.
     * This will fail if the stream is not in WaitingState
     */
    void beginStream(SocketAddress address);

    void ipcServerClientAvailable();

signals:
    void stateChanged(VideoServer *server, VideoServer::State state);
    void eos(VideoServer *server);
    void error(VideoServer *server, QString message);
};

} // namespace Rover
} // namespace Soro

#endif // VIDEOSERVER_H
