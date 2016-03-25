#ifndef SERIALCHANNEL_H
#define SERIALCHANNEL_H

#ifdef QT_CORE_LIB
#   include <QSerialPort>
#   include <QSerialPortInfo>
#   include <QTimerEvent>
#   include <QtCore>
#   include <QDebug>
#   include "soroutil.h"
#   include "logger.h"
#   define SERIAL_GETC(s, c) s.getChar(&c)
#   define SERIAL_PUTC(s, c) s.putChar(c)
#   define SERIAL_READABLE(s) s.bytesAvailable() > 0
#   define SERIAL_OBJECT QSerialPort
#endif
#ifdef TARGET_LPC1768
#   include "mbed.h"
#   define SERIAL_GETC(s, c) c = s.getc()
#   define SERIAL_PUTC(s, c) s.putc(c)
#   define SERIAL_READABLE(s) s.readable()
#   define SERIAL_OBJECT Serial
#endif

#define SERIAL_MASTER_ARM_CHANNEL_NAME "MasterArm"
#define SERIAL_ARM_CHANNEL_NAME "Arm"
#define SERIAL_DRIVE_CHANNEL_NAME "Drive"
#define SERIAL_GIMBAL_CHANNEL_NAME "Gimbal"

#include <cstring>

#define MAX_VALUE_14BIT 16383
#define SERIAL_MESSAGE_HEADER (unsigned char)255
#define SERIAL_MESSAGE_FOOTER (unsigned char)254
#define SERIAL_MESSAGE_HANDSHAKE (unsigned char)253
#define SERIAL_MESSAGE_HEARTBEAT (unsigned char)252

namespace Soro {

    /* Encodes a number into 14 bits (2 bytes whrere each byte is limited to 0x7F)
     * This is done to ensure the high bit of each byte is always 0 for frame alignment purposes
     *
     * Range 0-16383
     */
    static inline void serialize_14bit(unsigned short us, char *arr, int index) {
        arr[index] = (us >> 7) & 0x7F; /* low 7 bits of second byte, high bit of first byte */
        arr[index + 1] = us & 0x7F; //low 7 bits of first byte
    }

    /* Decodes a number encoded with the above function
     *
     * Range 0-16383
     */
    static inline unsigned short deserialize_14bit(const char *arr, int index) {
        return (arr[index] << 7) | arr[index + 1];
    }

    /* This class handles everything about communication between a Qt and mbed enviornment over a serial port.
     * It will handle finding the correct tty device (Qt side) and monitoring the connection to ensure it is still
     * alive (if a connection appear to be dead, the mbed will reset itself)
     *
     * NEVER EVER send any data using this class with a byte >=250. These are used for frame alignment and could cause
     * horrible problems.
     */
#ifdef QT_CORE_LIB
    class SerialChannel: public QObject {
        Q_OBJECT
#endif
#ifdef TARGET_LPC1768
    class SerialChannel {
#endif
    public:
        /* Writes a message verbatim through the serial port
         */
        void sendMessage(const char* message, int size);

        ~SerialChannel();

    private:
        char _buffer[256];
        int _size;
        int _readIndex;
        bool _messageAvailable;
        const char *_name;
        SERIAL_OBJECT *_serial;

        /* Reads the next message from the serial port and returns the number of chars read
         *
         * This will set the messageAvailable flat to true if a complete message is read in,
         * but will clear it again the next time it is called and a complete message is not read
         */
        int read();

/************************************************************
 ************************************************************
 ************************************************************
 ************************************************************/
#ifdef QT_CORE_LIB

    public:
        enum State {
            ConnectingState, ConnectedState
        };

        SerialChannel(const char *name, QObject *parent, Logger *log);

        SerialChannel::State getState() const;

    private:
        int _handshakeTimerId = TIMER_INACTIVE;
        int _watchdogTimerId = TIMER_INACTIVE;
        int _heartbeatTimerId = TIMER_INACTIVE;
        QList<QSerialPortInfo> _serialPorts;
        int _serialHandshakeIndex = 0;
        bool _verified = false;
        Logger *_log = NULL;
        SerialChannel::State _state;
        void resetConnection();
        qint64 _lastReadTime = 0;

    private slots:
        /* Slot that receives the QSerialPort's readyRead() signal
         */
        void serialReadyRead();
        void serialError(QSerialPort::SerialPortError err);

    signals:
        void messageReceived(const char* message, int size);
        void stateChanged(SerialChannel::State state);

    protected:
        void timerEvent(QTimerEvent *e);

#endif
/************************************************************
 ************************************************************
 ************************************************************
 ************************************************************/
#ifdef TARGET_LPC1768

    private:
        int _loopsWithoutMessage;
        int _loopHeartbeatCounter;
        int _interval;
        DigitalOut *_led;
    public:
        SerialChannel(const char *name, PinName tx, PinName rx, int ms_interval);
        bool getAvailableMessage(char *outMessage, int& outSize);
        void process();
#endif
    };

}

#endif // SERIALCHANNEL_H
