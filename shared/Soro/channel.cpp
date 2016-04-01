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

//Macros for the size of UDP and TCP headers
#define UDP_HEADER_BYTES 5
#define TCP_HEADER_BYTES 7

/*  Constructors and destructor
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

using namespace Soro;

Channel::Channel (QObject *parent, const QString &configFile, Logger *log) : QObject(parent) {
    _log = log;
    LOG_I("Opening configuration file " + configFile);
    QFile file(configFile, this);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_E("Cannot open configuration file " + configFile + " for read access");
        setState(ErrorState, false);
        return;
    }

    QTextStream stream(&file);
    parseConfigStream(stream);
    file.close();

    if (_state != ErrorState) init();
}

Channel::Channel (QObject *parent, const QUrl &configUrl, Logger *log) : QObject(parent) {
    _log = log;
    //load the config file form the network url
    _netConfigFileUrl = configUrl;
    _netConfigFileReply = NULL;
    fetchNetworkConfigFile();
}

Channel::Channel(QObject *parent, SocketAddress serverAddress, QString name, Protocol protocol,
                 EndPoint endPoint, QHostAddress hostAddress, Logger *log) : QObject(parent) {
    _log = log;
    _serverAddress = serverAddress;
    _name = name;
    _protocol = protocol;
    _isServer = endPoint == ServerEndPoint;
    _hostAddress.address = hostAddress;

    init();
}

Channel::~Channel() {
    if (_buffer != NULL) {
        delete [] _buffer;
    }
    if (_sentTimeLog != NULL) {
        delete [] _sentTimeLog;
    }
    if (_netConfigFileReply != NULL) {
        _netConfigFileReply->abort();
        delete _netConfigFileReply;
    }
    KILL_TIMER(_connectionMonitorTimerID);
    KILL_TIMER(_resetTcpTimerID);
    KILL_TIMER(_resetTimerID);
    KILL_TIMER(_fetchNetConfigFileTimerID);
    KILL_TIMER(_handshakeTimerID);
    //Qt will take care of cleaning up any object that has 'this' passed to it in its constructor
}

/*  Network configuration stuff, for loading the config file from a
 *  network resource
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

void Channel::fetchNetworkConfigFile() {    //PRIVATE
    //We are loading from a network configuration file and are waiting
    //for a response
    LOG_I("Loading configuration from  " + _netConfigFileUrl.toString());
    QNetworkAccessManager manager(this);
    _netConfigFileReply = manager.get(QNetworkRequest(_netConfigFileUrl));
    connect(_netConfigFileReply, SIGNAL(finished()),
            this, SLOT(netConfigFileAvailable()));
    connect(_netConfigFileReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(netConfigRequestError()));
}

void Channel::netConfigFileAvailable() {    //PRIVATE SLOT
    LOG_I("Downloaded configuration");
    QTextStream stream(_netConfigFileReply);
    parseConfigStream(stream);
    delete _netConfigFileReply;
    _netConfigFileReply = NULL;
        if (_state != ErrorState) {
        init();
        if (_openOnConfigured) {
            //the user has called open() before we had gotten a response from the config file server, so the request
            //had to be delayed until now
            open();
        }
    }
}

void Channel::netConfigRequestError() {  //PRIVATE SLOT
    LOG_E("Cannot fetch network configuration file: " + _netConfigFileReply->errorString() + ", retrying...");
    delete _netConfigFileReply;
    //retry again after RECOVERY_DELAY
    START_TIMER(_fetchNetConfigFileTimerID, RECOVERY_DELAY);
}

/*  Initialization, creates timers, sockets, apply configuration
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

void Channel::parseConfigStream(QTextStream &stream) { //PRIVATE
    const QString usingDefaultWarningString = "%1 was either not found or invalid while loading configuration, using default value %2";
    const QString parseErrorString = "%1 was either not found or invalid while loading configuration";

    IniParser parser;
    if (!parser.load(stream)) {
        LOG_E("The configuration file is not properly formatted");
        setState(ErrorState, false);
        return;
    }

    //These values must be in the file

    bool success;
    _name = parser.value(CONFIG_TAG_CHANNEL_NAME);
    parser.remove(CONFIG_TAG_CHANNEL_NAME);
    if (_name.isEmpty()) {
        LOG_E(parseErrorString.arg(CONFIG_TAG_CHANNEL_NAME));
        setState(ErrorState, false);
        return;
    }
    QString endpoint = parser.value(CONFIG_TAG_ENDPOINT).toLower();
    parser.remove(CONFIG_TAG_ENDPOINT);
    if (endpoint == "server") {
        _isServer = true;
    }
    else if (endpoint == "client") {
        _isServer = false;
    }
    else {
        LOG_E(parseErrorString.arg(CONFIG_TAG_ENDPOINT));
        setState(ErrorState, false);
        return;
    }
    success = parser.valueAsIP(CONFIG_TAG_SERVER_ADDRESS, &_serverAddress.address, true);
    parser.remove(CONFIG_TAG_SERVER_ADDRESS);
    if (!success) {
        if (_isServer) {      //Server address is not mandatory if we are the server
            _serverAddress.address = QHostAddress::Any;
        }
        else {
            LOG_E(parseErrorString.arg(CONFIG_TAG_SERVER_ADDRESS));
            setState(ErrorState, false);
            return;
        }
    }
    int port;
    success = parser.valueAsInt(CONFIG_TAG_SERVER_PORT, &port);
    parser.remove(CONFIG_TAG_SERVER_PORT);
    if (!success) {
        LOG_E(parseErrorString.arg(CONFIG_TAG_SERVER_PORT));
        setState(ErrorState, false);
        return;
    }
    _serverAddress.port = port;
    QString protocol = parser.value(CONFIG_TAG_PROTOCOL).toLower();
    parser.remove(CONFIG_TAG_PROTOCOL);
    if (protocol == "udp") {
        _protocol = UdpProtocol;
    }
    else if (protocol == "tcp") {
        _protocol = TcpProtocol;
    }
    else {
        LOG_E(parseErrorString.arg(CONFIG_TAG_PROTOCOL));
        setState(ErrorState, false);
        return;
    }

    //These values are optional

    success = parser.valueAsIP(CONFIG_TAG_HOST_ADDRESS, &_hostAddress.address, true);
    parser.remove(CONFIG_TAG_HOST_ADDRESS);
    if (!success) {
        LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_HOST_ADDRESS, "-any-"));
        _hostAddress.address = QHostAddress::Any;
    }
    success = parser.valueAsBool(CONFIG_TAG_DROP_OLD_PACKETS, &_dropOldPackets);
    parser.remove(CONFIG_TAG_DROP_OLD_PACKETS);
    if (!success) {
        LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_DROP_OLD_PACKETS, "true"));
        _dropOldPackets = true;
    }
    success = parser.valueAsBool(CONFIG_TAG_SEND_ACKS, &_sendAcks);
    parser.remove(CONFIG_TAG_SEND_ACKS);
    if (!success) {
        LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_SEND_ACKS, "true"));
        _sendAcks = true;
    }
    bool lowDelay;
    success = parser.valueAsBool(CONFIG_TAG_LOW_DELAY, &lowDelay);
    parser.remove(CONFIG_TAG_LOW_DELAY);
    if (!success) {
        LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_LOW_DELAY, "false"));
        lowDelay = false;
    }
    _lowDelaySocketOption = lowDelay ? 1 : 0;

    //check for unknown values (known ones were already removed
    foreach (QString unknown, parser.tags()) {
        LOG_W("Uknown configuration option " + unknown);
    }
}

void Channel::init() {  //PRIVATE
    //log tag for debugging
    LOG_TAG = _name + (_isServer ? "(S)" : "(C)");

    //create a buffer for storing received messages
    _buffer = new char[1024];
    _sentTimeLog = new qint64[SENT_LOG_CAP];

    LOG_I("Initializing with serverAddress=" + _serverAddress.toString()
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
    setState(ReadyState, false); //now safe to call open()
}

/*  Connection lifecycle management
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

void Channel::open() {
    LOG_D("open() called");
    setState(_state, true); //force emit the stateChanged signal
    switch (_state) {
    case ReadyState:
        //If this is the server, we will bind to the server port
        if (_isServer) {
            _hostAddress.port = _serverAddress.port;
            LOG_I("Attempting to bind to " + _hostAddress.toString());
        }
        else {
            _hostAddress.port = 0;
            LOG_I("Attempting to bind to an available port on " + _hostAddress.address.toString());
        }
        if (_tcpServer != NULL) {
            _tcpServer->setMaxPendingConnections(1);
        }
        //start the connection procedure
        resetConnection();
        break;
    case UnconfiguredState:
        //cannot open yet because we are waiting on a reply from the network server
        //where the config file is stored. Open as soon as it's ready.
        LOG_D("openOnConfiguration set to true");
        _openOnConfigured = true;
        break;
    default:
        LOG_E("Cannot call open() in the current channel state");
        break;
    }
}

void Channel::resetConnectionVars() {   //PRIVATE
    LOG_D("resetConnectionVars() called");
    _bufferLength = 0;
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
    emit statisticsUpdate(_lastRtt, _messagesUp, _messagesDown, _dataRateUp, _dataRateDown);
}

void Channel::resetConnection() {   //PRIVATE
    LOG_I("Attempting to connect to other side of channel...");
    setState(ConnectingState, false);
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
        LOG_D("Cancelling any previous TCP operations...");
        _tcpSocket->abort();
        if (_isServer) {
            LOG_D("Cleaning up TCP connection with old client...");
            _tcpSocket->close();
            delete _tcpSocket;
            _tcpSocket = NULL;
        }
        else {
            LOG_D("Trying to establish TCP connection with server " + _serverAddress.toString());
            _tcpSocket->bind(_hostAddress.address, _hostAddress.port);
            _tcpSocket->connectToHost(_serverAddress.address, _serverAddress.port);
        }
    }
    else if (_udpSocket != NULL) {
        LOG_D("Cancelling any previous UDP operations...");
        _udpSocket->abort();
        _udpSocket->bind(_hostAddress.address, _hostAddress.port);
        if (!_isServer) START_TIMER(_handshakeTimerID, HANDSHAKE_FREQUENCY);
    }
    if (_tcpServer != NULL) {
        if (!_tcpServer->isListening()) {
            LOG_D("Waiting for TCP client to connect on port " + QString::number(_hostAddress.port));
            _tcpServer->listen(_hostAddress.address, _hostAddress.port);
        }
        if (_tcpServer->hasPendingConnections()) {
            //allow the next pending connection since we just dropped the old one
            newTcpClient();
        }
    }
}

void Channel::timerEvent(QTimerEvent *e) {  //PROTECTED
    int id = e->timerId();
    if (id == _connectionMonitorTimerID) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        //check for a stale connection (several seconds without a message)
        if (now - _lastReceiveTime >= IDLE_CONNECTION_TIMEOUT) {
            LOG_E("Peer has stopped responding, dropping connection");
            resetConnection();
        }
        else if (now - _lastSendTime >= HEARTBEAT_INTERVAL) {
            //Send a heartbeat message, even on TCP (They are needed for RTT updates
            //and are a good idea anyway)
            sendHeartbeat();
        }
    }
    else if (id == _resetTcpTimerID) {
        LOG_E("Connected TCP peer did not verify identity in time");
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
    else if (id == _fetchNetConfigFileTimerID) {
        fetchNetworkConfigFile();
        KILL_TIMER(_fetchNetConfigFileTimerID); //single shot
    }
}

void Channel::configureNewTcpSocket() { //PRIVATE
    //set the signals for a new TCP socket
    if (_tcpSocket != NULL) {
        _socket = _tcpSocket;
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
    setState(ConnectingState, false);
    setPeerAddress(SocketAddress(_tcpSocket->peerAddress(), _tcpSocket->peerPort()));
    sendHandshake();
    //Close the connection if it is not verified in time
    START_TIMER(_resetTcpTimerID, IDLE_CONNECTION_TIMEOUT);
    LOG_I("TCP peer " + _peerAddress.toString() + " has connected");
}

void Channel::newTcpClient() {  //PRIVATE SLOT
    if (_tcpSocket == NULL) { //don't accept the connection if we already have one
        _socket = _tcpSocket = _tcpServer->nextPendingConnection();
        configureNewTcpSocket();
        tcpConnected();
    }
}

inline void Channel::setState(Channel::State state, bool forceUpdate) {   //PRIVATE
    //signals the stateChanged event
     if ((_state != state) | forceUpdate) {
         LOG_D("Setting state to " + QString::number(state));
         _state = state;
         emit stateChanged(_state);
     }
}

inline void Channel::setPeerAddress(Soro::SocketAddress address) {    //PRIVATE
    //signals the peerAddressChanged event
    if (_peerAddress != address) {
        LOG_D("Setting peer address to " + address.toString());
        _peerAddress = address;
        emit peerAddressChanged(_peerAddress);
    }
}

void Channel::close() {
    close(ReadyState);
}

void Channel::close(Channel::State closeState) {   //PRIVATE
    if (_state == UnconfiguredState) {
        LOG_W("Channel is loading it's configuration and cannot be closed until this process is finished");
        return;
    }
    LOG_W("Closing channel in state " + QString::number(closeState));
    if (_socket != NULL) {
        _socket->close();
    }
    if (_tcpServer != NULL) {
        _tcpServer->close();
    }
    KILL_TIMER(_connectionMonitorTimerID);
    KILL_TIMER(_resetTcpTimerID);
    KILL_TIMER(_resetTimerID);
    KILL_TIMER(_handshakeTimerID);
    resetConnectionVars();
    setState(closeState, false);
}

/*  Socket error handling
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

void Channel::connectionErrorInternal(QAbstractSocket::SocketError err) { //PRIVATE SLOT
    emit connectionError(err);
    LOG_E("Connection Error: " + _socket->errorString());
    //Attempt to reconnect after RECOVERY_DELAY
    //we should NOT directly call resetConnectio() here as that could
    //potentially force an error loop
    START_TIMER(_resetTimerID, RECOVERY_DELAY);
}

void Channel::serverErrorInternal(QAbstractSocket::SocketError err) { //PRIVATE SLOT
    emit connectionError(err);
    LOG_E("Server Error: " + _tcpServer->errorString());
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
    LOG_D("udpReadyRead() called");
    SocketAddress address;
    MESSAGE_ID ID;
    MESSAGE_TYPE type;
    qint64 status;
    while (_udpSocket->hasPendingDatagrams()) {
        //read in a datagram
        status = _udpSocket->readDatagram(_buffer, MAX_MESSAGE_LENGTH, &address.address, &address.port);
        if (status < 0) {
            //an error occurred reading from the socket, the onSocketError slot will handle it
            return;
        }
        _bufferLength = status;
        QDataStream stream(QByteArray::fromRawData(_buffer, _bufferLength));
        stream >> type;
        //ensure the datagram either came from the correct address, or is marked as a handshake
        if (_isServer) {
            if ((address != _peerAddress) & (type != MSGTYPE_CLIENT_HANDSHAKE)) {
                LOG_D("Received non-handshake UDP packet from unknown peer");
                continue;
            }
        }
        else if ((address != _peerAddress) & (address != _serverAddress)) {
            LOG_D("Received UDP packet that was not from server");
            continue;
        }
        _bytesDown += status;
        stream >> ID;
        ID = qFromBigEndian(ID);
        QByteArray message = QByteArray::fromRawData(_buffer + UDP_HEADER_BYTES, _bufferLength - UDP_HEADER_BYTES);
        processBufferedMessage(type, ID, message, address);
    }
}

void Channel::tcpReadyRead() {  //PRIVATE SLOT
    LOG_D("tcpReadyRead() called");
    qint64 status;
    while (_tcpSocket->bytesAvailable() > 0) {
        if (_bufferLength < TCP_HEADER_BYTES) {
            //read the header in first so we know how long the message should be
            status = _tcpSocket->read(_buffer + _bufferLength, TCP_HEADER_BYTES - _bufferLength);
            if (status < 0) {
                //an error occurred reading from the socket, the onSocketError slot will handle it
                return;
            }
            _bufferLength += status;
        }
        if (_bufferLength >= TCP_HEADER_BYTES) {
            //The header is in the buffer, so we know how long the packet is
            MESSAGE_LENGTH length;
            QDataStream stream(QByteArray::fromRawData(_buffer, TCP_HEADER_BYTES));
            stream >> length;
            length = qFromBigEndian(length);
            if (length > MAX_MESSAGE_LENGTH + TCP_HEADER_BYTES) {
                LOG_W("TCP peer sent a message with an invalid header (length=" + QString::number(length) + ")");
                resetConnection();
                return;
            }
            //read the rest of the message (if it's all there)
            status = _tcpSocket->read(_buffer + _bufferLength, length - _bufferLength);
            if (status < 0) {
                //an error occurred reading from the socket, the onSocketError slot will handle it
                _bufferLength = 0;
                return;
            }
            _bufferLength += status;
            if (_bufferLength == length) {
                //we have the whole message
                _bytesDown += length;
                MESSAGE_TYPE type;
                MESSAGE_ID ID;
                stream >> type;
                stream >> ID;
                ID = qFromBigEndian(ID);
                QByteArray message = QByteArray::fromRawData(_buffer + TCP_HEADER_BYTES, _bufferLength - TCP_HEADER_BYTES);
                _bufferLength = 0;
                processBufferedMessage(type, ID, message, _peerAddress);
            }
        }
    }
}

void Channel::processBufferedMessage(MESSAGE_TYPE type, MESSAGE_ID ID, const QByteArray &message, const SocketAddress &address) {
    switch (type) {
    case MSGTYPE_NORMAL:
        //normal data packet
        //check the packet sequence ID
        if ((ID > _lastReceiveID) | !_dropOldPackets){
            LOG_D("Received normal packet " + QString::number(ID));
            _lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
            _lastReceiveID = ID;
            emit messageReceived(message);
        }
        break;
    case MSGTYPE_SERVER_HANDSHAKE:
        //this packet is a handshake request
        LOG_D("Received server handshake packet " + QString::number(ID));
        if (!_isServer) {
            if (compareHandshake(message)) {
                //we are the client, and we got a respoonse from the server (yay)
                resetConnectionVars();
                setPeerAddress(address);
                KILL_TIMER(_handshakeTimerID);
                KILL_TIMER(_resetTcpTimerID);
                _lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
                _lastReceiveID = ID;
                setState(ConnectedState, false);
                START_TIMER(_connectionMonitorTimerID, (int)(HEARTBEAT_INTERVAL / 3.1415926));
                LOG_D("Received handshake response from server " + _serverAddress.toString());
            }
            else {
                LOG_W("Received server handshake with invalid channel name");
            }
        }
        return; //don't calculate statistics on handshake messages
    case MSGTYPE_CLIENT_HANDSHAKE:
        if (_isServer) {
            LOG_D("Received client handshake packet " + QString::number(ID));
            if (compareHandshake(message)) {
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
                setState(ConnectedState, false);
                START_TIMER(_connectionMonitorTimerID, (int)(HEARTBEAT_INTERVAL / 3.1415926));
                LOG_D("Received handshake request from client " + _peerAddress.toString());
            }
            else {
                LOG_W("Received client handshake with invalid channel name");
            }
        }
        return; //don't calculate statistics on handshake messages
    case MSGTYPE_HEARTBEAT:
        LOG_D("Received heartbeat packet " + QString::number(ID));
        //no reason to update or check _lastReceiveID
        _lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
        break;
    default:
        LOG_E("Peer sent a message with an invalid header (type=" + QString::number(type) + ")");
        resetConnection();
        return;
    case MSGTYPE_ACK:
        LOG_D("Received ack packet " + QString::number(ID));
        _lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
        int time = _lastReceiveTime - _lastAckReceiveTime;
        _lastAckReceiveTime = _lastReceiveTime;
        if (time != 0) {
            _dataRateUp = (_bytesUp * (100000 / time)) / 100;
            _dataRateDown = (_bytesDown * (100000 / time)) / 100;
        }
        _bytesUp = 0;
        _bytesDown = 0;
        QDataStream stream(message);
        MESSAGE_ID ackID;
        stream >> ackID;
        ackID = qFromBigEndian(ackID);
        if (ackID >= _nextSendID) break;
        int logIndex = _sentTimeLogIndex - (_nextSendID - ackID);
        if (logIndex < 0) {
            if (logIndex < -SENT_LOG_CAP) {
                LOG_W("Received ack for message that had already been discarded from the log, consider increasing SentLogCap in configuration");
                break;
            }
            logIndex += SENT_LOG_CAP;
        }
        _lastRtt = _lastReceiveTime - _sentTimeLog[logIndex];
        emit statisticsUpdate(_lastRtt, _messagesUp, _messagesDown, _dataRateUp, _dataRateDown);
        break;
    }
    _messagesDown++;
    //If we have reached _statisticsInterval without acking a received packet,
    //send one so the other side can calculate RTT
    if (_sendAcks && (QDateTime::currentMSecsSinceEpoch() - _lastAckSendTime >= STATISTICS_INTERVAL)) {
        _lastAckSendTime = _lastReceiveTime;
        QByteArray ack("", sizeof(MESSAGE_ID));
        QDataStream stream(&ack, QIODevice::WriteOnly);
        stream << qToBigEndian(ID);
        sendMessage(ack, MSGTYPE_ACK);
    }
}

inline bool Channel::compareHandshake(const QByteArray &message)  const { //PRIVATE
    if (message.size() != _nameUtf8.size()) return false;
    return _nameUtf8.startsWith(message);
}

/*  Sending methods
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

inline void Channel::sendHandshake() {   //PRIVATE SLOT
    LOG_D("Sending handshake to " + _peerAddress.toString());
    sendMessage(_nameUtf8, (_isServer ? MSGTYPE_SERVER_HANDSHAKE : MSGTYPE_CLIENT_HANDSHAKE));
}

inline void Channel::sendHeartbeat() {   //PRIVATE SLOT
    sendMessage(QByteArray(), MSGTYPE_HEARTBEAT);
}

bool Channel::sendMessage(const QByteArray &message) {
    if (_state == ConnectedState) {
        if (message.size() > MAX_MESSAGE_LENGTH) {
            LOG_W("Attempting to send a message that is too long, it will be truncated");
            QByteArray truncated(message);
            truncated.truncate(MAX_MESSAGE_LENGTH);
            return sendMessage(truncated, MSGTYPE_NORMAL);
        }
        return sendMessage(message, MSGTYPE_NORMAL);
    }
    else {
        LOG_W("Channel not connected, a message was not sent");
        return false;
    }
}

bool Channel::sendMessage(const QByteArray &message, MESSAGE_TYPE type) {   //PRIVATE
    qint64 status;
    LOG_D("Sending packet type=" + QString::number(type) + ",id=" + QString::number(_nextSendID));
    if (_protocol == UdpProtocol) {
        QByteArray arr("", UDP_HEADER_BYTES);
        QDataStream stream(&arr, QIODevice::WriteOnly);
        stream << (MESSAGE_TYPE)type;
        stream << (MESSAGE_ID)qToBigEndian(_nextSendID);
        arr.append(message);
        status = _udpSocket->writeDatagram(arr, _peerAddress.address, _peerAddress.port);
    }
    else if (_tcpSocket != NULL) {
        MESSAGE_LENGTH size = message.size() + TCP_HEADER_BYTES;
        QByteArray arr("", TCP_HEADER_BYTES);
        QDataStream stream(&arr, QIODevice::WriteOnly);
        stream << (MESSAGE_LENGTH)qToBigEndian(size);
        stream << (MESSAGE_TYPE)type;
        stream << (MESSAGE_ID)qToBigEndian(_nextSendID);
        arr.append(message);
        status = _tcpSocket->write(arr);
    }
    else {
        LOG_E("Attempted to send a message through a null TCP socket");
        return false;
    }
    if (status <= 0) {
        LOG_W("Could not send message (status=" + QString::number(status) + ")");
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

int Channel::getDataRateUp() const {
    return _dataRateUp;
}

int Channel::getDataRateDown() const {
    return _dataRateDown;
}

void Channel::setSendAcks(bool sendAcks) {
    _sendAcks = sendAcks;
}

void Channel::setUdpDropOldPackets(bool dropOldPackets) {
    _dropOldPackets = dropOldPackets;
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
