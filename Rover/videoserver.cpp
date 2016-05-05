#include "videoserver.h"

#define LOG_TAG _name + "(S)"

namespace Soro {
namespace Rover {

VideoServer::VideoServer(QGst::ElementPtr camera, QString name, SocketAddress host, Logger *log, QObject *parent) : QObject(parent) {
    _name = name;
    _log = log;
    _host = host;
    _camera = camera;

    LOG_I("Creating new video server");

    _controlChannel = new Channel(this, host.port, "videostream", Channel::TcpProtocol, host.host);
    connect(_controlChannel, SIGNAL(stateChanged(Channel::State)),
            this, SLOT(controlChannelStateChanged(Channel::State)));
    _controlChannel->open();

    _videoSocket = new QUdpSocket(this);

    _state = IdleState;
}

VideoServer::~VideoServer() {
    stop();
    _camera.clear();
}

void VideoServer::stop() {
    if (_state == IdleState) {
        LOG_W("stop() called: Server is already stopped");
        return;
    }
    LOG_I("stop() called: stopping video feed");
    resetPipeline();
    if (_controlChannel->getState() == Channel::ConnectedState) {
        // notify the client that the server is stopping the stream
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << QString("eos");
        _controlChannel->sendMessage(message.constData(), message.size());
    }
    _videoSocket->abort();
    setState(IdleState);
}

void VideoServer::start(StreamFormat format) {
    if (_state == IdleState) {
        LOG_I("start() called");
        resetPipeline();
        _format = format;
        setState(WaitingState);
        startInternal();
    }
    else {
        LOG_W("start() called: Server is already started");
    }
}

void VideoServer::startInternal() {
    if (_state != WaitingState) return;
    if (_controlChannel->getState() == Channel::ConnectedState) {
        _videoSocket->abort();
        if (!_videoSocket->bind(_host.host, _host.port)) {
            LOG_E("Cannot bind to video port: " + _videoSocket->errorString());
            QTimer::singleShot(500, this, SLOT(startInternal()));
            return;
        }
        connect(_videoSocket, SIGNAL(readyRead()), this, SLOT(videoSocketReadyRead()));
        _videoSocket->open(QIODevice::ReadWrite);
        // notify a connected client that there is about to be a stream change
        // and they should verify their UDP address
        LOG_I("Sending stream start message to client");
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << QString("start");
        _controlChannel->sendMessage(message.constData(), message.size());
        // client must respond within a certain time or the process will start again
        QTimer::singleShot(3000, this, SLOT(startInternal()));
    }
    else {
        LOG_I("Waiting for client to connect...");
        QTimer::singleShot(500, this, SLOT(startInternal()));
    }
}

void VideoServer::beginStream(SocketAddress address) {
    LOG_I("Starting stream NOW");

    // create pipeline
    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &VideoServer::onBusMessage);

    // create gstreamer command
    QString binStr = "videoconvert ! ";
    QString caps = "video/x-raw,format=I420";
    if ((_format.Width > 0) & (_format.Height > 0)) {
        binStr += "videoscale ! ";
        caps += ",width=" + QString::number(_format.Width) + ",height=" + QString::number(_format.Height);
    }
    if (_format.Framerate > 0) {
        caps += ",framerate=" + QString::number(_format.Framerate) + "/1";
        binStr += "videorate ! ";
    }
    binStr += caps;
    switch (_format.Encoding) {
    case MjpegEncoding:
        binStr += " ! jpegenc quality=" + QString::number(_format.Mjpeg_Quality) + " ! rtpjpegpay ! ";
        break;
    case Mpeg2Encoding:
        binStr += " ! avenc_mpeg4 bitrate=" + QString::number(_format.Mpeg2_Bitrate) + " ! rtpmp4vpay config-interval=3 ! ";
        break;
    default:
        LOG_E("Cannot start stream because the specified encoding is not valid");
        stop();
        return;
    }

    binStr += "udpsink host=" + address.host.toString() + " port=" + QString::number(address.port);
    LOG_I("Pipe command: <source> ! " +  binStr);
    QGst::BinPtr streamer = QGst::Bin::fromDescription(binStr);

    // link elements
    _pipeline->add(_camera, streamer);
    _camera->link(streamer);

    // play
    _pipeline->setState(QGst::StatePlaying);
    setState(StreamingState);
}

void VideoServer::resetPipeline() {
    if (_pipeline) {
        LOG_I("Resetting gstreamer pipeline");
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
}

void VideoServer::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
}

void VideoServer::videoSocketReadyRead() {
    if (!_videoSocket | (_state == StreamingState)) return;
    SocketAddress peer;
    char buffer[100];
    _videoSocket->readDatagram(&buffer[0], 100, &peer.host, &peer.port);
    if ((strcmp(buffer, _name.toLatin1().constData()) == 0) && (_format.Encoding != UnknownEncoding)) {
        LOG_I("Client has completed handshake on its UDP address");
        // send the client a message letting them know we are now streaming to their address,
        // and tell them the stream metadata
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << QString("streaming");
        stream << reinterpret_cast<unsigned int&>(_format.Encoding);
        stream << _format.Width;
        stream << _format.Height;
        stream << _format.Framerate;
        switch (_format.Encoding) {
        case MjpegEncoding:
            stream << _format.Mjpeg_Quality;
            break;
        case Mpeg2Encoding:
            stream << _format.Mpeg2_Bitrate;
            break;
        default:
            LOG_E("The format's encoding is set to Unknown, why am I starting a stream???");
            stop();
            return;
        }
        LOG_I("Sending stream configuration to client");
        _controlChannel->sendMessage(message.constData(), message.size());
        // Disconnect the video UDP socket so udpsink can bind to it
        disconnect(_videoSocket, SIGNAL(readyRead()), this, SLOT(videoSocketReadyRead()));
        _videoSocket->abort(); // MUST ABORT THE SOCKET!!!!
        beginStream(peer);
    }
}

void VideoServer::controlChannelStateChanged(Channel::State state) {
    if (state != Channel::ConnectedState) {
        stop();
    }
}

void VideoServer::onBusMessage(const QGst::MessagePtr & message) {
    switch (message->type()) {
    case QGst::MessageEos: {
        LOG_W("Received EOS message from stream, stopping");
        stop();
        // notify the client of eos
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << QString("eos");
        _controlChannel->sendMessage(message.constData(), message.size());
        emit eos();
    }
        break;
    case QGst::MessageError: {
        QString errorString = message.staticCast<QGst::ErrorMessage>()->error().message();
        LOG_E("Pipeline error: " + errorString);
        stop();
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << QString("error");
        stream << errorString;
        _controlChannel->sendMessage(message.constData(), message.size());
        emit error(errorString);
    }
        break;
    default:
        break;
    }
}

VideoServer::State VideoServer::getState() {
    return _state;
}

QString VideoServer::getCameraName() {
    return _name;
}

void VideoServer::setState(VideoServer::State state) {
    if (_state != state) {
        LOG_I("Changing to state " + QString::number(reinterpret_cast<unsigned int&>(state)));
        _state = state;
        emit stateChanged(state);
    }
}

} // namespace Rover
} // namespace Soro

