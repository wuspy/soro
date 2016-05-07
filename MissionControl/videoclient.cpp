#include "videoclient.h"

#define LOG_TAG _name + "(C)"

namespace Soro {
namespace MissionControl {

VideoClient::VideoClient(QString name, SocketAddress server, QHostAddress host, Logger *log, QObject *parent) : QObject(parent) {
    _name = name;
    _server = server;
    _log = log;

    LOG_I("Creating new video client for server at " + server.toString());

    _controlChannel = new Channel(this, _server, _name, Channel::TcpProtocol, host, _log);
    _videoSocket = new QUdpSocket(this);

    _buffer = new char[65536];

    connect(_controlChannel, SIGNAL(messageReceived(Channel*, const char*, Channel::MessageSize)),
            this, SLOT(controlMessageReceived(Channel*, const char*, Channel::MessageSize)));
    connect(_controlChannel, SIGNAL(stateChanged(Channel*, Channel::State)),
            this, SLOT(controlChannelStateChanged(Channel*, Channel::State)));

    _controlChannel->open();

    if (!_videoSocket->bind(host)) {
        LOG_E("Failed to bind to UDP socket");
    }

    _videoSocket->open(QIODevice::ReadWrite);

    START_TIMER(_calculateBitrateTimerId, 500);
}

VideoClient::~VideoClient() {
    endOfStream();
    if (_controlChannel) {
        _controlChannel->close();
        delete _controlChannel;
    }
    if (_videoSocket) {
        if (_videoSocket->isOpen()) _videoSocket->close();
        delete _videoSocket;
    }
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

StreamFormat VideoClient::getStreamFormat() const {
    return _format;
}

void VideoClient::controlMessageReceived(Channel *channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(size); Q_UNUSED(channel);
    QByteArray byteArray = QByteArray::fromRawData(message, size);
    QDataStream stream(byteArray);
    stream.setByteOrder(QDataStream::BigEndian);
    QString messageType;
    stream >> messageType;
    LOG_E("Got message: " + messageType);
    if (messageType.compare("start", Qt::CaseInsensitive) == 0) {
        LOG_I("Server has notified us of a new video stream");
        _format.Encoding = UnknownOrNoEncoding;
        disconnect(_videoSocket, SIGNAL(readyRead()), 0, 0);
        START_TIMER(_punchTimerId, 100);
        setState(ConnectedState);
    }
    else if (messageType.compare("streaming", Qt::CaseInsensitive) == 0) {
        // we were successful and are now receiving a video stream
        LOG_I("Server has confirmed our address and should begin streaming");
        videoSocketReadyRead();
        connect(_videoSocket, SIGNAL(readyRead()),
                this, SLOT(videoSocketReadyRead()));
        stream >> reinterpret_cast<quint32&>(_format.Encoding);
        stream >> _format.Width;
        stream >> _format.Height;
        stream >> _format.Framerate;
        switch (_format.Encoding) {
        case MjpegEncoding:
            stream >> _format.Mjpeg_Quality;
            break;
        case Mpeg2Encoding:
            stream >> _format.Mpeg2_Bitrate;
            break;
        default:
            LOG_E("Metadata from server specifies an unknown encoding");
            break;
        }
        KILL_TIMER(_punchTimerId);
        setState(StreamingState);
    }
    else if (messageType.compare("eos", Qt::CaseInsensitive) == 0) {
        LOG_I("Got EOS message from server");
        _format.Encoding = UnknownOrNoEncoding;
        KILL_TIMER(_punchTimerId);
        disconnect(_videoSocket, SIGNAL(readyRead()), 0, 0);
        setState(ConnectedState);
    }
    else if (messageType.compare("error", Qt::CaseInsensitive) == 0) {
        QString errorMessage;
        stream >> errorMessage;
        LOG_I("Got error message from server: " + errorMessage);
        _format.Encoding = UnknownOrNoEncoding;
        disconnect(_videoSocket, SIGNAL(readyRead()), 0, 0);
        KILL_TIMER(_punchTimerId);
        emit serverError(this, errorMessage);
        setState(ConnectedState);
    }
    else {
        LOG_E("Got unknown message from video server");
    }
}

void VideoClient::videoSocketReadyRead() {
    qint64 size;
    while (_videoSocket->hasPendingDatagrams()) {
        size = _videoSocket->readDatagram(_buffer, 65536);
        // update bit total
        _bitCount += size * 8;
        // push the data to the gstreamer element src pad
        if (_needsData) {
            QGst::BufferPtr ptr = QGst::Buffer::create(size);
            QGst::MapInfo memory;
            ptr->map(memory, QGst::MapWrite);
            memcpy(memory.data(), _buffer, size);
            ptr->unmap(memory);
            pushBuffer(ptr);
        }
        // forward the datagram to all specified addresses
        foreach (SocketAddress address, _forwardAddresses) {
            _videoSocket->writeDatagram(_buffer, size, address.host, address.port);
        }
    }
}

void VideoClient::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _punchTimerId) {
        LOG_I("punch timer tick");
        // send data to the the server so it can figure out our address
        _videoSocket->writeDatagram(_name.toLatin1().constData(), _name.size() + 1, _server.host, _server.port);
    }
    else if (e->timerId() == _calculateBitrateTimerId) {
        // this timer runs twice per second to calculate the bitrate received by the client
        _lastBitrate = _bitCount * 2;
        _bitCount = 0;
        emit statisticsUpdate(this, _lastBitrate);
    }
}

void VideoClient::controlChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    switch (state) {
    case Channel::ConnectedState:
        setState(ConnectedState);
        break;
    default:
        setState(ConnectingState);
        _format.Encoding = UnknownOrNoEncoding;
        disconnect(_videoSocket, SIGNAL(readyRead()), 0, 0);
        KILL_TIMER(_punchTimerId);
        break;
    }
}

QString VideoClient::getCameraName() const {
    return _name;
}

void VideoClient::needData(uint length) {
    Q_UNUSED(length);
    _needsData = true;
}

void VideoClient::enoughData() {
    _needsData = false;
}

VideoClient::State VideoClient::getState() const {
    return _state;
}

SocketAddress VideoClient::getServerAddress() const {
    return _server;
}

SocketAddress VideoClient::getHostAddress() const {
    return SocketAddress(_videoSocket->localAddress(), _videoSocket->localPort());
}

void VideoClient::setState(State state) {
    if (_state != state) {
        _state = state;
        emit stateChanged(this, _state);
    }
}

void VideoClient::setCameraName(QString name) {
    if (_name.compare(name) != 0) {
        _name = name;
        emit nameChanged(this, _name);
    }
}

} // namespace MissionControl
} // namespace Soro
