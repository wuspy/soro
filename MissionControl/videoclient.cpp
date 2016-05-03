#include "videoclient.h"

#define LOG_TAG _name + "(C)"

namespace Soro {
namespace MissionControl {

VideoClient::VideoClient(QString name, SocketAddress server, QHostAddress host, Logger *log, QObject *parent) : QObject(parent) {
    _name = name;
    _server = server;
    _log = log;

    LOG_I("Creating new video client for server at " + server.toString());

    _controlChannel = new Channel(this, _server, _name, Channel::TcpProtocol);
    _videoSocket = new QUdpSocket(this);
    _videoSocket->bind(host);
    _videoSocket->open(QIODevice::ReadWrite);

    _buffer = new char[65536];

    connect(_controlChannel, SIGNAL(messageReceived(const char*,Channel::MessageSize)),
            this, SLOT(controlMessageReceived(const char*,Channel::MessageSize)));
    connect(_controlChannel, SIGNAL(stateChanged(Channel::State)),
            this, SLOT(controlChannelStateChanged(Channel::State)));

    _controlChannel->open();

    START_TIMER(_calculateBitrateTimerId, 500);
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

VideoEncoding VideoClient::getEncoding() {
    return _encoding;
}

void VideoClient::controlMessageReceived(const char *message, Channel::MessageSize size) {
    Q_UNUSED(size);
    QString messageStr(message);
    if (messageStr.startsWith("[start]", Qt::CaseInsensitive)) {
        LOG_I("Server has notified us of a new video stream");
        setState(ConnectedState);
        _encoding = UNKNOWN;
        disconnect(_videoSocket, SIGNAL(readyRead()), 0, 0);
        START_TIMER(_punchTimerId, 100);
    }
    else if (messageStr.startsWith("[streaming]", Qt::CaseInsensitive)) {
        // we were successful and are now receiving a video stream
        LOG_I("Server has confirmed our address and should begin streaming");
        setState(StreamingState);
        videoSocketReadyRead();
        connect(_videoSocket, SIGNAL(readyRead()),
                this, SLOT(videoSocketReadyRead()));
        if (messageStr.indexOf("enc=MJPEG", Qt::CaseInsensitive) >= 0) {
            _encoding = MJPEG;
        }
        else if (messageStr.indexOf("enc=MPEG2", Qt::CaseInsensitive) >= 0) {
            _encoding = MPEG2;
        }
        else {
            _encoding = UNKNOWN;
        }
        KILL_TIMER(_punchTimerId);
    }
    else if (messageStr.startsWith("[eos]", Qt::CaseInsensitive)) {
        LOG_I("Got EOS message from server");
        setState(ConnectedState);
        _encoding = UNKNOWN;
        KILL_TIMER(_punchTimerId);
        disconnect(_videoSocket, SIGNAL(readyRead()), 0, 0);
        emit serverEos();
    }
    else if (messageStr.startsWith("[error]", Qt::CaseInsensitive)) {
        LOG_I("Got error message from server: " + QString(message + 7));
        setState(ConnectedState);
        _encoding = UNKNOWN;
        disconnect(_videoSocket, SIGNAL(readyRead()), 0, 0);
        KILL_TIMER(_punchTimerId);
        emit serverError();
    }
    else if (messageStr.startsWith("[stop]")) {
        LOG_I("Got a [stop] message from the server");
        setState(ConnectedState);
        _encoding = UNKNOWN;
        KILL_TIMER(_punchTimerId);
        disconnect(_videoSocket, SIGNAL(readyRead()), 0, 0);
        emit stopped();
    }
    else {
        LOG_E("Got unknown message from video server: " + messageStr);
    }
}

void VideoClient::videoSocketReadyRead() {
    qint64 size;
    while (_videoSocket->hasPendingDatagrams()) {
        size = _videoSocket->readDatagram(_buffer, 65536);
        _bitCount += size * 8;
        // forward the datagram to all specified addresses
        foreach (SocketAddress address, _forwardAddresses) {
            _videoSocket->writeDatagram(_buffer, size, address.host, address.port);
        }
    }
}

void VideoClient::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _punchTimerId) {
        LOG_D("punch timer tick");
        // send data to the the server so it can figure out our address
        _videoSocket->writeDatagram(_name.toLatin1().constData(), _name.size() + 1, _server.host, _server.port);
    }
    else if (e->timerId() == _calculateBitrateTimerId) {
        // this timer runs twice per second to calculate the bitrate received by the client
        _lastBitrate = _bitCount * 2;
        _bitCount = 0;
        emit statisticsUpdate(_lastBitrate);
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
