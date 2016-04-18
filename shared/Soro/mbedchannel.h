/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

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
#endif

#include <cstring>
#include "soro_global.h"

#define _MBED_MSG_TYPE_NORMAL 1
#define _MBED_MSG_TYPE_LOG 2

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

    void setChannelState(MbedChannel::State state);

private slots:
    void socketError(QAbstractSocket::SocketError err);
    void socketReadyRead();
    void resetConnection();

public:
    MbedChannel(SocketAddress host, unsigned char mbedId, QObject *parent = NULL, Logger *log = NULL);
    ~MbedChannel();

    void sendMessage(const char *message, int length);

signals:
    void messageReceived(const char* message, int length);
    void stateChanged(MbedChannel::State state);

protected:
    void timerEvent(QTimerEvent *e);

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

    void panic();
    void loadConfig();
    inline void initConnection();
    void sendMessage(char *message, int size, char type);

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
    MbedChannel(unsigned char mbedId);

    ~MbedChannel();

    /* Sets the timeout for blocking socket operations in milliseconds.
    */
    void setTimeout(unsigned int millis);

    inline void sendMessage(char *message, int length) {
        sendMessage(message, length, _MBED_MSG_TYPE_NORMAL);
    }

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
#endif // MBEDCHANNEL_H
