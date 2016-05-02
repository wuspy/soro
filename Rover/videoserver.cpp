#include "videoserver.h"

#define LOG_TAG _name + "(S)"

namespace Soro {
namespace Rover {

VideoServer::VideoServer(QString name, SocketAddress host, Logger *log, QObject *parent) : QObject(parent) {
    _name = name;
    _host = host;
    _log = log;

    LOG_I("Creating new video server");

    _controlChannel = new Channel(this, host.port, name, Channel::TcpProtocol, host.host);

    connect(_controlChannel, SIGNAL(peerAddressChanged(SocketAddress)),
            this, SLOT(clientChanged(SocketAddress)));

    _controlChannel->setSendAcks(false); // no need for latency calculations
    _controlChannel->open();

    _state = UnconfiguredState;
}

void VideoServer::configure(QGst::ElementPtr cameraSource, const StreamFormat *format) {
    resetPipeline();
    if (_camera) {
        // release any existing camera source
        _camera.clear();
    }
    _camera = cameraSource;
    _format = format;
    setState(WaitingState);
    if (_controlChannel->getState() == Channel::ConnectedState) {
        LOG_I("Got new stream configuration, beginning pre-stream handshake");
        START_TIMER(_sendStreamReadyTimerId, 1);
    }
    else {
        LOG_I("Got new stream configuration but no client has connected yet");
    }
}

void VideoServer::stop() {
    if ((_state == WaitingState) | (_state == StreamingState)) {
        if (_controlChannel->getState() == Channel::ConnectedState) {
            LOG_I("Stopping stream by request and notifying client");
            // notify client of EOS
            QString message("[eos]");
            _controlChannel->sendMessage(message.toLatin1().constData(), message.length() + 1);
        }
        resetPipeline();
        if (_camera) {
            // release any existing camera source
            _camera.clear();
        }
        _format = NULL;
        setState(UnconfiguredState);
    }
}

void VideoServer::beginStream(SocketAddress address) {
    if (_videoSocket) {
        // unbind and delete the video socket before creating the udpsink
        LOG_D("Unbinding temporary socket in preparation for new stream");
        _videoSocket->close();
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
        binStr += "videorate ! ";
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

    _streamer = QGst::Bin::fromDescription(binStr);

    // link elements
    _pipeline->add(_camera, _streamer);
    _camera->link(_streamer);

    // play
    LOG_I("Beginning stream");
    _pipeline->setState(QGst::StatePlaying);
}

void VideoServer::resetPipeline() {
    LOG_D("Resetting pipeline...");
    if (_pipeline) {
        if (_camera && _streamer) {
            // preserve the camera source element
            _camera->unlink(_streamer);
            _pipeline->remove(_camera);
        }
        _pipeline->setState(QGst::StateNull);
        _streamer->setState(QGst::StateNull);
        _pipeline.clear();
        _streamer.clear();
    }
}

void VideoServer::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _sendStreamReadyTimerId) {
        if (_state != WaitingState) {
            LOG_E("sendStreamReadyTimer not started in WaitingState");
            KILL_TIMER(_sendStreamReadyTimerId);
            return;
        }
        if (!_videoSocket) {
            // create a temporary socket which we will use to determine the client's address
            LOG_D("Creating temporary socket to receive client address");
            _videoSocket = new QUdpSocket(this);
            if (!_videoSocket->bind(_host.host, _host.port)) {
                LOG_E("Cannot bind to video port");
                return;
            }
            connect(_videoSocket, SIGNAL(readyRead()),
                    this, SLOT(videoSocketReadyRead()));
        }
        // notify a connected client that there is about to be a stream change
        // and they should verify their UDP address
        QString message;
        switch (_format->encoding()) {
        case MJPEG:
            message = QString("[ready]enc=MJPEG,w=%1,h=%2,fps=%3,q=%4")
                    .arg(_format->Width)
                    .arg(_format->Height)
                    .arg(_format->Framerate)
                    .arg(((MjpegStreamFormat*)_format)->Quality);
            break;
        case MPEG2:
            message = QString("[ready]enc=MPEG2,w=%1,h=%2,fps=%3,q=%4")
                    .arg(_format->Width)
                    .arg(_format->Height)
                    .arg(_format->Framerate)
                    .arg(((Mpeg2StreamFormat*)_format)->Bitrate);
            break;
        }
        _controlChannel->sendMessage(message.toLatin1().constData(), message.length() + 1);
        // kill this timer now that the message has been successfully sent
        KILL_TIMER(_sendStreamReadyTimerId);
    }
}

void VideoServer::clientChanged(SocketAddress client) {
    switch (_state) {
    case StreamingState:
        resetPipeline();
        setState(WaitingState);
        if (client.host != QHostAddress::Null) {
            LOG_I("Client has changed while already streaming, starting pre-stream handshake with new client");
            START_TIMER(_sendStreamReadyTimerId, 500);
        }
        else {
            LOG_I("Client has disconnected mid-stream");
        }
        break;
    case WaitingState:
        if (client.host != QHostAddress::Null) {
            LOG_I("Client is connected, starting pre-stream handshake");
            START_TIMER(_sendStreamReadyTimerId, 500);
        }
        else {
            LOG_I("Client has disconnected");
        }
        break;
    case UnconfiguredState:
        LOG_I("Client has connected but we have nothing to give them");
        break;
    }
}

void VideoServer::videoSocketReadyRead() {
    if (_state != WaitingState) {
        LOG_E("Receiving UDP client handshake not in WaitingState");
        return;
    }
    SocketAddress peer;
    char buffer[100];
    _videoSocket->readDatagram(&buffer[0], 100, &peer.host, &peer.port);
    if ((strcmp(buffer, _name.toLatin1().constData()) == 0) && _format != NULL) {
        LOG_I("Client has completed handshake on its UDP address");
        // send the client a message letting them know we are now streaming to their address
        QString message("[streaming]");
        _controlChannel->sendMessage(message.toLatin1().constData(), message.length() + 1);
        beginStream(peer);
    }
}

void VideoServer::onBusMessage(const QGst::MessagePtr & message) {
    if (_state != StreamingState) {
        LOG_E("Got pipeline bus message not in StreamingState");
        return;
    }
    switch (message->type()) {
    case QGst::MessageEos:
    {
        LOG_W("Received EOS message from stream, restarting anyway");
        resetPipeline();
        setState(WaitingState);
        // notify client of the EOS message
        QString clientMessage("[eos]");
        _controlChannel->sendMessage(clientMessage.toLatin1().constData(), clientMessage.length() + 1);
        // resend stream ready signal to client to try to restart the stream
        START_TIMER(_sendStreamReadyTimerId, 500);
        emit eos();
    }
        break;
    case QGst::MessageError:
    {
        QString errorString(message.staticCast<QGst::ErrorMessage>()->error().message());
        LOG_E("Pipeline error: " + errorString);
        resetPipeline();
        setState(WaitingState);
        // notify client of the error
        QString clientMessage = "[error]" + errorString;
        _controlChannel->sendMessage(clientMessage.toLatin1().constData(), clientMessage.length() + 1);
        // resend stream ready signal to client to try to restart the stream
        START_TIMER(_sendStreamReadyTimerId, 500);
        emit error(errorString);
    }
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

