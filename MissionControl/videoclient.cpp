#include "videoclient.h"

#define LOG_TAG _name + "(C)"

namespace Soro {
namespace MissionControl {

VideoClient::VideoClient(QString name, SocketAddress server, QHostAddress host, Logger *log, QObject *parent) : QObject(parent) {
    _name = name;
    _server = server;
    _log = log;

    LOG_I("Creating new video client for server at " + server.toString());

    _controlChannel = new Channel(this, server, name, Channel::TcpProtocol, host);
    _videoSocket = new QUdpSocket(this);
    _videoSocket->bind(host);
    _videoSocket->open(QIODevice::ReadWrite);

    connect(_controlChannel, SIGNAL(messageReceived(const char*,Channel::MessageSize)),
            this, SLOT(controlMessageReceived(const char*,Channel::MessageSize)));
}

void VideoClient::addForwardingAddress(SocketAddress address) {
    foreach (SocketAddress existing, _forwardAddresses) {
        if (existing == address) return;
    }
    _forwardAddresses.append(address);
}

void VideoClient::removeForwardingAddress(SocketAddress address) {
    int index = _forwardAddresses.indexOf(address);
    if (index >= 0) {
        _forwardAddresses.removeAt(index);
    }
}

const StreamFormat* VideoClient::getStreamFormat() {

}

void VideoClient::controlMessageReceived(const char *message, Channel::MessageSize size) {
    Q_UNUSED(size);
    QString messageStr(message);
    if (messageStr.startsWith("[ready]", Qt::CaseInsensitive)) {
        setState(ConnectedState);
        disconnect(_videoSocket, SIGNAL(readyRead()), 0, 0);
        START_TIMER(_punchTimerId, 100);
    }
    else if (messageStr.startsWith("[streaming]", Qt::CaseInsensitive)) {
        // we were successful and are now receiving a video stream
        setState(StreamingState);
        videoSocketReadyRead();
        connect(_videoSocket, SIGNAL(readyRead()),
                this, SLOT(videoSocketReadyRead()));
        KILL_TIMER(_punchTimerId);
    }
    else if (messageStr.startsWith("[eos]", Qt::CaseInsensitive)) {
        setState(ConnectedState);
        KILL_TIMER(_punchTimerId);
        disconnect(_videoSocket, SIGNAL(readyRead()), 0, 0);
        emit serverEos();
    }
    else if (messageStr.startsWith("[error]", Qt::CaseInsensitive)) {
        setState(ConnectedState);
        disconnect(_videoSocket, SIGNAL(readyRead()), 0, 0);
        KILL_TIMER(_punchTimerId);
        emit serverError(QString(message + 7));
    }
}

void VideoClient::videoSocketReadyRead() {
    qint64 size;
    while (_videoSocket->hasPendingDatagrams()) {
        size = _videoSocket->readDatagram(_buffer, 65536);
        // forward the datagram to all specified addresses
        foreach (SocketAddress address, _forwardAddresses) {
            _videoSocket->writeDatagram(_buffer, size, address.host, address.port);
        }
    }
}

void VideoClient::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _punchTimerId) {
        // send data to the the server so it can figure out our address
        const QByteArray message = _name.toLatin1();
        _videoSocket->writeDatagram(message.constData(), message.size(), _server.host, _server.port);
    }
}

void VideoClient::controlChannelStateChanged(Channel::State state) {
    switch (state) {
    case Channel::ConnectedState:
        setState(ConnectedState);
        break;
    default:
        setState(ConnectingState);
        break;
    }
}

VideoClient::State VideoClient::getState() {
    return _state;
}

void VideoClient::setState(State state) {
    if (_state != state) {
        _state = state;
        emit stateChanged(_state);
    }
}

} // namespace MissionControl
} // namespace Soro
