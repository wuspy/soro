/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

#ifndef SORO_MBEDCHANNEL_H
#define SORO_MBEDCHANNEL_H

#ifdef QT_CORE_LIB
#   include <QtCore>
#   include <QUdpSocket>
#   include <QHostAddress>
#   include "soro_global.h"
#   include "socketaddress.h"
#   include "logger.h"
#endif
#ifdef TARGET_LPC1768
#   include "mbed.h"
#   include "EthernetInterface.h"
#   include "UDPSocket.h"
#   include "lpc_phy.h"
#endif

#include <cstring>
#include "constants.h"

namespace Soro {

#ifdef QT_CORE_LIB

/* Qt implementation.
 * The Qt side acts as the server and waits for the
 * Mbed to connect.
 */
class LIBSORO_EXPORT MbedChannel: public QObject {
    Q_OBJECT

public:
    enum State {
        ConnectedState, ConnectingState
    };

    MbedChannel::State getState() const;

private:
    QString LOG_TAG;
    QUdpSocket *_socket;
    SocketAddress _host;
    State _state;
    char _buffer[512];
    bool _active;
    char _mbedId;
    unsigned int _lastReceiveId;
    unsigned int _nextSendId = 0;
    int _watchdogTimerId = TIMER_INACTIVE;
    int _resetConnectionTimerId = TIMER_INACTIVE;
    void setChannelState(MbedChannel::State state);

private slots:
    void socketError(QAbstractSocket::SocketError err);
    void socketReadyRead();
    void resetConnection();

public:
    MbedChannel(SocketAddress host, unsigned char mbedId, QObject *parent = NULL);
    ~MbedChannel();

    void sendMessage(const char *message, int length);

signals:
    void messageReceived(MbedChannel *channel, const char* message, int length);
    void stateChanged(MbedChannel *channel, MbedChannel::State state);

protected:
    void timerEvent(QTimerEvent *e);

};

#endif
#ifdef TARGET_LPC1768

/* Mbed implementation.
 * The Mbed acts as a client, and attempts to locate the server through
 * UDP broadcasting.
 */
class MbedChannel {
private:
    EthernetInterface *_eth;
    UDPSocket *_socket;
    Endpoint _server;
    time_t _lastSendTime;
    time_t _lastReceiveTime;
    unsigned int _nextSendId;
    unsigned int _lastReceiveId;
    char _mbedId;
    char _buffer[512];
    void (*_resetCallback)(void);

    /* Called in the event of an invalid or missing config file
     */
    void panic();
    /* Loads the config file containg the server port, and sets the server
     * address as broadcast on that port. Ethernet must be initialized and
     * connected before this is called
     */
    bool setServerAddress();
    /* Resets the mbed after calling the reset listener (if it is set)
     */
    void reset();
    /* Initializes etherent and attempts to find the server through a UDP broadcast.
     * Blocks until completion.
     */
    inline void initConnection();

    void sendMessage(char *message, int size, unsigned char type);

    inline bool isEthernetActive () {
        return (lpc_mii_read_data() & (1 << 0));
    }

public:
    /* Creates a new channel for ethernet communication.
     *
     * This will read from a file on local storage (server.txt) to
     * determine the port used for communication.
     *
     * Blocks until completion, usually for several seconds.
     */
    MbedChannel(unsigned char mbedId);

    ~MbedChannel();

    /* Sets the timeout for blocking socket operations in milliseconds.
     * Does not allow no timeout or timeouts greater than IDLE_CONNECTION_TIMEOUT / 2.
     */
    void setTimeout(unsigned int millis);

    inline void sendMessage(char *message, int length) {
        sendMessage(message, length, _MBED_MSG_TYPE_NORMAL);
    }

    /* Sends a log message to the server.
     */
    inline void log(char *message) {
        sendMessage(message, strlen(message) + 1, _MBED_MSG_TYPE_LOG);
    }

    /* Adds a listener to be notified if the ethernet
     * becomes disconnected, which will result in the mbed
     * resetting. This gives you a chance to clean up
     * or perform any necessary operations before a reset.
     */
    void setResetListener(void (*callback)(void));

    /* Reads a pending message (if available) and stores it in outMessage.
     *
     * Returns the length of the message, or -1 if none is available or
     * an error occurred.
     */
    int read(char *outMessage, int maxLength);
};

#endif

}
#endif // SORO_MBEDCHANNEL_H
