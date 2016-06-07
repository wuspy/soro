#include "mediaserver.h"

namespace Soro {
namespace Rover {

MediaServer::MediaServer(QString logTag, int mediaId, QString childProcessPath, SocketAddress host, Logger *log, QObject *parent) : QObject(parent) {
    LOG_TAG = logTag;
    _log = log;
    _host = host;
    _mediaId = mediaId;

    _controlChannel = Channel::createServer(this, host.port, "soro_media" + QString::number(mediaId), Channel::TcpProtocol, host.host);
    _controlChannel->open();

    connect(_controlChannel, SIGNAL(stateChanged(Channel*, Channel::State)),
            this, SLOT(controlChannelStateChanged(Channel*, Channel::State)));

    _mediaSocket = new QUdpSocket(this);

    _ipcServer = new QTcpServer(this);
    _ipcServer->listen(QHostAddress::LocalHost);
    connect(_ipcServer, SIGNAL(newConnection()),
            this, SLOT(ipcServerClientAvailable()));

    _child.setProgram(childProcessPath);

    _state = IdleState;
}

MediaServer::~MediaServer() {
    stop();
}

void MediaServer::beginStream(SocketAddress address) {
    QStringList args;
    constructChildArguments(args, _host, address, _ipcServer->serverPort());
    _child.setArguments(args);

    connect(&_child, SIGNAL(stateChanged(QProcess::ProcessState)),
               this, SLOT(childStateChanged(QProcess::ProcessState)));
    _child.start();

    LOG_I("Sending streaming message to client");
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << QString("streaming");
    constructStreamingMessage(stream);
    LOG_I("Sending stream configuration to client");
    _controlChannel->sendMessage(message.constData(), message.size());

    setState(StreamingState);
}

void MediaServer::stop() {
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

    onStreamStoppedInternal();

    if (_controlChannel->getState() == Channel::ConnectedState) {
        // notify the client that the server is stopping the stream
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << QString("eos");
        _controlChannel->sendMessage(message.constData(), message.size());
    }
    _mediaSocket->abort();
    setState(IdleState);
}

void MediaServer::initStream() {
    LOG_I("start() called");
    if (_state != IdleState) {
        LOG_I("Server is not idle, stopping operations");
        stop();
    }
    setState(WaitingState);
    beginClientHandshake();
}

void MediaServer::beginClientHandshake() {
    if (_state != WaitingState) return;
    if (_controlChannel->getState() == Channel::ConnectedState) {
        _mediaSocket->abort();
        if (!_mediaSocket->bind(_host.host, _host.port)) {
            LOG_E("Cannot bind to UDP media host " + _host.toString() + ": " + _mediaSocket->errorString());
            QTimer::singleShot(500, this, SLOT(beginClientHandshake()));
            return;
        }
        connect(_mediaSocket, SIGNAL(readyRead()), this, SLOT(mediaSocketReadyRead()));
        _mediaSocket->open(QIODevice::ReadWrite);
        // notify a connected client that there is about to be a stream change
        // and they should verify their UDP address
        LOG_I("Sending stream start message to client");
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << QString("start");
        _controlChannel->sendMessage(message.constData(), message.size());
        // client must respond on its UDP address within a certain time or the process will start again
        QTimer::singleShot(3000, this, SLOT(beginClientHandshake()));

    }
    else {
        LOG_I("Waiting for client to connect...");
        QTimer::singleShot(500, this, SLOT(beginClientHandshake()));
    }
}

void MediaServer::ipcServerClientAvailable() {
    if (!_ipcSocket) {
        _ipcSocket = _ipcServer->nextPendingConnection();
        LOG_I("Streaming process is connected to its parent through TCP");
    }
}

void MediaServer::mediaSocketReadyRead() {
    if (!_mediaSocket | (_state == StreamingState)) return;
    SocketAddress peer;
    char buffer[100];
    int length = _mediaSocket->readDatagram(&buffer[0], 100, &peer.host, &peer.port);

    QByteArray byteArray = QByteArray::fromRawData(buffer, length);
    QDataStream stream(byteArray);
    QString tag;
    int mediaId;
    stream >> tag;
    stream >> mediaId;

    if (tag.compare("soro_media", Qt::CaseInsensitive) != 0) {
        LOG_E("Got invalid handshake packet on UDP media port");
        return;
    }
    if (mediaId != _mediaId) {
        LOG_E("Got wrong media ID during UDP handshake, check your port configuration");
        return;
    }
    LOG_I("Client has completed handshake on its UDP address");
    // Disconnect the media UDP socket so udpsink can bind to it
    disconnect(_mediaSocket, SIGNAL(readyRead()), this, SLOT(mediaSocketReadyRead()));
    _mediaSocket->abort(); // MUST ABORT THE SOCKET!!!!
    beginStream(peer);
}

void MediaServer::childStateChanged(QProcess::ProcessState state) {
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
            emit error(this, "Streaming process exited due to a gstreamer error");
            break;
        case STREAMPROCESS_ERR_INVALID_ARGUMENT:
        case STREAMPROCESS_ERR_NOT_ENOUGH_ARGUMENTS:
        case STREAMPROCESS_ERR_UNKNOWN_CODEC:
            LOG_E("Streaming processes exited due to an argument error");
            emit error(this, "Streaming processes exited due to an argument error");
            break;
        case STREAMPROCESS_ERR_SOCKET_ERROR:
            LOG_E("Streaming process exited because it lost contact with the parent cameraprocess");
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

void MediaServer::controlChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    if (state != Channel::ConnectedState) {
        stop();
    }
}

int MediaServer::getMediaId() {
    return _mediaId;
}

MediaServer::State MediaServer::getState() const {
    return _state;
}

void MediaServer::setState(MediaServer::State state) {
    if (_state != state) {
        LOG_I("Changing to state " + QString::number(reinterpret_cast<quint32&>(state)));
        _state = state;
        emit stateChanged(this, state);
    }
}

} // namespace Rover
} // namespace Soro
