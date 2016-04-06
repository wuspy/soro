#ifndef MBEDCHANNEL_H
#define MBEDCHANNEL_H

#ifdef QT_CORE_LIB
#   include <QtCore>
#   include <QUdpSocket>
#   include <QHostAddress>
#   include "socketaddress.h"
#   include "logger.h"
#endif
#ifdef TARGET_LPC1768
#   include "mbed.h"
#   include "EthernetInterface.h"
#   include "UDPSocket.h"
#endif

#include <cstring>
#include "soro_global.h"

namespace Soro {

#ifdef QT_CORE_LIB

class MbedChannel: public QObject {
    Q_OBJECT

public:
    enum State {
        ConnectedState, ConnectingState
    };

private:
    QString LOG_TAG;
    Logger *_log;
    QUdpSocket *_socket;
    SocketAddress _host;
    SocketAddress _peer;
    State _state;
    char _buffer[512];
    bool _active;
    char _mbedId;
    unsigned int _lastReceiveId;
    unsigned int _nextSendId = 0;
    int _watchdogTimerId = TIMER_INACTIVE;
    int _resetConnectionTimerId = TIMER_INACTIVE;

    void setChannelState(MbedChannel::State state) {
        if (_state != state) {
            _state = state;
            emit stateChanged(state);
        }
    }

private slots:
    void socketError(QAbstractSocket::SocketError err) {
        LOG_E("Error: " + _socket->errorString());
        START_TIMER(_resetConnectionTimerId, 500);
    }

    void socketReadyRead() {
        qint64 length;
        while (_socket->hasPendingDatagrams()) {
            length = _socket->readDatagram(&_buffer[0], 512, &_peer.host, &_peer.port);
            if (length < 5) continue;
            if (_buffer[0] != reinterpret_cast<char&>(_mbedId)) {
                LOG_W("Recieved message from incorrect mbed ID "
                      + QString::number(reinterpret_cast<unsigned char&>(_buffer[0])));
            }
            unsigned int sequence;
            deserialize<unsigned int>(_buffer + 1, sequence);
            if (_state == ConnectingState) {
                LOG_I("Connected to mbed client");
                setChannelState(ConnectedState);
            }
            else if (sequence < _lastReceiveId) continue;
            _lastReceiveId = sequence;
            _active = true;
            if (length > 5) {
                emit messageReceived(_buffer + 5, length - 5);
            }
        }
    }

    void resetConnection() {
        LOG_I("Connection is resetting...");
        _lastReceiveId = 0;
        _active = false;
        _peer.host = QHostAddress::Null;
        _peer.port = 0;
        _socket->abort();
        if (_socket->bind(_host.host, _host.port)) {
            _socket->open(QIODevice::ReadWrite);
        }
    }

public:
    MbedChannel(SocketAddress host, char mbedId, Logger *log = NULL) {
        _host = host;
        _socket = new QUdpSocket(this);
        _mbedId = mbedId;
        _log = log;
        LOG_TAG = "Mbed(" + QString::fromLatin1(&mbedId, 1) + ")";
        connect(_socket, SIGNAL(readyRead()),
                this, SLOT(socketReadyRead()));
        connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(socketError(QAbstractSocket::SocketError)));
        resetConnection();
        START_TIMER(_watchdogTimerId, 2000);
    }

    ~MbedChannel() {
        _socket->abort();
        delete _socket;
    }

    void sendMessage(const char *message, int length) {
        if ((_state == ConnectedState) && (length < 500)) {
            _buffer[0] = '\0';
            serialize<unsigned int>(_buffer + 1, _nextSendId++);
            memcpy(_buffer + 5, message, length);
            _socket->writeDatagram(message, length + 5, _peer.host, _peer.port);
        }
    }

signals:
    void messageReceived(const char* message, int length);
    void stateChanged(MbedChannel::State state);

protected:
    void timerEvent(QTimerEvent *e) {
        QObject::timerEvent(e);
        if (e->timerId() == _watchdogTimerId) {
            if ((_state == ConnectedState) & !_active) {
                LOG_E("Mbed client has timed out");
                setChannelState(ConnectingState);
            }
            _active = false;
        }
    }

};

#endif
#ifdef TARGET_LPC1768

class MbedChannel {
private:
    EthernetInterface *_eth;
    DigitalOut *_led;
    UDPSocket *_socket;
    Endpoint _server;
    time_t _lastSendTime;
    unsigned int _nextSendId;
    char _mbedId;
    char _buffer[512];

public:
    MbedChannel(const char *serverHost, int serverPort, char mbedId, bool manageEthernet) {
        _mbedId = mbedId;
        _server.set_address(serverHost, serverPort);
        _led = new DigitalOut(LED4);
        if (manageEthernet) {
            _eth = new EthernetInterface;
            while (_eth->init() < 0) { wait(1); }
            while (!_eth->connect() < 0) { wait(1); }
        }
        else {
            _eth = NULL;
        }
        _socket = new UDPSocket;
        while (_socket->init() < 0) { wait(1); }
        _lastSendTime = time(NULL);
        *_led = 1;
    }

    ~MbedChannel() {
        _led = 0;
        delete _led;
        _socket->close();
        delete _socket;
        if (_eth != NULL) {
            //this class is managing the ethernet
            _eth->disconnect();
            delete _eth;
        }
    }

    void sendMessage(char *message, int length) {
        _buffer[0] = _mbedId;
        serialize<unsigned int>(_buffer + 1, _nextSendId++);
        memcpy(_buffer + 5, message, length);
        _socket->sendTo(_server, message, length + 5);
        _lastSendTime = time(NULL);
    }

    int read(char *buffer, int maxLength) {
        Endpoint peer;
        int len = _socket->receiveFrom(peer, buffer, maxLength);
        if (time(NULL) - _lastSendTime >= 1) {
            //send heartbeat
            sendMessage(NULL, 0);
        }
        if (peer.get_port() == _server.get_port()) {
            return len;
        }
        return -1;
    }
};

#endif

}
#endif // MBEDCHANNEL_H
