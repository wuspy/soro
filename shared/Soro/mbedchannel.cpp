/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

#include "mbedchannel.h"

#define IDLE_CONNECTION_TIMEOUT 2000
#define BROADCAST_PACKET "MbedChannel"
#define _MBED_MSG_TYPE_NORMAL 1
#define _MBED_MSG_TYPE_LOG 2
#define _MBED_MSG_TYPE_BROADCAST 3
#define _MBED_MSG_TYPE_HEARTBEAT 4

namespace Soro {

#ifdef QT_CORE_LIB

void MbedChannel::setChannelState(MbedChannel::State state) {
    if (_state != state) {
        _state = state;
        emit stateChanged(this, state);
    }
}

void MbedChannel::socketError(QAbstractSocket::SocketError err) {
    Q_UNUSED(err);
    LOG_E("Error: " + _socket->errorString());
    START_TIMER(_resetConnectionTimerId, 500);
}

void MbedChannel::socketReadyRead() {
    qint64 length;
    while (_socket->hasPendingDatagrams()) {
        length = _socket->readDatagram(&_buffer[0], 512, &_peer.host, &_peer.port);
        if ((length < 6) | (length == 512)) continue;
        if (_buffer[0] != reinterpret_cast<char&>(_mbedId)) {
            LOG_W("Recieved message from incorrect mbed ID "
                  + QString::number(reinterpret_cast<unsigned char&>(_buffer[0])));
            continue;
        }
        unsigned int sequence = deserialize<unsigned int>(_buffer + 2);
        if (_state == ConnectingState) {
            LOG_I("Connected to mbed client");
            setChannelState(ConnectedState);
        }
        else if (sequence < _lastReceiveId) continue;
        _lastReceiveId = sequence;
        _active = true;
        switch (reinterpret_cast<unsigned char&>(_buffer[1])) {
        case _MBED_MSG_TYPE_NORMAL:
            if (length > 6) {
                emit messageReceived(_buffer + 6, length - 6);
            }
            break;
        case _MBED_MSG_TYPE_LOG:
            LOG_I("Mbed:" + QString(_buffer + 6));
            break;
        case _MBED_MSG_TYPE_BROADCAST:
            _socket->writeDatagram(BROADCAST_PACKET, strlen(BROADCAST_PACKET) + 1, _peer.host, _peer.port);
            break;
        case _MBED_MSG_TYPE_HEARTBEAT:
            break;
        default:
            LOG_E("Got message with unknown type");
            break;
        }
    }
}

void MbedChannel::resetConnection() {
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

MbedChannel::MbedChannel(SocketAddress host, unsigned char mbedId, QObject *parent, Logger *log) : QObject(parent) {
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
    START_TIMER(_watchdogTimerId, IDLE_CONNECTION_TIMEOUT);
}

MbedChannel::~MbedChannel() {
    _socket->abort();
    delete _socket;
}

void MbedChannel::sendMessage(const char *message, int length) {
    if ((_state == ConnectedState) && (length < 500)) {
        _buffer[0] = '\0';
        serialize<unsigned int>(_buffer + 1, _nextSendId++);
        memcpy(_buffer + 5, message, length);
        _socket->writeDatagram(_buffer, length + 5, _peer.host, _peer.port);
    }
}

void MbedChannel::timerEvent(QTimerEvent *e) {
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

MbedChannel::State MbedChannel::getState() const {
    return _state;
}

#endif
#ifdef TARGET_LPC1768

extern "C" void mbed_reset();

void MbedChannel::panic() {
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

void MbedChannel::reset() {
    if (_resetCallback != NULL) {
        _resetCallback();
    }
    mbed_reset();
}

//Old method that read the actual IP from the file as well
/*void MbedChannel::loadConfig() {
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
}*/

int MbedChannel::loadPort() {
    LocalFileSystem local("local");
    FILE *configFile = fopen("/local/server.txt", "r");
    if (configFile != NULL) {
        char line[12];
        fgets(&line[0], 12, configFile);
        fclose(configFile);
        int port = (int)atoi(line);
        if ((port > 0) | (port <= 65535)) {
            return port;
        }
    }
    //an error occurred
    panic();
    return 0; //never actually reached
}

void MbedChannel::initConnection() {
    //initialize ethernet interface
    DigitalOut led1(LED1);
    DigitalOut led2(LED2);
    DigitalOut led3(LED3);
    DigitalOut led4(LED4);
    led1 = 1;
    if (_eth->init() != 0) {
        wait(0.5);
        reset();
    }
    //connect to ethernet and get address through DHCP
    led2 = 1;
    if (_eth->connect() != 0) {
        wait(0.5);
        reset();
    }
    led3 = 1;
    //now that we have an IP, set the broadcast address
    //and read from the config file what port we should broadcast on
    char *ip = _eth->getIPAddress();
    char *broadcast = new char[strlen(ip) + 3]; //make sure we have enough room to add 2 more digits
    strcpy(broadcast, ip);
    strcpy(strrchr(broadcast, '.'), ".255");
    _server.set_address(broadcast, loadPort());
    setTimeout(IDLE_CONNECTION_TIMEOUT / 3);
    //initialize socket
    while (_socket->init() != 0) {
        wait(0.2);
        led3 = 0;
        wait(0.2);
        led3 = 1;
    }
    if (_socket.set_broadcasting(true) != 0) {
        wait(0.2);
        led3 = 0;
        wait(0.2);
        led3 = 1;
        reset();
    }
    //find server through LAN broadcast
    led4 = 1;
    Endpoint peer;
    int packet_len = strlen(BROADCAST_PACKET) + 1;
    char buffer[packet_len];
    while (1) {
        //send broadcast handshake
        sendMessage(BROADCAST_PACKET, packet_len, _MBED_MSG_TYPE_BROADCAST);
        while (1) {
            //recieve any responses
            int len = _socket->receiveFrom(peer, &buffer[0], packet_len);
            if (len <= 0) break;
            if ((len == packet_len) && (strcmp(&buffer[0], BROADCAST_PACKET) == 0))  {
                //received a response from the server
                _server = peer;
                led1 = 0;
                led2 = 0;
                led3 = 0;
                led4 = 0;
                _socket.set_broadcasting(false);
                return;
            }
        }
        wait(0.2);
        led4 = 0;
        wait(0.2);
        led4 = 1;
    }
}

MbedChannel::MbedChannel(unsigned char mbedId) {
    _resetCallback = NULL;
    _mbedId = reinterpret_cast<char&>(mbedId);
    _eth = new EthernetInterface;
    _socket = new UDPSocket;
    initConnection();
    _lastSendTime = time(NULL);
    _lastReceiveId = 0;
    _nextSendId = 0;
}

MbedChannel::~MbedChannel() {
    _socket->close();
    delete _socket;
    //this class is managing the ethernet
    _eth->disconnect();
    delete _eth;
}

void MbedChannel::setTimeout(unsigned int millis) {
    if (millis < IDLE_CONNECTION_TIMEOUT / 2)
        _socket->set_blocking(false, millis);
}

void MbedChannel::sendMessage(char *message, int length, unsigned char type) {
    if (!isEthernetActive()) {
        reset();
    }
    _buffer[0] = _mbedId;
    _buffer[1] = reinterpret_cast<char&>(type);
    serialize<unsigned int>(_buffer + 2, _nextSendId++);
    memcpy(_buffer + 6, message, length);
    _socket->sendTo(_server, _buffer, length + 6);
    _lastSendTime = time(NULL);
}

void MbedChannel::setResetListener(void (*callback)(void)) {
    _resetCallback = callback;
}

int MbedChannel::read(char *outMessage, int maxLength) {
    if (!isEthernetActive()) {
        reset();
    }
    Endpoint peer;
    //Check if a heartbeat should be sent
    if (time(NULL) - _lastSendTime >= 1) {
        sendMessage(NULL, 0, _MBED_MSG_TYPE_HEARTBEAT);
    }
    int len = _socket->receiveFrom(peer, _buffer, maxLength);
    unsigned int sequence = deserialize<unsigned int>(_buffer + 1);
    if ((len < 5) || (sequence < _lastReceiveId) || (peer.get_port() != _server.get_port())) return -1;
    _lastReceiveId = sequence;
    memcpy(outMessage, _buffer + 5, len - 5);
    return len - 5;
}
#endif

}
