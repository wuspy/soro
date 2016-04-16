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
#   include "lpc_phy.h"
extern "C" void mbed_reset();
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
            LOG_I("Got message with length " + QString::number(length));
            if ((length < 5) | (length == 512)) continue;
            if (_buffer[0] != reinterpret_cast<char&>(_mbedId)) {
                LOG_W("Recieved message from incorrect mbed ID "
                      + QString::number(reinterpret_cast<unsigned char&>(_buffer[0])));
                continue;
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
        setChannelState(ConnectingState);
        _lastReceiveId = 0;
        _active = false;
        _peer.host = QHostAddress::Null;
        _peer.port = 0;
        _socket->abort();
        if (_socket->isOpen()) _socket->close();
        if (_socket->bind(_host.host, _host.port)) {
            LOG_I("Listening on UDP port " + _host.toString());
            _socket->open(QIODevice::ReadWrite);
        }
        else {
            LOG_E("Failed to bind to " + _host.toString());
        }
    }

public:
    MbedChannel(SocketAddress host, unsigned char mbedId, Logger *log = NULL) {
        _host = host;
        _socket = new QUdpSocket(this);
        _mbedId = reinterpret_cast<char&>(mbedId);
        _log = log;
        LOG_TAG = "Mbed(" + QString::number(mbedId) + ")";
        LOG_I("Creating new mbed channel");
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
        else if (e->timerId() == _resetConnectionTimerId) {
            resetConnection();
            KILL_TIMER(_resetConnectionTimerId); //single shot
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
    unsigned int _lastReceiveId;
    char _mbedId;
    char _buffer[512];
    void (*_resetCallback)(void);

    void panic() {
        DigitalOut led1(LED1);
        DigitalOut led2(LED2);
        DigitalOut led3(LED3);
        DigitalOut led4(LED4);
        while (1) {
            led1 = 1;
            led2 = 0;
            led3 = 0;
            led4 = 1;
            wait_ms(150);
            led1 = 0;
            led2 = 1;
            led3 = 1;
            led4 = 0;
            wait_ms(150);
        }
    }

    void loadConfig() {
        Serial pc(USBTX, USBRX);
        LocalFileSystem local("local");
        FILE *configFile = fopen("/local/server.txt", "r");
        if (configFile != NULL) {
            char *line = new char[64];
            fgets(line, 64, configFile);
            fclose(configFile);
            for (int i = 0; line[i] != '\0'; i++) {
                if (line[i] == ':') {
                    char *ip = new char[i + 1];
                    strncpy(ip, line, i);
                    ip[i] = '\0';
                    unsigned int port = (unsigned int)atoi(line + i + 1);
                    if ((port == 0) | (port > 65535)) break;
                    _server.set_address(ip, port);
                    return;
                }
            }
        }
        //an error occurred
        panic();
    }

    inline void initConnection() {
        while (_eth->init() != 0) {
             *_led = 1; wait_ms(100);
             *_led = 0; wait_ms(100);
        }
        while (_eth->connect() != 0) {
            *_led = 1; wait_ms(100);
            *_led = 0; wait_ms(50);
            *_led = 1; wait_ms(100);
            *_led = 0; wait_ms(50);
        }
        while (_socket->init() != 0) {
            *_led = 1; wait_ms(100);
            *_led = 0; wait_ms(50);
            *_led = 1; wait_ms(100);
            *_led = 0; wait_ms(50);
            *_led = 1; wait_ms(100);
            *_led = 0; wait_ms(50);
        }
    }

    inline bool isEthernetActive () {
        return (lpc_mii_read_data() & (1 << 0)) ? true : false;
    }

public:
    /* Creates a new channel for ethernet communication.
     *
     * This will read from a file on local storage (server.txt) to
     * determine the address it should communicate with.
     *
     * Blocks until completion, often for several seconds.
     */
    MbedChannel(unsigned char mbedId) {
        _resetCallback = NULL;
        _led = new DigitalOut(LED4);
        loadConfig();
        _mbedId = reinterpret_cast<char&>(mbedId);
        _eth = new EthernetInterface;
        _socket = new UDPSocket;
        initConnection();
        _lastSendTime = time(NULL);
        _lastReceiveId = 0;
        _nextSendId = 0;
        *_led = 1;
    }

    ~MbedChannel() {
        _led = 0;
        delete _led;
        _socket->close();
        delete _socket;
        //this class is managing the ethernet
        _eth->disconnect();
        delete _eth;
    }

    /* Sets the timeout for blocking socket operations in milliseconds.
    */
    void setTimeout(unsigned int millis) {
        _socket->set_blocking(false, millis);
    }

    void sendMessage(char *message, int length) {
        if (!isEthernetActive()) {
            if (_resetCallback != NULL) {
                _resetCallback();
            }
            mbed_reset();
        }
        _buffer[0] = _mbedId;
        serialize<unsigned int>(_buffer + 1, _nextSendId++);
        memcpy(_buffer + 5, message, length);
        _socket->sendTo(_server, _buffer, length + 5);
        _lastSendTime = time(NULL);
    }

    /* Adds a listener to be notified if the ethernet
     * becomes disconnected, which will result in the mbed
     * resetting. This gives you a chance to clean up
     * or perform any necessary operations before a reset.
     */
    void setResetListener(void (*callback)(void)) {
        _resetCallback = callback;
    }

    /* Reads a pending message (if available) and stores it in outMessage.
     *
     * Returns the length of the message, or -1 if none is available or
     * an error occurred.
     */
    int read(char *outMessage, int maxLength) {
        if (!isEthernetActive()) {
            if (_resetCallback != NULL) {
                _resetCallback();
            }
            mbed_reset();
        }
        Endpoint peer;
        //Check if a heartbeat should be sent
        if (time(NULL) - _lastSendTime >= 1) {
            sendMessage(NULL, 0);
        }
        int len = _socket->receiveFrom(peer, _buffer, maxLength);
        unsigned int sequence;
        deserialize<unsigned int>(_buffer + 1, sequence);
        if ((len < 5) || (sequence < _lastReceiveId) || (peer.get_port() != _server.get_port())) return -1;
        _lastReceiveId = sequence;
        memcpy(outMessage, _buffer + 5, len - 5);
        return len - 5;
    }
};

#endif

}
#endif // MBEDCHANNEL_H
