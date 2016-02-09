#ifndef CHANNEL_H
#define CHANNEL_H

#include "soro_global.h"
#include "logger.h"
#include "tagvalueparser.h"

#include <QUdpSocket>
#include <QTcpSocket>

#include <cstring>  //for memcmp

/* Channels abstract over message-based internet communication in a super easy way,
 * supporting either TCP or UDP as the transport protocol.
 *
 * The following information describes the inner workings of this class and is not
 * necessary to understand the API.
 *
 * The following headers are added to messages sent through a channel:
 *
 * -------- TCP message structure --------
 *  (2 bytes)   Packet Length                   (quint16)
 *  (1 byte)    Type                            (quint8)
 *  (4 bytes)   ID                              (quint32)
 *  ~ Message Data ~
 *
 * -------- UDP message structure --------
 *  (1 byte)    Type                            (quint8)
 *  (4 bytes)   ID                              (quint32)
 *  ~ Message Data ~
 *
 * The 'Type' field contains information about the data contained in the message.
 *  - For a normal message, 'Type' is TYPE_NORMAL.
 *
 * The ID field uniquely identifies all messages sent by this endpoint. The ID value
 * increases (newer messages have higher IDs), so they also function an a sequence number
 * in UDP mode.
 */
class SOROSHARED_EXPORT Channel: public QObject {
    Q_OBJECT

private:
    //data types used for header information
    typedef quint32 MESSAGE_ID;     //4 bytes, unsigned 32-bit int
    typedef quint8 MESSAGE_TYPE;    //1 byte, unsigned byte
    typedef quint16 MESSAGE_LENGTH; //2 bytes, unsigned 16-bit int

    //Message type identifiers
    static const MESSAGE_TYPE MSGTYPE_NORMAL = 0;
    static const MESSAGE_TYPE MSGTYPE_CLIENT_HANDSHAKE = 1;
    static const MESSAGE_TYPE MSGTYPE_SERVER_HANDSHAKE = 2;
    static const MESSAGE_TYPE MSGTYPE_HEARTBEAT = 3;
    static const MESSAGE_TYPE MSGTYPE_ACK = 4;

public:

    /* Wrapper for a QHostAddress (IP) and quint16 (port) used to identify
     * the address of a network socket
     */
    struct SocketAddress {
        QHostAddress address;
        quint16 port;

        SocketAddress(const QHostAddress &address, quint16 port) {
            this->address = address;
            this->port = port;
        }

        SocketAddress() {
            this->address = QHostAddress::Any;
            this->port = 0;
        }

        QString toString() const {
            return address.toString() + ":" + QString::number(port);
        }

        bool operator==(const SocketAddress& other) const {
            //In a situations where we are working with IPv4 addresses, but the :ffff: (IPv6)
            //prefix is added to only one of them, simple 'a == b' will fail
            Q_IPV6ADDR a = address.toIPv6Address();
            Q_IPV6ADDR b = other.address.toIPv6Address();
            return (std::memcmp(&a, &b, sizeof(Q_IPV6ADDR)) == 0)
                    & (port == other.port);
        }

        inline bool operator!=(const SocketAddress& other) const {
            return !(*this == other);
        }

    };

    /* Protocol modes supported by a channel
     */
    enum Protocol {
        UdpProtocol, TcpProtocol
    };

    /* Lists the state a channel can be in
     */
    enum State {
        AwaitingConfigurationState, //The channel is receiving a configuration file from a network
                                    //resource

        ReadyState, //The channel is ready to be connected as soon as open() is called

        ConnectingState,    //The channel is attempting to connect

        ConnectedState, //The channel is connected to the other side

        ErrorState  //The channel has encountered an unrecoverable error and cannot be used
                    //This is usually do to invalid configuration (specifying an unbindable port or host)
    };

    //The maximum size of a sent message (the header may make the actual message
    //slighty larger)
    static const MESSAGE_LENGTH MAX_MESSAGE_LENGTH = 500;

    /* Creates a new Channel with a local configuration file
     */
    Channel (QObject *parent, const QString &configFile, Logger *log);

    /* Creates a new Channel with the configuration file on a network server
     */
    Channel (QObject *parent, const QUrl &configUrl, Logger *log);

    ~Channel();

    /* Gets the name the channel was configured with
     */
    QString getName() const;

    /* Gets the protocol the channel was configured with
     */
    Channel::Protocol getProtocol() const;

    /* Gets the address of the currently connected peer
     */
    Channel::SocketAddress getPeerAddress() const;

    /* Opens the channel and attempts to connect. This may not have an
     * immediate effect if the channel is receiving its configuration from
     * a network resource
     */
    void open();

    /* Closes communication until Open() is called again
     */
    void close();

    /* Sends a message to the other side of the channel
     */
    bool sendMessage(const QByteArray &message);

    /* Returns true if this channel object acts as the server side
     */
    bool isServer() const;

    /* Gets the state the channel is currently in
     */
    Channel::State getState() const;

    /* Gets the uptime of the channel connection in seconds. Returns -1 if no
     * connection has been established
     */
    int getConnectionUptime() const;

    /* Gets the last calculated round trip time for the connection
     */
    int getLastRtt() const;

    /* Gets the number of messages send through this connection
     */
    quint64 getConnectionMessagesUp() const;

    /* Gets the number of messages received through this connection
     */
    quint64 getConnectionMessagesDown() const;

    int getDataRateUp() const;

    int getDataRateDown() const;

private:
    char *_buffer;  //buffer for received messages
    MESSAGE_LENGTH _bufferLength;

    QString _name;  //The name of the channel, also as a UTF8 byte array for handshaking
    QByteArray _nameUtf8;

    State _state;   //current state the channel is in

    qint64 *_sentTimeLog;   //Used for statistic calculation
    int _sentTimeLogIndex;
    qint64 _lastAckSendTime;
    qint64 _connectionEstablishedTime;
    int _lastRtt;

    QString LOG_TAG;    //Tag for debugging, ususally the
                        //channel name plus (S) for server or (C) for client

    SocketAddress _serverAddress;   //address of the server side of the channel
                                    //If we are the server, this may be 0 if the user
                                    //chose not to specify since it is not needed
    SocketAddress _hostAddress; //The address of the host to bind the channel to
    SocketAddress _peerAddress; //The address of the currently connected peer

    Protocol _protocol; //Protocol used by the channel (UDP or TCP)
    QUrl _netConfigFileUrl; //URL to obtain the configuration file from, if configuring
                        //over the internet

    QNetworkReply *_netConfigFileReply; //Used for network configuration file
    bool _openOnConfigured;

    Logger* _log;   //Logger object we are writing log messages to (can be null)

    bool _isServer;         //Holders for configuration preferences
    bool _dropOldPackets;
    bool _sendAcks;
    int _lowDelaySocketOption;

    QTcpSocket *_tcpSocket; //Currently active TCP socket
    QTcpServer *_tcpServer; //Currently active TCP server (for registering TCP clients)
    QUdpSocket *_udpSocket; //Currently active UDP socket
    QAbstractSocket *_socket;   //Pointer to either the TCP or UDP socket, depending on the configuration

    MESSAGE_ID _nextSendID; //ID to mark the next message with
    MESSAGE_ID _lastReceiveID;  //ID the most recent inbound message was marked with
    quint64 _messagesUp;    //Total number of sent messages
    quint64 _messagesDown;  //Total number of received messages
    quint64 _bytesUp;
    quint64 _bytesDown;
    int _dataRateUp;
    int _dataRateDown;

    int _connectionMonitorTimerID;  //Timer ID's for repeatedly executed tasks and watchdogs
    int _handshakeTimerID;
    int _resetTimerID;
    int _resetTcpTimerID;
    int _fetchNetConfigFileTimerID;

    qint64 _lastReceiveTime; //Last time a message was received
    qint64 _lastSendTime;
    qint64 _lastAckReceiveTime;

    inline void setState(State state);  //Internal method to set the channel status and
                                        //emit the statusChanged signal

    inline void setPeerAddress(Channel::SocketAddress address); //Internal method to set the channel peer
                                                                //address and emit the peerAddressChanged signal

    inline bool sendMessage(const QByteArray &message, MESSAGE_TYPE type);  //Internal method to send a message with
                                                                            //a specific type field

    bool sendMessage(const QByteArray &message, MESSAGE_TYPE type, MESSAGE_ID ID);  //Internal method to send a message with
                                                                                    //a specific type field and ID field

    void close(State closeState);   //Internal method to close the channel and set the closed state

    inline bool compareHandshake(const QByteArray &message) const;  //Compares a received handshake message with the correct one

    void processBufferedMessage(MESSAGE_TYPE type, MESSAGE_ID ID,
                                const QByteArray &message, const SocketAddress &address);   //Processes a received message

    void configureNewTcpSocket();   //Sets up a newly created TCP socket

    void resetConnectionVars(); //Resets variables relating to the current connection state,
                                //called when a new connection is established

    inline void initVars(); //Initializes variables when the channel is fist created (mostly nulling pointers)

    void initWithConfiguration(QTextStream &stream);    //Sets up a channel for use once the configuration source
                                                        //has been located and accessed as a QTextStream
    void resetConnection(); //Closes any existing connections and attempts to reconnect. Called in the event
                            //of a network error or dropped connection. Also called for initial connection.

    inline void sendHandshake();    //Sends a handshake message to the connected peer

    inline void sendHeartbeat();    //Sends a heartbeat message to the connected peer

    void fetchNetworkConfigFile();

private slots:
    void udpReadyRead();
    void tcpReadyRead();
    void tcpConnected();
    void newTcpClient();
    void connectionError(QAbstractSocket::SocketError err);
    void serverError(QAbstractSocket::SocketError err);
    void netConfigFileAvailable();
    void netConfigRequestError();

signals: //Always public

    /* Signal to notify an observer that a message has been received
     */
    void messageReceived(Channel *channel, const QByteArray &message);

    /* Signal to notify an observer that the state of the channel has changed
     */
    void stateChanged(Channel *channel, Channel::State state);

    /* Signal to notify an observer that the connected peer has changed
     * If the channel is no longer connected, peerAddress.address will be QHostAddress::Null
     *
     * Also, if this channel is the client side of a UDP connection, peer address will always be the
     * server address regardless of the state of the connection since that is the only allowed address
     * for communication.
     */
    void peerAddressChanged(Channel *channel,  const Channel::SocketAddress &peerAddress);

    /* Signal to notify an observer that the connection statistics have been updated
     */
    void statisticsUpdate(Channel *channel, int rtt, quint64 msg_up, quint64 msg_down, int rate_up, int rate_down);

    void connectionError(Channel* channel, QAbstractSocket::SocketError err);

protected:
    void timerEvent(QTimerEvent *);

};

#endif // CHANNEL_H
