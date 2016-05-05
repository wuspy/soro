#ifndef VIDEOSERVER_H
#define VIDEOSERVER_H

#include <QObject>
#include <QUdpSocket>

#include <Qt5GStreamer/QGst/Ui/VideoWidget>
#include <Qt5GStreamer/QGst/Pipeline>
#include <Qt5GStreamer/QGst/Element>
#include <Qt5GStreamer/QGst/ElementFactory>
#include <Qt5GStreamer/QGst/Bin>
#include <Qt5GStreamer/QGst/Bus>
#include <Qt5GStreamer/QGlib/RefPointer>
#include <Qt5GStreamer/QGlib/Error>
#include <Qt5GStreamer/QGlib/Connect>
#include <Qt5GStreamer/QGst/Message>

#include "socketaddress.h"
#include "soro_global.h"
#include "videoencoding.h"
#include "logger.h"
#include "channel.h"

namespace Soro {
namespace Rover {

class VideoServer : public QObject {
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
    void start(QGst::ElementPtr camera, StreamFormat format);
    QString getCameraName();
    VideoServer::State getState();

private:

    Logger *_log = NULL;
    QString _name;
    SocketAddress _host;
    QGst::PipelinePtr _pipeline;
    QGst::ElementPtr _camera;
    Channel *_controlChannel = NULL;
    QUdpSocket *_videoSocket = NULL;
    StreamFormat _format;
    State _state = IdleState;

    /* Resets the stream pipeline and releases all elements
     * except the source camera. This does not alter the server
     * state in itself.
     */
    void resetPipeline();
    /* Internal state change method
     */
    void setState(VideoServer::State state);

private slots:
    void onBusMessage(const QGst::MessagePtr & message);
    void videoSocketReadyRead();
    void controlChannelStateChanged(Channel::State state);
    void startInternal();

    /* Begins streaming video to the provided address.
     * This will fail if the stream is not in WaitingState
     */
    void beginStream(SocketAddress address);

signals:
    void stateChanged(VideoServer::State state);
    void eos();
    void error(QString message);

protected:
    void timerEvent(QTimerEvent *e);
};

} // namespace Rover
} // namespace Soro

#endif // VIDEOSERVER_H
