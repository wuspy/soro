#include "mediaclient.h"

namespace Soro {
namespace MissionControl {

MediaClient::MediaClient(QString logTag, int mediaId, SocketAddress server, QHostAddress host, Logger *log, QObject *parent)
    : QObject(parent) {

    LOG_TAG = logTag;
    _mediaId = mediaId;
    _server = server;
    _log = log;

    LOG_I("Creating new media client for server at " + server.toString());

    _controlChannel = new Channel(this, _server, "soro_media" + QString::number(mediaId), Channel::TcpProtocol, host, _log);
    _mediaSocket = new QUdpSocket(this);

    _buffer = new char[65536];

    connect(_controlChannel, SIGNAL(messageReceived(Channel*, const char*, Channel::MessageSize)),
            this, SLOT(controlMessageReceived(Channel*, const char*, Channel::MessageSize)));

    _controlChannel->open();

    // MUST connect this signal after opening the channel, otherwise the stateChanged()
    // signal will be emitted and trigger an invoking of a virtual method in
    // this class before its superclass implementation has been added to the vtable
    // in the superclass's own constructor
    connect(_controlChannel, SIGNAL(stateChanged(Channel*, Channel::State)),
            this, SLOT(controlChannelStateChanged(Channel*, Channel::State)));

    if (_controlChannel->getState() == Channel::ErrorState) {
        LOG_E("The TCP channel could not be initialized");
    }

    if (!_mediaSocket->bind(host)) {
        LOG_E("Failed to bind to UDP socket");
    }

    _mediaSocket->open(QIODevice::ReadWrite);

    START_TIMER(_calculateBitrateTimerId, 1000);
}

MediaClient::~MediaClient() {
    if (_controlChannel) {
        disconnect(_controlChannel, 0, 0, 0);
        _controlChannel->close();
        delete _controlChannel;
    }
    if (_mediaSocket) {
        disconnect(_mediaSocket, 0, 0, 0);
        if (_mediaSocket->isOpen()) _mediaSocket->close();
        delete _mediaSocket;
    }
}

void MediaClient::addForwardingAddress(SocketAddress address) {
    foreach (SocketAddress existing, _forwardAddresses) {
        if (existing == address) return;
    }
    _forwardAddresses.append(address);
}

void MediaClient::removeForwardingAddress(SocketAddress address) {
    int index = _forwardAddresses.indexOf(address);
    if (index >= 0) {
        _forwardAddresses.removeAt(index);
    }
}

void MediaClient::controlMessageReceived(Channel *channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(size); Q_UNUSED(channel);
    QByteArray byteArray = QByteArray::fromRawData(message, size);
    QDataStream stream(byteArray);
    stream.setByteOrder(QDataStream::BigEndian);
    QString messageType;
    stream >> messageType;
    LOG_E("Got message: " + messageType);
    if (messageType.compare("start", Qt::CaseInsensitive) == 0) {
        LOG_I("Server has notified us of a new media stream");
        disconnect(_mediaSocket, SIGNAL(readyRead()), 0, 0);
        START_TIMER(_punchTimerId, 100);
        onServerStartMessageInternal();
        setState(ConnectedState);
    }
    else if (messageType.compare("streaming", Qt::CaseInsensitive) == 0) {
        // we were successful and are now receiving a media stream
        LOG_I("Server has confirmed our address and should begin streaming");
        _errorString = ""; // clear error string since we have an active connection;
        mediaSocketReadyRead();
        connect(_mediaSocket, SIGNAL(readyRead()),
                this, SLOT(mediaSocketReadyRead()));
        KILL_TIMER(_punchTimerId);
        onServerStreamingMessageInternal(stream);
        setState(StreamingState);
    }
    else if (messageType.compare("eos", Qt::CaseInsensitive) == 0) {
        LOG_I("Got EOS message from server");
        KILL_TIMER(_punchTimerId);
        disconnect(_mediaSocket, SIGNAL(readyRead()), 0, 0);
        _lastBitrate = 0;
        onServerEosMessageInternal();
        setState(ConnectedState);
    }
    else if (messageType.compare("error", Qt::CaseInsensitive) == 0) {
        stream >> _errorString;
        LOG_I("Got error message from server: " + _errorString);
        disconnect(_mediaSocket, SIGNAL(readyRead()), 0, 0);
        _lastBitrate = 0;
        KILL_TIMER(_punchTimerId);
        onServerErrorMessageInternal();
        setState(ConnectedState);
    }
    else {
        LOG_E("Got unknown message from media server");
    }
}

void MediaClient::mediaSocketReadyRead() {
    qint64 size;
    while (_mediaSocket->hasPendingDatagrams()) {
        size = _mediaSocket->readDatagram(_buffer, 65536);
        // update bit total
        _bitCount += size * 8;
        // forward the datagram to all specified addresses
        foreach (SocketAddress address, _forwardAddresses) {
            _mediaSocket->writeDatagram(_buffer, size, address.host, address.port);
        }
    }
}

void MediaClient::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _punchTimerId) {
        LOG_I("punch timer tick");
        // send data to the the server so it can figure out our address
        QByteArray message;
        QDataStream stream(&message, QIODevice::WriteOnly);
        stream << QString("soro_media");
        stream << _mediaId;
        _mediaSocket->writeDatagram(message.constData(), message.size(), _server.host, _server.port);
    }
    else if (e->timerId() == _calculateBitrateTimerId) {
        // this timer runs twice per second to calculate the bitrate received by the client
        _lastBitrate = _bitCount;
        _bitCount = 0;
    }
}

void MediaClient::controlChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    switch (state) {
    case Channel::ConnectedState:
        setState(ConnectedState);
        onServerConnectedInternal();
        break;
    default:
        setState(ConnectingState);
        disconnect(_mediaSocket, SIGNAL(readyRead()), 0, 0);
        KILL_TIMER(_punchTimerId);
        onServerDisconnectedInternal();
        break;
    }
}

QString MediaClient::getErrorString() const {
    return _errorString;
}

int MediaClient::getMediaId() const {
    return _mediaId;
}

MediaClient::State MediaClient::getState() const {
    return _state;
}

SocketAddress MediaClient::getServerAddress() const {
    return _server;
}

SocketAddress MediaClient::getHostAddress() const {
    return SocketAddress(_mediaSocket->localAddress(), _mediaSocket->localPort());
}

int MediaClient::getBitrate() const {
    return _lastBitrate;
}

void MediaClient::setState(State state) {
    if (_state != state) {
        _state = state;
        emit stateChanged(this, _state);
    }
}


} // namespace MissionControl
} // namespace Soro
