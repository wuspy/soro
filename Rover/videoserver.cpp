#include "videoserver.h"

#define LOG_TAG _name + "(S)"

namespace Soro {
namespace Rover {

VideoServer::VideoServer(QString name, SocketAddress host, Logger *log, QObject *parent) : QObject(parent) {
    _name = name;
    _log = log;
    _host = host;

    LOG_I("Creating new video server");

    _controlChannel = new Channel(this, host.port, name, Channel::TcpProtocol, host.host);

    connect(_controlChannel, SIGNAL(stateChanged(Channel::State)),
            this, SLOT(controlChannelStateChanged(Channel::State)));

    _controlChannel->open();

    _state = IdleState;
}

VideoServer::~VideoServer() {
    stop();
}

void VideoServer::stop() {
    LOG_I("stop() called");
    resetPipeline();
    if (_controlChannel->getState() == Channel::ConnectedState) {
        const char *message = "[stop]";
        _controlChannel->sendMessage(message, strlen(message) + 1);
    }
    if (_videoSocket) {
        _videoSocket->abort();
        delete _videoSocket;
        _videoSocket = NULL;
    }
    setState(IdleState);
}

void VideoServer::start(QGst::ElementPtr source, const StreamFormat *format) {
    LOG_I("start() called");
    resetPipeline();
    _camera = source;
    _format = format;
    setState(WaitingState);
    if (_controlChannel->getState() == Channel::ConnectedState) {
        if (!_videoSocket) {
            // create a temporary socket which we will use to determine the client's address
            _videoSocket = new QUdpSocket(this);
            if (!_videoSocket->bind(_host.host, _host.port)) {
                LOG_E("Cannot bind to video port, retrying...");
                delete _videoSocket;
                _videoSocket = NULL;
                START_TIMER(_startTimerId, 500);
                return;
            }
            connect(_videoSocket, SIGNAL(readyRead()),
                    this, SLOT(videoSocketReadyRead()));
        }
        // notify a connected client that there is about to be a stream change
        // and they should verify their UDP address
        LOG_I("Sending stream configuration to client");
        const char *message = "[start]";
        _controlChannel->sendMessage(message, strlen(message) + 1);
        // client must respond within a certain time or the process will start again
        START_TIMER(_startTimerId, 3000);
    }
    else {
        LOG_W("Waiting for client to connect...");
        START_TIMER(_startTimerId, 500);
    }
}

void VideoServer::beginStream(SocketAddress address) {
    LOG_I("Starting stream NOW");
    if (_videoSocket) {
        // unbind and delete the video socket before creating the udpsink
        _videoSocket->abort();
        delete _videoSocket;
        _videoSocket = NULL;
    }

    setState(StreamingState);

    // create pipeline
    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &VideoServer::onBusMessage);

    // create gstreamer command
    QString binStr = "videoconvert ! ";
    QString caps = "video/x-raw,format=I420";
    if ((_format->Width > 0) & (_format->Height > 0)) {
        binStr += "videoscale ! ";
        caps += ",width=" + QString::number(_format->Width) + ",height=" + QString::number(_format->Height);
    }
    if (_format->Framerate > 0) {
        caps += ",framerate=" + QString::number(_format->Framerate) + "/1";
    }
    binStr += caps;
    switch (_format->encoding()) {
    case MJPEG:
        binStr += " ! jpegenc quality=" + QString::number(((MjpegStreamFormat*)_format)->Quality) + " ! rtpjpegpay ! ";
        break;
    case MPEG2:
        binStr += " ! avenc_mpeg4 bitrate=" + QString::number(((Mpeg2StreamFormat*)_format)->Bitrate) + " ! rtpmp4vpay config-interval=3 ! ";
        break;
    }

    binStr += "udpsink host=" + address.host.toString() + " port=" + QString::number(address.port);

    QGst::BinPtr streamer = QGst::Bin::fromDescription(binStr);

    // link elements
    _pipeline->add(_camera, streamer);
    _camera->link(streamer);

    // play
    LOG_I("Beginning stream");
    _pipeline->setState(QGst::StatePlaying);
}

void VideoServer::resetPipeline() {
    if (_pipeline) {
        LOG_I("Resetting gstreamer pipeline");
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
    _camera.clear();
}

void VideoServer::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _startTimerId) {
        KILL_TIMER(_startTimerId);
        start(_camera, _format);
    }
}

void VideoServer::videoSocketReadyRead() {
    SocketAddress peer;
    char buffer[100];
    _videoSocket->readDatagram(&buffer[0], 100, &peer.host, &peer.port);
    if ((strcmp(buffer, _name.toLatin1().constData()) == 0) && _format != NULL) {
        LOG_I("Client has completed handshake on its UDP address");
        KILL_TIMER(_startTimerId);
        // send the client a message letting them know we are now streaming to their address,
        // and tell them the stream metadata
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream << "streaming";
        switch (_format->encoding()) {
        case MJPEG:

            break;
        case MPEG2:
            response = "[streaming]enc=MPEG2";
            break;
        }
        LOG_I("Sending stream configuration to client");
        _controlChannel->sendMessage(response, strlen(response) + 1);
        beginStream(peer);
    }
}

void VideoServer::controlChannelStateChanged(Channel::State state) {
    if (_pipeline && (state != Channel::ConnectedState)) {
        LOG_W("Lost connection to client, stopping stream");
        stop();
    }
}

void VideoServer::onBusMessage(const QGst::MessagePtr & message) {
    const char *clientMessage;
    QString errorString;

    switch (message->type()) {
    case QGst::MessageEos:
        LOG_W("Received EOS message from stream, stopping");
        stop();
        // notify the client of eos
        clientMessage = "[eos]";
        _controlChannel->sendMessage(clientMessage, strlen(clientMessage) + 1);
        emit eos();
        break;
    case QGst::MessageError:
        errorString = message.staticCast<QGst::ErrorMessage>()->error().message();
        LOG_E("Pipeline error: " + errorString);
        stop();
        // notify client of the error
        clientMessage = "[error]";
        _controlChannel->sendMessage(clientMessage, strlen(clientMessage) + 1);
        emit error(errorString);
        break;
    default:
        break;
    }
}

void VideoServer::setState(VideoServer::State state) {
    if (_state != state) {
        _state = state;
        emit stateChanged(state);
    }
}

} // namespace Rover
} // namespace Soro

