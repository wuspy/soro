#include "channel.h"

//rough rate at which handshakes are sent when trying to establish a UDP connection
#define HANDSHAKE_FREQUENCY 250
//timeout for dropping a connection with no received packets
#define IDLE_CONNECTION_TIMEOUT 5000
//rough rate at which this channel should send the other side an ack for their last packet
#define STATISTICS_INTERVAL 500
//rough rate at which heartbeats should be sent if no other packets are being sent
#define HEARTBEAT_INTERVAL 500
//number of sent entries to log for rtt calculation
#define SENT_LOG_CAP 300
//delay after an error when a reconnect will be tried
#define RECOVERY_DELAY 1000

//Tags for writing the configuration file
#define CONFIG_TAG_SERVER_ADDRESS "serveraddress"
#define CONFIG_TAG_SERVER_PORT "serverport"
#define CONFIG_TAG_CHANNEL_NAME "name"
#define CONFIG_TAG_PROTOCOL "protocol"
#define CONFIG_TAG_HOST_ADDRESS "hostaddress"
#define CONFIG_TAG_ENDPOINT "endpoint"
#define CONFIG_TAG_DROP_OLD_PACKETS "dropoldpackets"
#define CONFIG_TAG_LOW_DELAY "lowdelay"
#define CONFIG_TAG_SEND_ACKS "sendacks"

/*  Constructors and destructor
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

namespace Soro {

Channel::Channel(QObject *parent) : QObject(parent) { }

Channel* Channel::createClient(QObject *parent, SocketAddress serverAddress, QString name, Protocol protocol,
                 QHostAddress hostAddress){
    Channel *c = new Channel(parent);
    c->_serverAddress = serverAddress;
    c->_protocol = protocol;
    c->_isServer = false;
    c->_hostAddress.host = hostAddress;
    c->_name = name;

    c->init();

    return c;
}

Channel* Channel::createServer(QObject *parent, quint16 port, QString name, Protocol protocol,
                 QHostAddress hostAddress) {
    Channel *c = new Channel(parent);
    c->_serverAddress.host = QHostAddress::Any;
    c->_serverAddress.port = port;
    c->_protocol = protocol;
    c->_isServer = true;
    c->_hostAddress.host = hostAddress;
    c->_name = name;

    c->init();

    return c;
}

Channel::~Channel() {
    if (_sentTimeLog != NULL) {
        delete [] _sentTimeLog;
    }
    if (_tcpSocket) {
        delete _tcpSocket;
        _tcpSocket = NULL;
    }
    if (_udpSocket) {
        delete _udpSocket;
        _udpSocket = NULL;
    }
    if (_tcpServer) {
        delete _tcpServer;
        _tcpServer = NULL;
    }
    if (_nameUtf8) {
        delete [] _nameUtf8;
    }
    _socket = NULL;
}

/*  Initialization, creates timers, sockets, apply configuration
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

void Channel::init() {  //PRIVATE
    //log tag for debugging
    LOG_TAG = _name + (_isServer ? "(S)" : "(C)");

    //format the name as a UTF8 byte array
    QByteArray toUtf8 = _name.toUtf8();
    _nameUtf8 = new char[toUtf8.size() + 1];
    strcpy(_nameUtf8, toUtf8.constData());
    _nameUtf8Size = strlen(_nameUtf8) + 1; //include \0 char

    //create a buffer for storing received messages
    _sentTimeLog = new qint64[SENT_LOG_CAP];

    LOG_I(LOG_TAG, "Initializing with serverAddress=" + _serverAddress.toString()
          + ",protocol=" + (_protocol == TcpProtocol ? "TCP" : "UDP"));

    //create socket and connect signals
    if (_protocol == UdpProtocol) {
        //Clients and servers for UDP communication function similarly (unlike TCP)
        _socket = _udpSocket = new QUdpSocket(this);
        _socket->setSocketOption(QAbstractSocket::LowDelayOption, _lowDelaySocketOption);
        connect(_socket, SIGNAL(readyRead()), this, SLOT(udpReadyRead()));
        connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(connectionErrorInternal(QAbstractSocket::SocketError)));
    }
    else if (_isServer) {
        //Server for a TCP connection; we must use a QTcpServer to manage connecting peers
        _tcpServer = new QTcpServer(this);
        connect(_tcpServer, SIGNAL(newConnection()), this, SLOT(newTcpClient()));
        connect(_tcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)),
                this, SLOT(serverErrorInternal(QAbstractSocket::SocketError)));
    }
    else {
        //Client for a TCP connection
        _socket = _tcpSocket = new QTcpSocket(this);
        _socket->setSocketOption(QAbstractSocket::LowDelayOption, _lowDelaySocketOption);
        configureNewTcpSocket(); //This is its own function, as it must be called every time a new
                                 //TCP client connects in a server scenario
    }
    setChannelState(ReadyState, false); //now safe to call open()
}

/*  Connection lifecycle management
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

void Channel::open() {
    LOG_D(LOG_TAG, "open() called");
    switch (_state) {
    case ReadyState:
        //verify the name length
        if (_nameUtf8Size > 64) {
            LOG_E(LOG_TAG, "Name is too long (max 64 characters)");
            setChannelState(ErrorState, true);
            return;
        }
        //If this is the server, we will bind to the server port
        if (_isServer) {
            _hostAddress.port = _serverAddress.port;
            LOG_I(LOG_TAG, "Attempting to bind to " + _hostAddress.toString());
        }
        else {
            _hostAddress.port = 0;
            LOG_I(LOG_TAG, "Attempting to bind to an available port on " + _hostAddress.host.toString());
        }
        if (_tcpServer != NULL) {
            _tcpServer->setMaxPendingConnections(1);
        }
        //start the connection procedure
        resetConnection();
        break;
    default:
        LOG_E(LOG_TAG, "Cannot call open() in the current channel state");
        break;
    }
}

void Channel::resetConnectionVars() {   //PRIVATE
    LOG_D(LOG_TAG, "resetConnectionVars() called");
    while (!_delayPackets.empty()) {
        // Clear the delay packet queue
        PacketWrapper *next = _delayPackets.dequeue();
        delete next->data;
        delete next;
    }
    _receiveBufferLength = 0;
    _lastReceiveID = 0;
    _lastRtt = -1;
    _lastAckSendTime = 0;
    _lastAckReceiveTime = 0;
    _connectionEstablishedTime = QDateTime::currentMSecsSinceEpoch();
    _nextSendID = 1;
    _messagesDown = 0;
    _messagesUp = 0;
    _bytesUp = 0;
    _bytesDown = 0;
    _dataRateUp = 0;
    _dataRateDown = 0;
    _sentTimeLogIndex = 0;
}

void Channel::resetConnection() {   //PRIVATE
    LOG_I(LOG_TAG, "Attempting to connect to other side of channel...");
    resetConnectionVars();
    KILL_TIMER(_connectionMonitorTimerID);
    KILL_TIMER(_handshakeTimerID);
    KILL_TIMER(_resetTcpTimerID);
    KILL_TIMER(_resetTimerID);
    if (_isServer) {
        setPeerAddress(SocketAddress(QHostAddress::Null, 0));
    }
    else {
        //Only allow the server address to connect
        setPeerAddress(_serverAddress);
    }
    if (_tcpSocket != NULL) {
        LOG_D(LOG_TAG, "Cancelling any previous TCP operations...");
        _tcpSocket->abort();
        if (_isServer) {
            LOG_D(LOG_TAG, "Cleaning up TCP connection with old client...");
            _tcpSocket->close();
            delete _tcpSocket;
            _tcpSocket = NULL;
        }
        else {
            LOG_D(LOG_TAG, "Trying to establish TCP connection with server " + _serverAddress.toString());
            _tcpSocket->bind(_hostAddress.host, _hostAddress.port);
            if (!_tcpSocket->isOpen()) _tcpSocket->open(QIODevice::ReadWrite);
            _tcpSocket->connectToHost(_serverAddress.host, _serverAddress.port);
            LOG_I(LOG_TAG, "Bound to TCP port " + QString::number(_tcpSocket->localPort()));
        }
    }
    else if (_udpSocket != NULL) {
        LOG_D(LOG_TAG, "Cancelling any previous UDP operations...");
        _udpSocket->abort();
        _udpSocket->bind(_hostAddress.host, _hostAddress.port);
        if (!_udpSocket->isOpen()) _udpSocket->open(QIODevice::ReadWrite);
        if (!_isServer) START_TIMER(_handshakeTimerID, HANDSHAKE_FREQUENCY);
        LOG_I(LOG_TAG, "Bound to UDP port " + QString::number(_udpSocket->localPort()));
    }
    if (_tcpServer != NULL) {
        if (!_tcpServer->isListening()) {
            LOG_D(LOG_TAG, "Waiting for TCP client to connect on port " + QString::number(_hostAddress.port));
            _tcpServer->listen(_hostAddress.host, _hostAddress.port);
            LOG_I(LOG_TAG, "Listenign to TCP port " + QString::number(_tcpServer->serverPort()));
        }
        if (_tcpServer->hasPendingConnections()) {
            //allow the next pending connection since we just dropped the old one
            newTcpClient();
        }
    }

    setChannelState(ConnectingState, false);
}

void Channel::timerEvent(QTimerEvent *e) {  //PROTECTED
    int id = e->timerId();
    if (id == _connectionMonitorTimerID) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        //check for a stale connection (several seconds without a message)
        if (now - _lastReceiveTime >= IDLE_CONNECTION_TIMEOUT) {
            LOG_E(LOG_TAG, "Peer has stopped responding, dropping connection");
            resetConnection();
        }
        else if (now - _lastSendTime >= HEARTBEAT_INTERVAL) {
            //Send a heartbeat message, even on TCP (They are needed for RTT updates
            //and are a good idea anyway)
            sendHeartbeat();
        }
    }
    else if (id == _resetTcpTimerID) {
        LOG_E(LOG_TAG, "Connected TCP peer did not verify identity in time");
        resetConnection();
        KILL_TIMER(_resetTcpTimerID); //single shot
    }
    else if (id == _resetTimerID) {
        resetConnection();
        KILL_TIMER(_resetTimerID); //single shot
    }
    else if (id == _handshakeTimerID) {
        sendHandshake();
    }
    else if (!_delayPackets.empty()) {
        PacketWrapper* next = _delayPackets.dequeue();
        //This must be a delay send timer
        if (_state == ConnectedState) {
            if (_udpSocket != NULL) {
                _udpSocket->writeDatagram(next->data, next->len, _peerAddress.host, _peerAddress.port);
            }
            else if (_tcpSocket != NULL) {
                _tcpSocket->write(next->data, next->len);
            }
            delete next->data;
            delete next;
        }
        killTimer(e->timerId());
    }
}

void Channel::configureNewTcpSocket() { //PRIVATE
    //set the signals for a new TCP socket
    if (_tcpSocket) {
        _socket = _tcpSocket;
        LOG_I(LOG_TAG, "New Socket");
        _socket->setSocketOption(QAbstractSocket::LowDelayOption, _lowDelaySocketOption);
        connect(_socket, SIGNAL(readyRead()), this, SLOT(tcpReadyRead()));
        connect(_socket, SIGNAL(connected()), this, SLOT(tcpConnected()));
        connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(connectionErrorInternal(QAbstractSocket::SocketError)));
    }
}

void Channel::tcpConnected() {  //PRIVATE SLOT
    //Establishing a TCP connection does not mean this channel is connected.
    //Both sides of the channel must send handshakes to each other to verify identity.
    //If this message is not sent timely (within a few seconds), both sides will disconnect
    //and attempt the whole thing over again
    setPeerAddress(SocketAddress(_tcpSocket->peerAddress(), _tcpSocket->peerPort()));
    sendHandshake();
    //Close the connection if it is not verified in time
    START_TIMER(_resetTcpTimerID, IDLE_CONNECTION_TIMEOUT);
    LOG_I(LOG_TAG, "TCP peer " + _peerAddress.toString() + " has connected");

    setChannelState(ConnectingState, false);
}

void Channel::newTcpClient() {  //PRIVATE SLOT
    if (_tcpSocket != NULL) {
        _tcpSocket->abort();
        if (_tcpSocket->isOpen()) {
            _tcpSocket->close();
        }
        delete _tcpSocket;
    }
    _tcpSocket = _tcpServer->nextPendingConnection();
    configureNewTcpSocket();
    tcpConnected();
}

inline void Channel::setChannelState(Channel::State state, bool forceUpdate) {   //PRIVATE
    //signals the stateChanged event
     if ((_state != state) | forceUpdate) {
         LOG_D(LOG_TAG, "Setting state to " + QString::number(state));
         _state = state;
         emit stateChanged(this, _state);
     }
}

inline void Channel::setPeerAddress(Soro::SocketAddress address) {    //PRIVATE
    //signals the peerAddressChanged event
    if (_peerAddress != address) {
        LOG_D(LOG_TAG, "Setting peer address to " + address.toString());
        _peerAddress = address;
        emit peerAddressChanged(this, _peerAddress);
    }
}

void Channel::close() {
    close(ReadyState);
}

void Channel::close(Channel::State closeState) {   //PRIVATE
    if ((_state == ConnectedState) || (_state == ConnectingState)) {
        LOG_W(LOG_TAG, "Closing channel in state " + QString::number(closeState));
        if (_socket) {
            _socket->abort();
        }
        if (_tcpServer) {
            _tcpServer->close();
        }
        KILL_TIMER(_connectionMonitorTimerID);
        KILL_TIMER(_resetTcpTimerID);
        KILL_TIMER(_resetTimerID);
        KILL_TIMER(_handshakeTimerID);
        resetConnectionVars();

        setChannelState(closeState, false);
    }
}

/*  Socket error handling
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

void Channel::connectionErrorInternal(QAbstractSocket::SocketError err) { //PRIVATE SLOT
    emit connectionError(this, err);
    LOG_E(LOG_TAG, "Connection Error: " + _socket->errorString());
    //Attempt to reconnect after RECOVERY_DELAY
    //we should NOT directly call resetConnectio() here as that could
    //potentially force an error loop
    START_TIMER(_resetTimerID, RECOVERY_DELAY);
}

void Channel::serverErrorInternal(QAbstractSocket::SocketError err) { //PRIVATE SLOT
    emit connectionError(this, err);
    LOG_E(LOG_TAG, "Server Error: " + _tcpServer->errorString());
    //don't automatically kill the connection if it's still active, only the listening server
    //experienced the error
    if (_tcpSocket == NULL || _tcpSocket->state() != QAbstractSocket::ConnectedState) {
        START_TIMER(_resetTimerID, RECOVERY_DELAY);
    }
}

/*  Receiving methods, reading from the connected socket and processing
 *  messages
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

void Channel::udpReadyRead() {  //PRIVATE SLOT
    LOG_D(LOG_TAG, "udpReadyRead() called");
    SocketAddress address;
    MessageID ID;
    MessageType type;
    qint64 status;
    while (_udpSocket->hasPendingDatagrams()) {
        //read in a datagram
        status = _udpSocket->readDatagram(_receiveBuffer, MAX_MESSAGE_LENGTH, &address.host, &address.port);
        if (status < 0) {
            //an error occurred reading from the socket, the onSocketError slot will handle it
            return;
        }
        _receiveBufferLength = status;
        type = reinterpret_cast<MessageType&>(_receiveBuffer[0]);
        //ensure the datagram either came from the correct address, or is marked as a handshake
        if (_isServer) {
            if ((address != _peerAddress) & (type != MSGTYPE_CLIENT_HANDSHAKE)) {
                LOG_D(LOG_TAG, "Received non-handshake UDP packet from unknown peer");
                continue;
            }
        }
        else if ((address != _peerAddress) & (address != _serverAddress)) {
            LOG_D(LOG_TAG, "Received UDP packet that was not from server");
            continue;
        }
        _bytesDown += status;
        ID = deserialize<MessageID>(_receiveBuffer + 1);
        processBufferedMessage(type, ID, _receiveBuffer + UDP_HEADER_SIZE, _receiveBufferLength - UDP_HEADER_SIZE, address);
    }
}

void Channel::tcpReadyRead() {  //PRIVATE SLOT
    LOG_D(LOG_TAG, "tcpReadyRead() called");
    qint64 status;
    while (_tcpSocket->bytesAvailable() > 0) {
        if (_receiveBufferLength < TCP_HEADER_SIZE) {
            //read the header in first so we know how long the message should be
            status = _tcpSocket->read(_receiveBuffer + _receiveBufferLength, TCP_HEADER_SIZE - _receiveBufferLength);
            if (status < 0) {
                //an error occurred reading from the socket, the onSocketError slot will handle it
                return;
            }
            _receiveBufferLength += status;
        }
        if (_receiveBufferLength >= TCP_HEADER_SIZE) {
            //The header is in the buffer, so we know how long the packet is
            MessageSize length = deserialize<MessageSize>(_receiveBuffer);
            if (length > MAX_MESSAGE_LENGTH + TCP_HEADER_SIZE) {
                LOG_W(LOG_TAG, "TCP peer sent a message with an invalid header (length=" + QString::number(length) + ")");
                resetConnection();
                return;
            }
            //read the rest of the message (if it's all there)
            status = _tcpSocket->read(_receiveBuffer + _receiveBufferLength, length - _receiveBufferLength);
            if (status < 0) {
                //an error occurred reading from the socket, the onSocketError slot will handle it
                _receiveBufferLength = 0;
                return;
            }
            _receiveBufferLength += status;
            if (_receiveBufferLength == length) {
                //we have the whole message
                _bytesDown += length;
                MessageType type = reinterpret_cast<MessageType&>(_receiveBuffer[sizeof(MessageSize)]);
                MessageID ID = deserialize<MessageID>(_receiveBuffer + sizeof(MessageSize) + 1);
                processBufferedMessage(type, ID, _receiveBuffer + TCP_HEADER_SIZE, _receiveBufferLength - TCP_HEADER_SIZE, _peerAddress);
                _receiveBufferLength = 0;
            }
        }
    }
}

void Channel::processBufferedMessage(MessageType type, MessageID ID, const char *message, MessageSize size, const SocketAddress &address) {
    switch (type) {
    case MSGTYPE_NORMAL:
        //normal data packet
        //check the packet sequence ID
        if ((ID > _lastReceiveID) | !_dropOldPackets){
            LOG_D(LOG_TAG, "Received normal packet " + QString::number(ID));
            _lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
            _receivedPackets++;
            _droppedPackets += ID - _lastReceiveID + 1;
            _lastReceiveID = ID;
            emit messageReceived(this, message, size);
        }
        break;
    case MSGTYPE_SERVER_HANDSHAKE:
        //this packet is a handshake request
        LOG_D(LOG_TAG, "Received server handshake packet " + QString::number(ID));
        if (!_isServer) {
            if (compareHandshake(message, size)) {
                //we are the client, and we got a respoonse from the server (yay)
                resetConnectionVars();
                setPeerAddress(address);
                KILL_TIMER(_handshakeTimerID);
                KILL_TIMER(_resetTcpTimerID);
                _lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
                _lastReceiveID = ID;
                _wasConnected = true;
                START_TIMER(_connectionMonitorTimerID, (int)(HEARTBEAT_INTERVAL / 3.1415926));
                LOG_D(LOG_TAG, "Received handshake response from server " + _serverAddress.toString());

                setChannelState(ConnectedState, false);
            }
            else {
                LOG_W(LOG_TAG, "Received server handshake with invalid channel name");
            }
        }
        return; //don't calculate statistics on handshake messages
    case MSGTYPE_CLIENT_HANDSHAKE:
        if (_isServer) {
            LOG_D(LOG_TAG, "Received client handshake packet " + QString::number(ID));
            if (compareHandshake(message, size)) {
                //We are the server getting a new (valid) handshake request, respond back and record the address
                resetConnectionVars();
                setPeerAddress(address);
                KILL_TIMER(_resetTcpTimerID);
                if (_protocol == UdpProtocol) {
                    //send a handshake back in UDP server mode
                    sendHandshake();
                }
                _lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
                _lastReceiveID = ID;
                _wasConnected = true;
                START_TIMER(_connectionMonitorTimerID, (int)(HEARTBEAT_INTERVAL / 3.1415926));
                LOG_D(LOG_TAG, "Received handshake request from client " + _peerAddress.toString());

                setChannelState(ConnectedState, false);
            }
            else {
                LOG_W(LOG_TAG, "Received client handshake with invalid channel name");
            }
        }
        return; //don't calculate statistics on handshake messages
    case MSGTYPE_HEARTBEAT:
        LOG_D(LOG_TAG, "Received heartbeat packet " + QString::number(ID));
        //no reason to update or check _lastReceiveID
        _lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
        break;
    default:
        LOG_E(LOG_TAG, "Peer sent a message with an invalid header (type=" + QString::number(type) + ")");
        resetConnection();
        return;
    case MSGTYPE_ACK:
        LOG_D(LOG_TAG, "Received ack packet " + QString::number(ID));
        _lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
        int time = _lastReceiveTime - _lastAckReceiveTime;
        _lastAckReceiveTime = _lastReceiveTime;
        if (time != 0) {
            _dataRateUp = (_bytesUp * (100000 / time)) / 100;
            _dataRateDown = (_bytesDown * (100000 / time)) / 100;
        }
        _bytesUp = 0;
        _bytesDown = 0;
        MessageID ackID = deserialize<MessageID>(message);
        if (ackID >= _nextSendID) break;
        int logIndex = _sentTimeLogIndex - (_nextSendID - ackID);
        if (logIndex < 0) {
            if (logIndex < -SENT_LOG_CAP) {
                LOG_W(LOG_TAG, "Received ack for message that had already been discarded from the log, consider increasing SentLogCap in configuration");
                break;
            }
            logIndex += SENT_LOG_CAP;
        }
        _lastRtt = _lastReceiveTime - _sentTimeLog[logIndex];
        break;
    }
    _messagesDown++;
    //If we have reached _statisticsInterval without acking a received packet,
    //send one so the other side can calculate RTT
    if (_sendAcks && (QDateTime::currentMSecsSinceEpoch() - _lastAckSendTime >= STATISTICS_INTERVAL)) {
        _lastAckSendTime = _lastReceiveTime;
        char* ack = new char[sizeof(MessageID)];
        serialize<MessageID>(ack, ID);
        sendMessage(ack, sizeof(MessageID), MSGTYPE_ACK);
    }
}

inline bool Channel::compareHandshake(const char *message, MessageSize size)  const { //PRIVATE
    if ((int)size != _nameUtf8Size) return false; //size + 1 to account for \0
    return strncmp(_nameUtf8, message, _nameUtf8Size) == 0;
}

/*  Sending methods
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

inline void Channel::sendHandshake() {   //PRIVATE SLOT
    LOG_D(LOG_TAG, "Sending handshake to " + _peerAddress.toString());
    sendMessage(_nameUtf8, (MessageSize)_nameUtf8Size, (_isServer ? MSGTYPE_SERVER_HANDSHAKE : MSGTYPE_CLIENT_HANDSHAKE));
}

inline void Channel::sendHeartbeat() {   //PRIVATE SLOT
    sendMessage("\0", 0, MSGTYPE_HEARTBEAT);
}

bool Channel::sendMessage(const char *message, MessageSize size) {
    if (_state == ConnectedState) {
        if (size > MAX_MESSAGE_LENGTH) {
            LOG_W(LOG_TAG, "Attempted to send a message that is too long, it will be truncated");
            return sendMessage(message, MAX_MESSAGE_LENGTH, MSGTYPE_NORMAL);
        }
        return sendMessage(message, size, MSGTYPE_NORMAL);
    }
    else {
        LOG_W(LOG_TAG, "Channel not connected, a message was not sent");
        return false;
    }
}

bool Channel::sendMessage(const char *message, MessageSize size, MessageType type) {   //PRIVATE
    qint64 status;
    char *buffer;
    //LOG_D(LOG_TAG, "Sending packet type=" + QString::number(type) + ",id=" + QString::number(_nextSendID));
    if (_protocol == UdpProtocol) {
        if (_simulatedDelay == 0) {
            _sendBuffer[0] = reinterpret_cast<char&>(type);
            serialize<MessageID>(_sendBuffer + 1, _nextSendID);
            memcpy(_sendBuffer + sizeof(MessageID) + 1, message, (size_t)size);
            status = _udpSocket->writeDatagram(_sendBuffer, size + UDP_HEADER_SIZE, _peerAddress.host, _peerAddress.port);
        }
        else {
            PacketWrapper *wrapper = new PacketWrapper;
            wrapper->data = new char[size + UDP_HEADER_SIZE];
            wrapper->data[0] = reinterpret_cast<char&>(type);
            serialize<MessageID>(wrapper->data + 1, _nextSendID);
            memcpy(wrapper->data + sizeof(MessageID) + 1, message, (size_t)size);
            _delayPackets.enqueue(wrapper);
            startTimer(_simulatedDelay);
            status = size + UDP_HEADER_SIZE;
        }
    }
    else if (_tcpSocket != NULL) {
        MessageSize newSize = size + TCP_HEADER_SIZE;
        if (_simulatedDelay == 0) {
            serialize<MessageSize>(_sendBuffer, newSize);
            _sendBuffer[sizeof(MessageSize)] = reinterpret_cast<char&>(type);
            serialize<MessageID>(_sendBuffer + sizeof(MessageSize) + 1, _nextSendID);
            memcpy(_sendBuffer + sizeof(MessageID) + sizeof(MessageSize) + 1, message, (size_t)size);
            status = _tcpSocket->write(_sendBuffer, newSize);
        }
        else {
            PacketWrapper *wrapper = new PacketWrapper;
            wrapper->data = new char[newSize];
            serialize<MessageSize>(wrapper->data, newSize);
            wrapper->data[sizeof(MessageSize)] = reinterpret_cast<char&>(type);
            serialize<MessageID>(wrapper->data + sizeof(MessageSize) + 1, _nextSendID);
            memcpy(wrapper->data + sizeof(MessageID) + sizeof(MessageSize) + 1, message, (size_t)size);
            _delayPackets.enqueue(wrapper);
            startTimer(_simulatedDelay);
            status = newSize;
        }
    }
    else {
        LOG_E(LOG_TAG, "Attempted to send a message through a null TCP socket");
        return false;
    }
    if (status <= 0) {
        LOG_W(LOG_TAG, "Could not send message (status=" + QString::number(status) + ")");
        return false;
    }
    //log statistics and increment _nextSendID
    _messagesUp++;
    _bytesUp += status;
    _lastSendTime = QDateTime::currentMSecsSinceEpoch();
    _sentTimeLog[_sentTimeLogIndex] = _lastSendTime;
    _nextSendID++;
    _sentTimeLogIndex++;
    if (_sentTimeLogIndex >= SENT_LOG_CAP) {
        _sentTimeLogIndex = 0;
    }
    return true;
}

/*  Getters
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

QString Channel::getName() const {
    return _name;
}

Channel::Protocol Channel::getProtocol() const {
    return _protocol;
}

bool Channel::isServer() const {
    return _isServer;
}

Soro::SocketAddress Channel::getPeerAddress() const {
    return _peerAddress;
}

Channel::State Channel::getState() const {
    return _state;
}

int Channel::getUdpDroppedPacketsPercent() {
    int percent = 0;
    if ((_protocol == UdpProtocol) && (_droppedPackets + _receivedPackets) > 0) {
        percent = _droppedPackets / (_droppedPackets + _receivedPackets);
        _receivedPackets = 0;
        _droppedPackets = 0;
    }
    return percent;
}

int Channel::getConnectionUptime() const {
    if (_state == ConnectedState) {
        return (QDateTime::currentMSecsSinceEpoch() - _connectionEstablishedTime) / 1000;
    }
    return -1;
}

int Channel::getLastRtt() const {
    return _lastRtt;
}

quint64 Channel::getConnectionMessagesUp() const {
    return _messagesUp;
}

quint64 Channel::getConnectionMessagesDown() const {
    return _messagesDown;
}

int Channel::getBitsPerSecondUp() const {
    return _dataRateUp;
}

int Channel::getBitsPerSecondDown() const {
    return _dataRateDown;
}

void Channel::setSendAcks(bool sendAcks) {
    _sendAcks = sendAcks;
}

void Channel::setSimulatedDelay(int ms) {
    _simulatedDelay = ms;
}

SocketAddress Channel::getHostAddress() const {
    if (_udpSocket != NULL) {
        return SocketAddress(_udpSocket->localAddress(), _udpSocket->localPort());
    }
    if (_tcpServer != NULL) {
        return SocketAddress(_tcpServer->serverAddress(), _tcpServer->serverPort());
    }
    if (_tcpSocket != NULL) {
        return SocketAddress(_tcpSocket->localAddress(), _tcpSocket->localPort());
    }
    return SocketAddress(QHostAddress::Null, 0);
}

SocketAddress Channel::getProvidedServerAddress() const {
    return _serverAddress;
}

void Channel::setUdpDropOldPackets(bool dropOldPackets) {
    _dropOldPackets = dropOldPackets;
}

bool Channel::wasConnected() {
    return _wasConnected;
}

void Channel::setLowDelaySocketOption(bool lowDelay) {
    _lowDelaySocketOption = lowDelay ? 1 : 0;
    if (_udpSocket != NULL) {
        _udpSocket->setSocketOption(QAbstractSocket::LowDelayOption, _lowDelaySocketOption);
    }
    if (_tcpSocket != NULL) {
        _tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, _lowDelaySocketOption);
    }
}

}
