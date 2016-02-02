#include "channel.h"

const QString Channel::IPV4_REGEX = "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";
const QString Channel::IPV6_REGEX = "(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))";

Channel::Channel (QObject *parent, QString configFile, Logger *log) : QObject(parent) {
    _log = log;
    LOG_TAG = "CHANNEL";
    _tcpServer = NULL;
    _tcpSocket = NULL;
    _udpSocket = NULL;
    _socket = NULL;
    _buffer = NULL;
    _watchdogTimer = NULL;

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

Channel::Channel (QObject *parent, QUrl configUrl, Logger *log) : QObject(parent) {
    _log = log;
    LOG_TAG = "CHANNEL";
    _tcpServer = NULL;
    _tcpSocket = NULL;
    _udpSocket = NULL;
    _socket = NULL;
    _buffer = NULL;

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
    if (_sentTimeTable != NULL) {
        delete _sentTimeTable;
    }
    //Qt will take care of cleaning up any object that has 'this' passed to it in its constructor
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
    
    //these items must exits
    bool foundProtocol = false;
    bool foundName = false;
    bool foundServerAddress = false;
    bool foundServerPort = false;
    bool foundEndPoint = false;

    const QString usingDefaultWarningString = "Cannot parse value for %1 in configuration, using default value";
    const QString parseErrorString = "Cannot parse value for %1 in configuration";
    
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;
        int sepIndex = line.indexOf(' ');
        if (sepIndex < 0) continue;
        QString tag = line.mid(0, sepIndex).trimmed().toLower();
        QString value = line.mid(sepIndex + 1).trimmed();
        if (tag == CONFIG_TAG_SERVER_ADDRESS) {
            if (QRegExp(IPV4_REGEX).exactMatch(value) || QRegExp(IPV6_REGEX).exactMatch(value)) {
                _serverAddress.address = QHostAddress(value);
                foundServerAddress = true;
            }
            else {
                LOG_E(parseErrorString.arg(CONFIG_TAG_SERVER_ADDRESS));
            }
        }
        else if (tag == CONFIG_TAG_HOST_ADDRESS) {
            if (QRegExp(IPV4_REGEX).exactMatch(value) || QRegExp(IPV6_REGEX).exactMatch(value)) {
                _hostAddress.address = QHostAddress(value);
            }
            else {
                LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_HOST_ADDRESS));
            }
        }
        else if (tag == CONFIG_TAG_SERVER_PORT) {
            bool valid;
            _serverAddress.port = value.toInt(&valid);
            if (!valid) {
                LOG_E(parseErrorString.arg(CONFIG_TAG_SERVER_PORT));
                continue;
            }
            foundServerPort = true;
        }
        else if (tag == CONFIG_TAG_WATCHDOG_INTERVAL) {
            bool valid;
            _watchdogInterval = value.toInt(&valid);
            if (!valid) {
                LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_WATCHDOG_INTERVAL));
                _watchdogInterval = DEFAULT_WATCHDOG_INTERVAL;
            }
        }
        else if (tag == CONFIG_TAG_IDLE_CONNECTION_TIMEOUT) {
            bool valid;
            _idleConnectionTimeout = value.toInt(&valid);
            if (!valid) {
                LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_IDLE_CONNECTION_TIMEOUT));
                _idleConnectionTimeout = DEFAULT_IDLE_CONNECTION_TIMEOUT;
            }
        }
        else if (tag == CONFIG_TAG_TCP_VERIFY_TIMEOUT) {
            bool valid;
            _tcpVerifyTimeout = value.toInt(&valid);
            if (!valid) {
                LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_TCP_VERIFY_TIMEOUT));
                _tcpVerifyTimeout = DEFAULT_TCP_VERIFY_TIMEOUT;
            }
        }
        else if (tag == CONFIG_TAG_SENT_LOG_CAP) {
            bool valid;
            _sentLogCap = value.toInt(&valid);
            if (!valid) {
                LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_SENT_LOG_CAP));
                _sentLogCap = DEFAULT_SENT_LOG_CAP;
            }
        }
        else if (tag == CONFIG_TAG_STATISTICS_INTERVAL) {
            bool valid;
            _statisticsInterval = value.toInt(&valid);
            if (!valid) {
                LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_STATISTICS_INTERVAL));
                _statisticsInterval = DEFAULT_STATISTICS_INTERVAL;
            }
        }
        else if (tag == CONFIG_TAG_DROP_OLD_PACKETS) {
            bool valid;
            _dropOldPackets = parseBoolString(value, &valid);
            if (!valid) {
                LOG_W(usingDefaultWarningString.arg(CONFIG_TAG_DROP_OLD_PACKETS));
                _dropOldPackets = false;
            }
        }
        else if (tag == CONFIG_TAG_CHANNEL_NAME) {
            _name = value;
            foundName = true;
        }
        else if (tag == CONFIG_TAG_PROTOCOL) {
            if (value.toLower() == "udp") {
                _protocol = UdpProtocol;
            }
            else if (value.toLower() == "tcp") {
                _protocol = TcpProtocol;
            }
            else {
                LOG_E(parseErrorString.arg(CONFIG_TAG_PROTOCOL));
                continue;
            }
            foundProtocol = true;
        }
        else if (tag == CONFIG_TAG_ENDPOINT) {
            if (value.toLower() == "server") {
                _isServer = true;
            }
            else if (value.toLower() == "client") {
                _isServer = false;
            }
            else {
                LOG_E(parseErrorString.arg(CONFIG_TAG_ENDPOINT));
                continue;
            }
            foundEndPoint = true;
        }
        else {
            LOG_W("Configuration contains unknown tag " + tag);
        }
    }

    if (!foundProtocol | !foundServerAddress | !foundServerPort | !foundEndPoint | !foundName) {
        //config file was incomplete or invalid
        LOG_E("Coniguration file was incomplete");
        setState(ErrorState);
        return;
    }
    
    //log tag for debugging
    LOG_TAG = _name + (_isServer ? "(S)" : "(C)");

    //create a buffer for storing received messages
    _buffer = new char[1024];

    LOG_I("Initializing with serverAddress=" + _serverAddress.toString()
          + ",protocol=" + (_protocol == TcpProtocol ? "TCP" : "UDP"));

    //Create a QHash for storing sent message ID's and times for QoS monitoring
    _sentTimeTable = _isServer ? new QHash<MESSAGE_ID, QTime>() : NULL;

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
        configureNewTcpSocket(); //This is its own function, as it must be called every time a new
                                 //TCP client connects in a server scenario
    }
    setState(ReadyState); //now safe to call open()
}

inline bool Channel::parseBoolString(QString string, bool* value) { //PRIVATE
    string = string.toLower();
    if (string == "true" || string == "1") {
        *value = true;
        return true;
    }
    else if (string == "false" || string == "0") {
        *value = false;
        return true;
    }
    return false;
}

void Channel::open() {
    if (_state == ReadyState) {
        //If this is the server, we will bind to the server port
        if (_isServer) {
            _hostAddress.port = _serverAddress.port;
            LOG_I("Attempting to bind to " + _hostAddress.toString());
        }
        else {
            _hostAddress.port = 0;
            LOG_I("Attempting to bind to an available port on " + _hostAddress.address.toString());
        }
        if ((_protocol == TcpProtocol) & _isServer) {
            _socket = _tcpSocket = NULL;
            _tcpServer->setMaxPendingConnections(1);
            resetConnection();
        }
        else {
            _socket->bind(_hostAddress.address, _hostAddress.port);
        }
        if (!_watchdogTimer->isActive()) {
            _watchdogTimer->start();
        }
        //start the connection procedure
        resetConnection();
    }
    else {
        LOG_E("Cannot call open() in the current channel state");
    }
}

void Channel::configureNewTcpSocket() {
    //set the signals for a new TCP socket
    if (_tcpSocket != NULL) {
        _socket = _tcpSocket;
        connect(_socket, SIGNAL(readyRead()), this, SLOT(tcpReadyRead()));
        connect(_socket, SIGNAL(connected()), this, SLOT(tcpConnected()));
        connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(socketError(QAbstractSocket::SocketError)));
    }
}

void Channel::routeMessage(Channel *sender, QByteArray message) {  //PRIVATE SLOT
    sendMessage(message);
}

void Channel::socketError(QAbstractSocket::SocketError err) { //PRIVATE SLOT
    switch (err) {
    case QAbstractSocket::SocketAddressNotAvailableError:
        LOG_E("FATAL: The address is not available to open on this device. Select a different host address");
        close(ErrorState);
        break;
    case QAbstractSocket::AddressInUseError:
        LOG_E("FATAL: Address in use. Select a different port");
        close(ErrorState);
        break;
    default:
        LOG_E("Connection Error: " + _socket->errorString());
        setState(DisconnectedState); //this will force the timer to reset the connection
        break;
    }
}

void Channel::serverError(QAbstractSocket::SocketError err) { //PRIVATE SLOT
    switch (err) {
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

void Channel::close(Channel::State status) {   //PRIVATE
    LOG_W("Closing channel with status " + QString::number(status));
    if (_watchdogTimer->isActive()) {
        _watchdogTimer->stop();
    }
    if (_socket != NULL) {
        _socket->close();
    }
    if (_tcpServer != NULL) {
        _tcpServer->close();
    }
    setState(status);
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
        QByteArray arr(_buffer, _bufferLength);
        QDataStream stream(&arr, QIODevice::ReadOnly);
        stream >> type;
        //ensure the datagram either came from the correct address, or is marked as a handshake
        if (((address.address == _peerAddress.address) & (address.port == _peerAddress.port))
                | (_isServer & (type == MSGTYPE_CLIENT_HANDSHAKE))) {
            stream >> ID;
            ID = qFromBigEndian(ID);
            QByteArray message(_buffer + UDP_HEADER_BYTES, _bufferLength - UDP_HEADER_BYTES);
            processBufferedMessage(type, ID, message, address);
        }
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
            QByteArray arr(_buffer, TCP_HEADER_BYTES);
            QDataStream stream(&arr, QIODevice::ReadOnly);
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
                QByteArray message(_buffer + TCP_HEADER_BYTES, _bufferLength - TCP_HEADER_BYTES);
                _bufferLength = 0;
                processBufferedMessage(type, ID, message, _peerAddress);
            }
        }
    }
}

void Channel::processBufferedMessage(MESSAGE_TYPE type, MESSAGE_ID ID, QByteArray message, SocketAddress address) {
    switch (type) {
    case MSGTYPE_NORMAL:
        //normal data packet
        //check the packet sequence ID
        if ((ID > _lastReceiveID) | !_dropOldPackets){
            _lastReceiveID = ID;
            emit messageReceived(this, message);
            _lastReceiveTime = QTime::currentTime();
        }
        break;
    case MSGTYPE_SERVER_HANDSHAKE:
        //this packet is a handshake request
        if (!_isServer && compareHandshake(message)) {
            if (compareHandshake(message)) {
                //we are the client, and we got a respoonse from the server (yay)
                resetConnectionVars();
                setPeerAddress(address);
                setState(ConnectedState);
                if (_protocol == TcpProtocol) _tcpVerified = true;
                LOG_I("Received handshake response from server " + _serverAddress.toString());
                _lastReceiveTime = QTime::currentTime();
                _lastReceiveID = ID;
            }
            else {
                LOG_W("Received server handshake with invalid channel name");
            }
        }
        return;
    case MSGTYPE_CLIENT_HANDSHAKE:
        if (_isServer && compareHandshake(message)) {
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
                LOG_I("Received handshake request from client "
                                        + _peerAddress.toString());
                _lastReceiveTime = QTime::currentTime();
                _lastReceiveID = ID;
            }
            else {
                LOG_W("Received client handshake with invalid channel name");
            }
        }
        return;
    case MSGTYPE_HEARTBEAT:
        _lastReceiveTime = QTime::currentTime();
        break;
    case MSGTYPE_ACK:
        if (_isServer && _sentTimeTable->contains(ID)) {
            int rtt = _sentTimeTable->value(ID).msecsTo(QTime::currentTime());
            emit statisticsUpdate(this, rtt, _messagesUp, _messagesDown);
            _sentTimeTable->remove(ID);
        }
        break;
    default:
        LOG_W("Peer sent a message with an invalid header (type=" + QString::number(type) + ")");
        resetConnection();
        return;
    }
    _messagesDown++;
    if (!_isServer & (_lastStatisticsSendTime.msecsTo(QTime::currentTime()) >= _statisticsInterval)) {
        _lastStatisticsSendTime = _lastReceiveTime;
        sendMessage(QByteArray(""), MSGTYPE_ACK, ID);
    }
}

inline bool Channel::compareHandshake(QByteArray message) { //PRIVATE
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

inline void Channel::setState(Channel::State status) {   //PRIVATE
    //signals the statusChanged event
     if (_state != status) {
         _state = status;
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

void Channel::resetConnectionVars() {
    _bufferLength = 0;
    _lastReceiveID = 0;
    _lastStatisticsSendTime = QTime::currentTime();
    _nextSendID = 1;
    _tcpVerifyTicks = 0;
    _messagesDown = _messagesUp = 0;
    if (_isServer && _sentTimeTable != NULL) _sentTimeTable->clear();
}

void Channel::resetConnection() {
    LOG_I("Attempting to connect to other side of channel...");
    setState(ConnectingState);
    resetConnectionVars();
    if (_isServer) {
        setPeerAddress(SocketAddress(QHostAddress::Null, 0));
        _sentTimeTable->clear();
    }
    else {
        //Only allow the server address to connect
        setPeerAddress(_serverAddress);
    }
    if (_tcpSocket != NULL) {
        if (_tcpSocket->state() == QAbstractSocket::ConnectedState) {
            LOG_I("Disconnecting from TCP peer "
                  + SocketAddress(_tcpSocket->peerAddress(), _tcpSocket->peerPort()).toString());
            _tcpSocket->disconnectFromHost();
        }
        if (_isServer) {
            LOG_I("Cleaning up TCP connection with old client...");
            if (_tcpSocket->state() == QAbstractSocket::BoundState) {
                _tcpSocket->close();
            }
            delete _tcpSocket;
            _tcpSocket = NULL;
        }
        else {
            LOG_I("Trying to establish TCP connection with server " + _serverAddress.toString());
            _tcpSocket->connectToHost(_serverAddress.address, _serverAddress.port);
        }
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
    sendMessage(QByteArray(""), MSGTYPE_HEARTBEAT);
}

bool Channel::sendMessage(QByteArray message) {
    if (_state == ConnectedState) {
        if (message.size() > MAX_MESSAGE_LENGTH) {
            LOG_W("Attempting to send a message that is too long, it will be truncated");
            message.truncate(MAX_MESSAGE_LENGTH);
        }
        return sendMessage(message, MSGTYPE_NORMAL);
    }
    else {
        LOG_W("Channel not connected, a message was not sent");
        return false;
    }
}

inline bool Channel::sendMessage(QByteArray message, MESSAGE_TYPE type) {  //PRIVATE
    return sendMessage(message, type, ++_nextSendID);
}

bool Channel::sendMessage(QByteArray message, MESSAGE_TYPE type, MESSAGE_ID ID) {  //PRIVATE
    qint64 status;
    if (_protocol == UdpProtocol) {
        QByteArray arr(NULL, message.size() + UDP_HEADER_BYTES);
        QDataStream stream(&arr, QIODevice::WriteOnly);
        stream << (MESSAGE_TYPE)type;
        stream << (MESSAGE_ID)qToBigEndian(ID);
        arr.append(message);
        status = _udpSocket->writeDatagram(arr, _peerAddress.address, _peerAddress.port);
    }
    else if (_tcpSocket != NULL) {
        MESSAGE_LENGTH size = message.size() + TCP_HEADER_BYTES;
        QByteArray arr(NULL, size);
        QDataStream stream(&arr, QIODevice::WriteOnly);
        stream << (MESSAGE_LENGTH)qToBigEndian(size);
        stream << (MESSAGE_TYPE)type;
        stream << (MESSAGE_ID)qToBigEndian(ID);
        arr.append(message);
        status = _tcpSocket->write(arr);
    }
    else {
        LOG_E("Attempted to send a message through a null TCP socket");
        return false;
    }
    _messagesUp++;
    _lastSendTime = QTime::currentTime();
    //Log the ID and time of sending the message
    if (_isServer) {
        _sentTimeTable->insert(_nextSendID, _lastSendTime);
        //remove an old entry
        _sentTimeTable->remove(_nextSendID - _sentLogCap);
    }
    if (status < 0) {
        LOG_W("Could not send message (status=" + QString::number(status) + ")");
        return false;
    }
    return true;
}

QString Channel::getName() {
    return _name;
}

Channel::Protocol Channel::getProtocol() {
    return _protocol;
}

bool Channel::isServer() {
    return _isServer;
}

Channel::SocketAddress Channel::getPeerAddress() {
    return _peerAddress;
}

Channel::State Channel::getState() {
    return _state;
}
