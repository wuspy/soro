#include "channel.h"

Channel::Channel (QObject *parent, const QString &configFile, Logger *log) : QObject(parent) {
    _log = log;
    initVars();
    LOG_I("Opening configuration file " + configFile);
    QFile file(configFile, this);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_E("Cannot open configuration file " + configFile + " for read access");
        setState(ErrorState);
        return;
    }

    QTextStream stream(&file);
    initWithConfiguration(stream);

    file.close();
}

Channel::Channel (QObject *parent, const QUrl &configUrl, Logger *log) : QObject(parent) {
    _log = log;
    initVars();
    _netConfigUrl = configUrl;
    setState(AwaitingConfigurationState);
    _watchdogTimer = new QTimer(this);
    _watchdogTimer->setInterval(DEFAULT_WATCHDOG_INTERVAL);
    _watchdogTimer->start();
}

Channel::~Channel() {
    if (_buffer != NULL) {
        delete _buffer;
    }
    if (_sentTimeLog != NULL) {
        delete [] _sentTimeLog;
    }
    //Qt will take care of cleaning up any object that has 'this' passed to it in its constructor
}

inline void Channel::initVars() {
    LOG_TAG = "CHANNEL";
    _tcpServer = NULL;
    _tcpSocket = NULL;
    _udpSocket = NULL;
    _socket = NULL;
    _buffer = NULL;
    _sentTimeLog = NULL;
    _watchdogTimer = NULL;
    _openOnConfigured = false;
    _ackNextMessage = false;
}

void Channel::initWithConfiguration(QTextStream &stream) { //PRIVATE
    //default if not specified
    _dropOldPackets = true;
    _watchdogInterval = DEFAULT_WATCHDOG_INTERVAL;
    _statisticsInterval = DEFAULT_STATISTICS_INTERVAL;
    _idleConnectionTimeout = DEFAULT_IDLE_CONNECTION_TIMEOUT;
    _tcpVerifyTimeout = DEFAULT_TCP_VERIFY_TIMEOUT;
    _sentLogCap = DEFAULT_SENT_LOG_CAP;
    _hostAddress.address = QHostAddress::Any;
    
    const QString usingDefaultWarningString = "%1 was either not found or invalid while loading configuration, using default value %2";
    const QString parseErrorString = "%1 was either not found or invalid while loading configuration";

    TagValueParser parser;
    if (!parser.load(stream)) {
        LOG_E("The configuration file is not properly formatted");
        setState(ErrorState);
        return;
    }

    //These values must be in the file

    bool success;
    _name = parser.value(CONFIG_TAG_CHANNEL_NAME);
    parser.remove(CONFIG_TAG_CHANNEL_NAME);
    if (_name.isEmpty()) {
        LOG_E(parseErrorString.arg(CONFIG_TAG_CHANNEL_NAME));
        setState(ErrorState);
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
        setState(ErrorState);
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
            setState(ErrorState);
            return;
        }
    }
    int port;
    success = parser.valueAsInt(CONFIG_TAG_SERVER_PORT, &port);
    parser.remove(CONFIG_TAG_SERVER_PORT);
    if (!success) {
        LOG_E(parseErrorString.arg(CONFIG_TAG_SERVER_PORT));
        setState(ErrorState);
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
        setState(ErrorState);
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
    success = parser.valueAsInt(CONFIG_TAG_WATCHDOG_INTERVAL, &_watchdogInterval);
    parser.remove(CONFIG_TAG_WATCHDOG_INTERVAL);
    if (!success) {
        LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_WATCHDOG_INTERVAL, QString::number(DEFAULT_WATCHDOG_INTERVAL)));
        _watchdogInterval = DEFAULT_WATCHDOG_INTERVAL;
    }
    success = parser.valueAsInt(CONFIG_TAG_IDLE_CONNECTION_TIMEOUT, &_idleConnectionTimeout);
    parser.remove(CONFIG_TAG_IDLE_CONNECTION_TIMEOUT);
    if (!success) {
        LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_IDLE_CONNECTION_TIMEOUT, QString::number(DEFAULT_IDLE_CONNECTION_TIMEOUT)));
        _idleConnectionTimeout = DEFAULT_IDLE_CONNECTION_TIMEOUT;
    }
    success = parser.valueAsInt(CONFIG_TAG_SENT_LOG_CAP, &_sentLogCap);
    parser.remove(CONFIG_TAG_SENT_LOG_CAP);
    if (!success) {
        LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_SENT_LOG_CAP, QString::number(DEFAULT_SENT_LOG_CAP)));
        _sentLogCap = DEFAULT_SENT_LOG_CAP;
    }
    success = parser.valueAsInt(CONFIG_TAG_STATISTICS_INTERVAL, &_statisticsInterval);
    parser.remove(CONFIG_TAG_STATISTICS_INTERVAL);
    if (!success) {
        LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_STATISTICS_INTERVAL, QString::number(DEFAULT_STATISTICS_INTERVAL)));
        _statisticsInterval = DEFAULT_STATISTICS_INTERVAL;
    }
    success = parser.valueAsInt(CONFIG_TAG_TCP_VERIFY_TIMEOUT, &_tcpVerifyTimeout);
    parser.remove(CONFIG_TAG_TCP_VERIFY_TIMEOUT);
    if (!success) {
        LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_TCP_VERIFY_TIMEOUT, QString::number(DEFAULT_TCP_VERIFY_TIMEOUT)));
        _tcpVerifyTimeout = DEFAULT_TCP_VERIFY_TIMEOUT;
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

    //log tag for debugging
    LOG_TAG = _name + (_isServer ? "(S)" : "(C)");

    //create a buffer for storing received messages
    _buffer = new char[1024];
    _sentTimeLog = new QTime[_sentLogCap];

    LOG_I("Initializing with serverAddress=" + _serverAddress.toString()
          + ",protocol=" + (_protocol == TcpProtocol ? "TCP" : "UDP"));

    //setup the watchdog timer
    if (_watchdogTimer == NULL) {
        _watchdogTimer = new QTimer(this);
        connect(_watchdogTimer, SIGNAL(timeout()), this, SLOT(watchdogTick()));
    }
    _watchdogTimer->setInterval(_watchdogInterval);

    //create socket and connect signals
    if (_protocol == UdpProtocol) {
        //Clients and servers for UDP communication function similarly (unlike TCP)
        _socket = _udpSocket = new QUdpSocket(this);
        _socket->setSocketOption(QAbstractSocket::LowDelayOption, _lowDelaySocketOption);
        connect(_socket, SIGNAL(readyRead()), this, SLOT(udpReadyRead()));
        connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(socketError(QAbstractSocket::SocketError)));
    }
    else if (_isServer) {
        //Server for a TCP connection; we must use a QTcpServer to manage connecting peers
        _tcpServer = new QTcpServer(this);
        connect(_tcpServer, SIGNAL(newConnection()), this, SLOT(newTcpClient()));
        connect(_tcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)),
                this, SLOT(serverError(QAbstractSocket::SocketError)));
    }
    else {
        //Client for a TCP connection
        _socket = _tcpSocket = new QTcpSocket(this);
        _socket->setSocketOption(QAbstractSocket::LowDelayOption, _lowDelaySocketOption);
        configureNewTcpSocket(); //This is its own function, as it must be called every time a new
                                 //TCP client connects in a server scenario
    }
    setState(ReadyState); //now safe to call open()
}

void Channel::open() {
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
        if (!_watchdogTimer->isActive()) {
            _watchdogTimer->start();
        }
        //start the connection procedure
        resetConnection();
        break;
    case AwaitingConfigurationState:
        //cannot open yet because we are waiting on a reply from the network server
        //where the config file is stored. Open as soon as it's ready.
        _openOnConfigured = true;
        break;
    default:
        LOG_E("Cannot call open() in the current channel state");
        break;
    }
}

void Channel::configureNewTcpSocket() {
    //set the signals for a new TCP socket
    if (_tcpSocket != NULL) {
        _socket = _tcpSocket;
        _socket->setSocketOption(QAbstractSocket::LowDelayOption, _lowDelaySocketOption);
        connect(_socket, SIGNAL(readyRead()), this, SLOT(tcpReadyRead()));
        connect(_socket, SIGNAL(connected()), this, SLOT(tcpConnected()));
        connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(socketError(QAbstractSocket::SocketError)));
    }
}

void Channel::routeMessage(Channel *sender, const QByteArray &message) {  //PRIVATE SLOT
    sendMessage(message);
}

void Channel::socketError(QAbstractSocket::SocketError err) { //PRIVATE SLOT
    switch (err) {
    case QAbstractSocket::SocketTimeoutError:    //non-fatal
        LOG_E("A socket operation has timed out");
        break;
    case QAbstractSocket::TemporaryError:    //non-fatal
        LOG_E("Temporary connection error: " + _socket->errorString());
        break;
    case QAbstractSocket::SocketAddressNotAvailableError:   //fatal
        LOG_E("FATAL: The address is not available to open on this device. Select a different host address");
        close(ErrorState);
        break;
    case QAbstractSocket::AddressInUseError:    //fatal
        LOG_E("FATAL: Address in use. Select a different port");
        close(ErrorState);
        break;
    case QAbstractSocket::DatagramTooLargeError: //non-fatal, should never happen with the checks we have in place
        LOG_E("Attempted to send a datagram that was too large");
        break;
    default:    //reset connection to be safe
        LOG_E("Connection Error: " + _socket->errorString());
        //this will force the watchdog to reset the connection,
        //we should NOT directly call resetConnectio() here as that could
        //potentially force an error loop
        setState(DisconnectedState);
        break;
    }
}

void Channel::serverError(QAbstractSocket::SocketError err) { //PRIVATE SLOT
    switch (err) {
    case QAbstractSocket::SocketTimeoutError:    //non-fatal
        LOG_E("A server operation has timed out");
        break;
    case QAbstractSocket::TemporaryError:    //non-fatal
        LOG_E("Temporary server error: " + _socket->errorString());
        break;
    case QAbstractSocket::SocketAddressNotAvailableError:
        LOG_E("FATAL: The address is not available to open on this device. Select a different host address");
        close(ErrorState);
        break;
    case QAbstractSocket::AddressInUseError:
        LOG_E("FATAL: Address in use. Select a different port");
        close(ErrorState);
        break;
    default:
        LOG_E("Server Error: " + _tcpServer->errorString());
        //don't automatically kill the connection if it's still active, only the listening server
        //experienced the error
        if (_tcpSocket == NULL || _tcpSocket->state() != QAbstractSocket::ConnectedState) {
            setState(DisconnectedState); //this will force the timer to reset the connection
        }
        break;
    }
}

void Channel::close() {
    close(ReadyState);
}

void Channel::close(Channel::State closeState) {   //PRIVATE
    LOG_W("Closing channel in state " + QString::number(closeState));
    if (_watchdogTimer != NULL && _watchdogTimer->isActive()) {
        _watchdogTimer->stop();
    }
    if (_socket != NULL) {
        _socket->close();
    }
    if (_tcpServer != NULL) {
        _tcpServer->close();
    }
    setState(closeState);
}

void Channel::udpReadyRead() {  //PRIVATE SLOT
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
                continue;
            }
        }
        else if ((address != _peerAddress) & (address != _serverAddress)) {
            continue;
        }
        stream >> ID;
        ID = qFromBigEndian(ID);
        QByteArray message = QByteArray::fromRawData(_buffer + UDP_HEADER_BYTES, _bufferLength - UDP_HEADER_BYTES);
        processBufferedMessage(type, ID, message, address);
    }
}

void Channel::tcpReadyRead() {  //PRIVATE SLOT
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
            _lastReceiveTime = QTime::currentTime();
            _lastReceiveID = ID;
            emit messageReceived(this, message);
        }
        break;
    case MSGTYPE_SERVER_HANDSHAKE:
        //this packet is a handshake request
        if (!_isServer) {
            if (compareHandshake(message)) {
                //we are the client, and we got a respoonse from the server (yay)
                resetConnectionVars();
                setPeerAddress(address);
                if (_protocol == TcpProtocol) _tcpVerified = true;
                _lastReceiveTime = QTime::currentTime();
                _lastReceiveID = ID;
                setState(ConnectedState);
                LOG_I("Received handshake response from server " + _serverAddress.toString());
            }
            else {
                LOG_W("Received server handshake with invalid channel name");
            }
        }
        return; //don't calculate statistics on handshake messages
    case MSGTYPE_CLIENT_HANDSHAKE:
        if (_isServer) {
            if (compareHandshake(message)) {
                //We are the server getting a new (valid) handshake request, respond back and record the address
                resetConnectionVars();
                setPeerAddress(address);
                setState(ConnectedState);
                if (_protocol == TcpProtocol) {
                    _tcpVerified = true;
                }
                else {
                    sendHandshake();
                }
                _lastReceiveTime = QTime::currentTime();
                _lastReceiveID = ID;
                LOG_I("Received handshake request from client "
                                        + _peerAddress.toString());
            }
            else {
                LOG_W("Received client handshake with invalid channel name");
            }
        }
        return; //don't calculate statistics on handshake messages
    case MSGTYPE_HEARTBEAT:
        //no reason to update or check _lastReceiveID
        _lastReceiveTime = QTime::currentTime();
        break;
    default:
        LOG_W("Peer sent a message with an invalid header (type=" + QString::number(type) + ")");
        resetConnection();
        return;
    case MSGTYPE_ACK:
        _lastReceiveTime = QTime::currentTime();
        QDataStream stream(message);
        MESSAGE_ID ackID;
        stream >> ackID;
        ackID = qFromBigEndian(ackID);
        if (ackID >= _nextSendID) break;
        int logIndex = _sentTimeLogIndex - _nextSendID - ackID;
        if (logIndex < 0) {
            if (logIndex <= -_sentLogCap) {
                LOG_W("Received ack for message that had already been discarded from the log, consider increasing SentLogCap in configuration");
                break;
            }
            logIndex = _sentLogCap - logIndex;
        }
        int rtt = _sentTimeLog[logIndex].msecsTo(_lastReceiveTime);
        emit statisticsUpdate(this, rtt, _messagesUp, _messagesDown);
        break;
    }
    _messagesDown++;
    //If we have reached _statisticsInterval without acking a received packet,
    //send one so the other side can calculate RTT
    if (_lastAckSendTime.msecsTo(QTime::currentTime()) >= _statisticsInterval) {
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

void Channel::tcpConnected() {  //PRIVATE SLOT
    //Establishing a TCP connection does not mean this channel is connected.
    //Both sides of the channel must send handshakes to each other to verify identity.
    //If this message is not sent timely (within a few seconds), both sides will disconnect
    //and attempt the whole thing over again
    setState(ConnectingState);
    _tcpVerifyTicks = 0;
    _tcpVerified = false;
    setPeerAddress(SocketAddress(_tcpSocket->peerAddress(), _tcpSocket->peerPort()));
    sendHandshake();
    LOG_I("TCP peer " + _peerAddress.toString() + " has connected");
}

void Channel::newTcpClient() {  //PRIVATE SLOT
    if (_tcpSocket == NULL) { //don't accept the connection if we already have one
        _socket = _tcpSocket = _tcpServer->nextPendingConnection();
        configureNewTcpSocket();
        tcpConnected();
    }
}

inline void Channel::setState(Channel::State state) {   //PRIVATE
    //signals the stateChanged event
     if (_state != state) {
         _state = state;
         emit stateChanged(this, _state);
     }
}

inline void Channel::setPeerAddress(Channel::SocketAddress address) {    //PRIVATE
    //signals the peerAddressChanged event
    if (_peerAddress != address) {
        _peerAddress = address;
        emit peerAddressChanged(this, _peerAddress);
    }
}

void Channel::watchdogTick() {   //PRIVATE SLOT
    switch (_state) {
    case ConnectedState:
        //check for sending heartbeat message, even on TCP (They are needed for QoC updates
        //and are a good idea anyway)
        if (_lastSendTime.msecsTo(QTime::currentTime()) > (_idleConnectionTimeout / 5)) {
            //A message hasn't been sent in a while, send a heartbeat
            //to let the other side know the we are still active
            sendHeartbeat();
        }
        //check for a stale connection (several seconds without a message)
        else if (_lastReceiveTime.msecsTo(QTime::currentTime()) >= _idleConnectionTimeout) {
            LOG_E("Peer has stopped responding, dropping connection");
            resetConnection();
        }
        break;
    case ConnectingState:
        //Check for a TCP verification timeout, and attempt to reconnect if the peer
        //doesn't verify in a few seconds
        if ((_protocol == TcpProtocol) &&
             (_tcpSocket != NULL) &&
             (_tcpSocket->state() == QAbstractSocket::ConnectedState) &&
                !_tcpVerified) {
            _tcpVerifyTicks++;
            if (_tcpVerifyTicks > _tcpVerifyTimeout / _watchdogInterval) {
                LOG_E("Peer did not verify TCP connection in time");
                resetConnection();
            }
            else {
                LOG_I("Awaiting verification from connected TCP peer...");
            }
        }
        //check for sending UDP handshake
        else if ((_protocol == UdpProtocol) & !_isServer) {
            //send a handshake request to the server's address
            sendHandshake();
        }
        break;
    case DisconnectedState:
    case ReadyState:
        //The channel is disconnected, and the watchdog is still running so we
        //should reconnect it
        resetConnection();
        break;
    case ErrorState:
        LOG_E("Fatal error caught, stopping channel monitor");
        _watchdogTimer->stop();
        break;
    case AwaitingConfigurationState:
        //We are loading from a network configuration file and are waiting
        //for a response
        if (_netConfigReply == NULL) {
            LOG_I("Loading configuration from  " + _netConfigUrl.toString());
            QNetworkAccessManager manager(this);
            _netConfigReply = manager.get(QNetworkRequest(_netConfigUrl));
        }
        if (_netConfigReply->isFinished()) {
            LOG_I("Downloaded configuration");
            _watchdogTimer->stop(); //will be restarted once open() is called
            QTextStream stream(_netConfigReply);
            initWithConfiguration(stream);
            delete _netConfigReply;
            _netConfigReply = NULL;
            if (_openOnConfigured) {
                //the user has called open() before we had the configuration, so the request has
                //been delayed
                open();
            }
        }
        else if (!_netConfigReply->isRunning()) {
            LOG_E("Could not fetch configuration from " + _netConfigUrl.toString() + ", retrying...");
            delete _netConfigReply;
            QNetworkAccessManager manager(this);
            _netConfigReply = manager.get(QNetworkRequest(_netConfigUrl));
        }
        break;
    }
}

void Channel::resetConnectionVars() {   //PRIVATE
    _bufferLength = 0;
    _lastReceiveID = 0;
    _lastAckSendTime = QTime::currentTime();
    _nextSendID = 1;
    _tcpVerifyTicks = 0;
    _messagesDown = _messagesUp = 0;
    _sentTimeLogIndex = 0;
}

void Channel::resetConnection() {
    LOG_I("Attempting to connect to other side of channel...");
    setState(ConnectingState);
    resetConnectionVars();
    _sentTimeLogIndex = 0;
    if (_isServer) {
        setPeerAddress(SocketAddress(QHostAddress::Null, 0));
    }
    else {
        //Only allow the server address to connect
        setPeerAddress(_serverAddress);
    }
    if (_tcpSocket != NULL) {
        LOG_I("Cancelling any previous TCP operations...");
        _tcpSocket->abort();
        if (_isServer) {
            LOG_I("Cleaning up TCP connection with old client...");
            _tcpSocket->close();
            delete _tcpSocket;
            _tcpSocket = NULL;
        }
        else {
            LOG_I("Trying to establish TCP connection with server " + _serverAddress.toString());
            _tcpSocket->bind(_hostAddress.address, _hostAddress.port);
            _tcpSocket->connectToHost(_serverAddress.address, _serverAddress.port);
        }
    }
    else if (_udpSocket != NULL) {
        LOG_I("Cancelling any previous UDP operations...");
        _udpSocket->abort();
        _udpSocket->bind(_hostAddress.address, _hostAddress.port);
    }
    if (_tcpServer != NULL) {
        if (!_tcpServer->isListening()) {
            LOG_I("Waiting for TCP client to connect on port " + QString::number(_hostAddress.port));
            _tcpServer->listen(_hostAddress.address, _hostAddress.port);
        }
        if (_tcpServer->hasPendingConnections()) {
            //allow the next pending connection since we just dropped the old one
            newTcpClient();
        }
    }
}

void Channel::route(Channel *receiver) {
    LOG_I("Messages received on this channel are now being forwarded to " + receiver->LOG_TAG);
    connect(this, SIGNAL(messageReceived(Channel*,QByteArray)), receiver, SLOT(routeMessage(Channel*, QByteArray)));
}

void Channel::unroute(Channel *receiver) {
    disconnect(this, SIGNAL(messageReceived(Channel*,QByteArray)), receiver, SLOT(routeMessage(Channel*, QByteArray)));
}

inline void Channel::sendHandshake() {   //PRIVATE
    LOG_I("Sending handshake to " + _peerAddress.toString());
    sendMessage(_nameUtf8, (_isServer ? MSGTYPE_SERVER_HANDSHAKE : MSGTYPE_CLIENT_HANDSHAKE));
}

inline void Channel::sendHeartbeat() {   //PRIVATE
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

bool Channel::sendMessage(const QByteArray &message, MESSAGE_TYPE type) {
    qint64 status;
    if (_protocol == UdpProtocol) {
        QByteArray arr("", UDP_HEADER_BYTES);
        arr.reserve(3);
        QDataStream stream(&arr, QIODevice::WriteOnly);
        stream << (MESSAGE_TYPE)type;
        stream << (MESSAGE_ID)qToBigEndian(_nextSendID);
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
    _lastSendTime = QTime::currentTime();
    _sentTimeLog[_sentTimeLogIndex] = _lastSendTime;
    _nextSendID++;
    _sentTimeLogIndex++;
    if (_sentTimeLogIndex >= _sentLogCap) {
        _sentTimeLogIndex = 0;
    }
    return true;
}

QString Channel::getName() const {
    return _name;
}

Channel::Protocol Channel::getProtocol() const {
    return _protocol;
}

bool Channel::isServer() const {
    return _isServer;
}

Channel::SocketAddress Channel::getPeerAddress() const {
    return _peerAddress;
}

Channel::State Channel::getState() const {
    return _state;
}
