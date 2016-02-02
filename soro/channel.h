#ifndef CHANNEL_H
#define CHANNEL_H

#include "soro_global.h"
#include "logger.h"
#include <QUdpSocket>
#include <QTcpSocket>
#include <cstring>

#define UDP_HEADER_BYTES 5
#define TCP_HEADER_BYTES 7

#define DEFAULT_IDLE_CONNECTION_TIMEOUT 5000
#define DEFAULT_STATISTICS_INTERVAL 1000
#define DEFAULT_TCP_VERIFY_TIMEOUT 5000
#define DEFAULT_WATCHDOG_INTERVAL 100
#define DEFAULT_SENT_LOG_CAP 500

#define CONFIG_TAG_SERVER_ADDRESS "serveraddress"
#define CONFIG_TAG_SERVER_PORT "serverport"
#define CONFIG_TAG_CHANNEL_NAME "name"
#define CONFIG_TAG_PROTOCOL "protocol"
#define CONFIG_TAG_HOST_ADDRESS "hostaddress"
#define CONFIG_TAG_ENDPOINT "endpoint"
#define CONFIG_TAG_DROP_OLD_PACKETS "dropoldpackets"
#define CONFIG_TAG_WATCHDOG_INTERVAL "watchdoginterval"
#define CONFIG_TAG_STATISTICS_INTERVAL "statisticsinterval"
#define CONFIG_TAG_IDLE_CONNECTION_TIMEOUT "idletimeout"
#define CONFIG_TAG_TCP_VERIFY_TIMEOUT "tcpverifytimeout"
#define CONFIG_TAG_SENT_LOG_CAP "sentlogcap"

#define LOG_I(X) if (_log != NULL) _log->i(LOG_TAG, X)
#define LOG_W(X) if (_log != NULL) _log->w(LOG_TAG, X)
#define LOG_E(X) if (_log != NULL) _log->e(LOG_TAG, X)

typedef quint32 MESSAGE_ID;     //4 bytes
typedef quint8 MESSAGE_TYPE;    //1 byte
typedef quint16 MESSAGE_LENGTH; //2 bytes

/* Channels abstract over internet communication in a super-easy way, supporting either
 * TCP or UDP as the transport protocol.
 *
 *
 * The following information describes the inner workings of this class and is not
 * necessary to understand the API.
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
 *  - For a handshaking message, 'Type' is TYPE_HANDSHAKE, and the
 * message data contains the channel name the other side was given.
 *
 *  - For a heartbeat message, 'Type' is TYPE_HEARTBEAT, and the
 * message data is empty.
 *
 * The ID field uniquely identifies all messages sent by this endpoint. The ID value
 * increases (newer messages have higher IDs), so they also function an a sequence number
 * in UDP mode.
 */
class SOROSHARED_EXPORT Channel: public QObject {
    Q_OBJECT

public:
    
    /* Wrapper for a QHostAddress (IP) and quint16 (port) used to identify
     * the address of a network socket
     */
    struct SocketAddress {
        QHostAddress address;
        quint16 port;

        SocketAddress(QHostAddress address, quint16 port) {
            this->address = address;
            this->port = port;
        }

        SocketAddress() {
            this->address = QHostAddress::Any;
            this->port = 0;
        }

        inline QString toString() {
            return address.toString() + ":" + QString::number(port);
        }

        inline bool operator==(const SocketAddress& other) {
            return (address == other.address) & (port = other.port);
        }

        inline bool operator!=(const SocketAddress& other) {
            return !(*this == other);
        }
    };

    /* Protocol modes supported by a Channel
     */
    enum Protocol {
        UdpProtocol, TcpProtocol
    };

    /* Lists the current state a Channel can be in
     */
    enum State {
        AwaitingConfigurationState,
        ReadyState,
        ConnectingState,
        ConnectedState,
        DisconnectedState,
        ErrorState
    };

    //The maximum size of a sent message (the header may make the actual message
    //slighty larger)
    static const MESSAGE_LENGTH MAX_MESSAGE_LENGTH = 512;

    ~Channel();

    Channel (QObject *parent, QString configFile, Logger *log);
    Channel (QObject *parent, QUrl configUrl, Logger *log);
    QString getName();
    Protocol getProtocol();
    SocketAddress getPeerAddress();
    void route(Channel *receiver);
    void unroute(Channel *receiver);
    void open();
    void close();
    bool sendMessage(QByteArray message);
    bool isServer();
    void setDropOldPackets(bool dropOld);
    State getState();

private:
    //Message type identifiers
    static const MESSAGE_TYPE MSGTYPE_NORMAL = 0;
    static const MESSAGE_TYPE MSGTYPE_CLIENT_HANDSHAKE = 1;
    static const MESSAGE_TYPE MSGTYPE_SERVER_HANDSHAKE = 2;
    static const MESSAGE_TYPE MSGTYPE_HEARTBEAT = 3;
    static const MESSAGE_TYPE MSGTYPE_ACK = 4;

    static const QString IPV4_REGEX;
    static const QString IPV6_REGEX;

    char *_buffer;
    MESSAGE_LENGTH _bufferLength;
    QByteArray _nameUtf8;
    State _state;
    QHash<MESSAGE_ID, QTime>* _sentTimeTable;
    bool _qosAckNextMessage;
    QString _name;
    QString LOG_TAG;
    SocketAddress _serverAddress;
    SocketAddress _hostAddress;
    SocketAddress _peerAddress;
    Protocol _protocol;
    QUrl _netConfigUrl;
    QNetworkReply *_netConfigReply;
    int _tcpVerifyTimeout;
    int _sentLogCap;
    int _statisticsInterval;
    int _idleConnectionTimeout;
    int _watchdogInterval;
    Logger* _log;
    bool _isServer;
    bool _dropOldPackets;
    int _tcpVerifyTicks;
    bool _tcpVerified;
    QTcpSocket *_tcpSocket;
    QTcpServer *_tcpServer;
    QUdpSocket *_udpSocket;
    QAbstractSocket *_socket;
    MESSAGE_ID _nextSendID;
    MESSAGE_ID _lastReceiveID;
    quint64 _messagesUp;
    quint64 _messagesDown;
    QTimer *_watchdogTimer;
    QTime _lastReceiveTime;
    QTime _lastSendTime;
    QTime _lastStatisticsSendTime;

    inline void setState(State state);
    inline void setPeerAddress(Channel::SocketAddress address);
    inline bool sendMessage(QByteArray message, MESSAGE_TYPE type);
    bool sendMessage(QByteArray message, MESSAGE_TYPE type, MESSAGE_ID ID);
    void close(State closeState);
    inline bool compareHandshake(QByteArray message);
    inline void sendHandshake();
    inline void sendHeartbeat();
    void resetConnection();
    void processBufferedMessage(MESSAGE_TYPE type, MESSAGE_ID ID,
                                QByteArray message, SocketAddress address);
    void configureNewTcpSocket();
    void resetConnectionVars();
    void initWithConfiguration(QTextStream &stream);
    bool parseBoolString(QString string, bool* value);

private slots:
    void routeMessage(Channel *sender, QByteArray message);
    void udpReadyRead();
    void tcpReadyRead();
    void watchdogTick();
    void tcpConnected();
    void newTcpClient();
    void socketError(QAbstractSocket::SocketError err);
    void serverError(QAbstractSocket::SocketError err);

signals:
    void messageReceived(Channel *channel, QByteArray message);
    void stateChanged(Channel *channel, Channel::State state);
    void peerAddressChanged(Channel *channel, Channel::SocketAddress peerAddress);
    void statisticsUpdate(Channel *channel, int rtt, quint64 msg_up, quint64 msg_down);
};

#endif // CHANNEL_H
