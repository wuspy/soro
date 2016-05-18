#include "videoserver.h"

#define LOG_TAG "Camera Server " + QString::number(_cameraId)

namespace Soro {
namespace Rover {

VideoServer::VideoServer(int cameraId, SocketAddress host, Logger *log, QObject *parent) : QObject(parent) {
    _cameraId = cameraId;
    _log = log;
    _host = host;

    LOG_I("Creating new video server with camera ID\"" + QString::number(cameraId) + "\" and address " + host.toString());

    _controlChannel = new Channel(this, host.port, "camera" + QString::number(cameraId), Channel::TcpProtocol, host.host);
    connect(_controlChannel, SIGNAL(stateChanged(Channel*, Channel::State)),
            this, SLOT(controlChannelStateChanged(Channel*, Channel::State)));
    _controlChannel->open();

    _videoSocket = new QUdpSocket(this);

    _ipcServer = new QTcpServer(this);
    _ipcServer->listen(QHostAddress::LocalHost);
    connect(_ipcServer, SIGNAL(newConnection()),
            this, SLOT(ipcServerClientAvailable()));

    _child.setProgram(QCoreApplication::applicationDirPath() + "/VideoStreamProcess");

    _state = IdleState;
}

VideoServer::~VideoServer() {
    stop();
}

void VideoServer::stop() {
    if (_state == IdleState) {
        LOG_W("stop() called: Server is already stopped");
        return;
    }
    if (_child.state() != QProcess::NotRunning) {
        LOG_I("stop() called: asking the streaming process to stop");
        if (_ipcSocket) {
            _ipcSocket->write("stop");
            _ipcSocket->flush();
            if (!_child.waitForFinished(1000)) {
                LOG_E("Streaming process did not respond to stop request, terminating it");
                _child.terminate();
                _child.waitForFinished();
                LOG_I("Streaming process has been terminated");
            }
            else {
                LOG_I("Streaming process has exited gracefully");
            }
        }
        else {
            LOG_E("Streaming process is not connected to the rover process, terminating it");
            _child.terminate();
            _child.waitForFinished();
            LOG_I("Streaming process has been terminated");
        }
    }
    else {
        LOG_I("stop() called, however the child process is not running");
    }
    if (_ipcSocket) {
        disconnect(_ipcSocket, 0, 0, 0);
        _ipcSocket->abort();
        delete _ipcSocket;
        _ipcSocket = NULL;
    }
    _currentCamera = "";
    _format.Encoding = UnknownOrNoEncoding;
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

void VideoServer::start(QString deviceName, StreamFormat format) {
    LOG_I("start() called");
    if (_state != IdleState) {
        LOG_I("Server is not idle, stopping operations");
        stop();
    }
    _currentCamera = deviceName;
    _format = format;
    setState(WaitingState);
    startInternal();
}

void VideoServer::start(FlyCapture2::PGRGuid camera, StreamFormat format) {
    start("FlyCapture2:" + QString::number(camera.value[0]) + ":"
                        + QString::number(camera.value[1]) + ":"
                        + QString::number(camera.value[2]) + ":"
                        + QString::number(camera.value[3]),
                        format);
}

void VideoServer::startInternal() {
    if (_state != WaitingState) return;
    if (_controlChannel->getState() == Channel::ConnectedState) {
        _videoSocket->abort();
        if (!_videoSocket->bind(_host.host, _host.port)) {
            LOG_E("Cannot bind to video host " + _host.toString() + ": " + _videoSocket->errorString());
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
        // client must respond on its UDP address within a certain time or the process will start again
        QTimer::singleShot(3000, this, SLOT(startInternal()));

    }
    else {
        LOG_I("Waiting for client to connect...");
        QTimer::singleShot(500, this, SLOT(startInternal()));
    }
}

void VideoServer::beginStream(SocketAddress address) {
    QStringList args;
    args << _currentCamera;
    args << QString::number(reinterpret_cast<unsigned int&>(_format.Encoding));
    args << QString::number(_format.Height);
    switch (_format.Encoding) {
    case MjpegEncoding:
        args << QString::number(_format.Mjpeg_Quality);
        break;
    default:
        args << QString::number(_format.Bitrate);
        break;
    }

    args << QHostAddress(address.host.toIPv4Address()).toString();
    args << QString::number(address.port);
    args << QHostAddress(_host.host.toIPv4Address()).toString();
    args << QString::number(_host.port);
    args << QString::number(_ipcServer->serverPort());

    _child.setArguments(args);
    connect(&_child, SIGNAL(stateChanged(QProcess::ProcessState)),
               this, SLOT(childStateChanged(QProcess::ProcessState)));
    _child.start();
    qDebug() << "Starting with args " << args;

    LOG_I("Sending streaming message to client");
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << QString("streaming");
    stream << _format;
    LOG_I("Sending stream configuration to client");
    _controlChannel->sendMessage(message.constData(), message.size());

    setState(StreamingState);
}

void VideoServer::ipcServerClientAvailable() {
    if (!_ipcSocket) {
        _ipcSocket = _ipcServer->nextPendingConnection();
        connect(_ipcSocket, SIGNAL(readyRead()),
                this, SLOT(ipcSocketReadyRead()));
        LOG_I("Streaming process is connected to its parent through TCP");
    }
}

void VideoServer::ipcSocketReadyRead() {
    if (_ipcSocket->canReadLine()) {
        QByteArray errorMessage = _ipcSocket->readLine(256);
        emit error(this, QString(errorMessage));
    }
}

void VideoServer::videoSocketReadyRead() {
    if (!_videoSocket | (_state == StreamingState)) return;
    SocketAddress peer;
    char buffer[100];
    int length = _videoSocket->readDatagram(&buffer[0], 100, &peer.host, &peer.port);

    QByteArray byteArray = QByteArray::fromRawData(buffer, length);
    QDataStream stream(byteArray);
    QString tag;
    int cameraId;
    stream >> tag;
    stream >> cameraId;

    if (tag.compare("camera", Qt::CaseInsensitive) != 0) {
        LOG_E("Got invalid handshake packet on UDP video port");
        return;
    }
    if (cameraId != _cameraId) {
        LOG_E("Got wrong camera ID during UDP handshake, check your port configuration");
        return;
    }
    LOG_I("Client has completed handshake on its UDP address");
    // Disconnect the video UDP socket so udpsink can bind to it
    disconnect(_videoSocket, SIGNAL(readyRead()), this, SLOT(videoSocketReadyRead()));
    _videoSocket->abort(); // MUST ABORT THE SOCKET!!!!
    beginStream(peer);
}

void VideoServer::childStateChanged(QProcess::ProcessState state) {
    switch (state) {
    case QProcess::NotRunning:
        disconnect(&_child, SIGNAL(stateChanged(QProcess::ProcessState)),
                   this, SLOT(childStateChanged(QProcess::ProcessState)));

        switch (_child.exitCode()) {
        case 0:
        case STREAMPROCESS_ERR_GSTREAMER_EOS:
            LOG_I("Streaming process has exited normally");
            emit eos(this);
            break;
        case STREAMPROCESS_ERR_FLYCAP_ERROR:
            LOG_E("Streaming processes exited due to an error in FlyCapture2 processing");
            break;
        case STREAMPROCESS_ERR_GSTREAMER_ERROR:
            LOG_E("The streaming processes exited due to a gstreamer error");
            break;
        case STREAMPROCESS_ERR_INVALID_ARGUMENT:
        case STREAMPROCESS_ERR_NOT_ENOUGH_ARGUMENTS:
        case STREAMPROCESS_ERR_UNKNOWN_CODEC:
            LOG_E("Streaming processes exited due to an argument error");
            emit error(this, "Streaming processes exited due to an argument error");
            break;
        case STREAMPROCESS_ERR_SOCKET_ERROR:
            LOG_E("Streaming process exited because it lost contact with the parent process");
            emit error(this, "Streaming process exited because it lost contact with the parent process");
            break;
        default:
            LOG_E("Streaming process exited due to an unknown error (exit code " + QString::number(_child.exitCode()) + ")");
            emit error(this, "Streaming process exited due to an unknown error (exit code " + QString::number(_child.exitCode()) + ")");
            break;
        }

        stop();
        break;
    case QProcess::Starting:
        LOG_I("Child is starting...");
        break;
    case QProcess::Running:
        LOG_I("Child has started successfully");
        break;
    }
}

void VideoServer::controlChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    if (state != Channel::ConnectedState) {
        stop();
    }
}

VideoServer::State VideoServer::getState() const {
    return _state;
}

int VideoServer::getCameraId() const {
    return _cameraId;
}

const StreamFormat& VideoServer::getCurrentStreamFormat() const {
    return _format;
}

void VideoServer::setState(VideoServer::State state) {
    if (_state != state) {
        LOG_I("Changing to state " + QString::number(reinterpret_cast<unsigned int&>(state)));
        _state = state;
        emit stateChanged(this, state);
    }
}

} // namespace Rover
} // namespace Soro
