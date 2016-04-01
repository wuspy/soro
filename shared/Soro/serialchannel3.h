#ifndef SERIALCHANNEL3_H
#define SERIALCHANNEL3_H

#ifdef QT_CORE_LIB
#   include <QSerialPort>
#   include <QSerialPortInfo>
#   include <QTimerEvent>
#   include <QtCore>
#   include <QDebug>
#   include <iostream>
#   include "soro_global.h"
#   include "logger.h"
#   define SERIAL_GETCHAR(s, c) s->getChar(&c)
#   define SERIAL_PUTCHAR(s, c) s->putChar(c)
#   define SERIAL_READABLE(s) s->bytesAvailable() > 0
#   define SERIAL_WRITEABLE(s) s->isOpen()
#   define SERIAL_OBJECT QSerialPort
#endif
#ifdef TARGET_LPC1768
#   include "mbed.h"
#   define SERIAL_GETCHAR(s, c) c = s->getc()
#   define SERIAL_PUTCHAR(s, c) s->putc(c)
#   define SERIAL_READABLE(s) s->readable()
#   define SERIAL_WRITEABLE(s) s->writeable()
#   define SERIAL_OBJECT Serial
extern "C" void mbed_reset();
#   define LOG_D(m) log(m, SERIAL_LOG_DEBUG_ID)
#   define LOG_I(m) log(m, SERIAL_LOG_INFO_ID)
#   define LOG_W(m) log(m, SERIAL_LOG_WARNING_ID)
#   define LOG_E(m) log(m, SERIAL_LOG_ERROR_ID)
#endif

#define SERIAL_MESSAGE_HEADER (unsigned char)255
#define SERIAL_HEARTBEAT_HEADER (unsigned char)254
#define SERIAL_HANDSHAKE_HEADER (unsigned char)253
#define SERIAL_NAME_HEADER (unsigned char)252
#define SERIAL_LOG_HEADER (unsigned char)251
#define SERIAL_MESSAGE_FOOTER (unsigned char)250

#define SERIAL_LOG_DEBUG_ID (unsigned char)1
#define SERIAL_LOG_INFO_ID (unsigned char)2
#define SERIAL_LOG_WARNING_ID (unsigned char)3
#define SERIAL_LOG_ERROR_ID (unsigned char)4

#define SERIAL_IDLE_CONNECTION_TIMEOUT 2000
#define SERIAL_HEARTBEAT_INTERVAL 1000

using namespace std;

#include <cstring>
#include <cstdio>

namespace Soro {

static const int MAX_VALUE_14BIT = 16383;

/* Encodes a number into 14 bits (2 bytes whrere each byte is limited to 0x7F)
 * This is done to ensure the high bit of each byte is always 0 for frame alignment purposes
 *
 * Range 0-16383
 */
static inline void serialize_14bit(unsigned short us, char *arr, int index) {
    arr[index] = (us >> 7) & 0x7F; /* low 7 bits of second byte, high bit of first byte */
    arr[index + 1] = us & 0x7F; //low 7 bits of first byte
}

/* Converts a float ranging from -1 to 1 into an unsigned char,
 * ranging from 0 to 200
 */
static inline char joyFloatToByte(float val) {
    val = (val + 1) * 100;
    unsigned char uc = (unsigned char)val;
    return reinterpret_cast<char&>(uc);
}

/* Converts an int ranging from -100 to 100 into an unsigned char,
 * ranging from 0 to 200
 */
static inline char joyIntToByte(int val) {
    val += 100;
    unsigned char uc = (unsigned char)val;
    return reinterpret_cast<char&>(uc);
}

/* Converts an int ranging from -100 to 100 into an unsigned char,
 * ranging from 0 to 200
 */
static inline int joyFloatToInt(float val) {
    return (int)(val * 100);
}

/* Converts a byte encoded joystick axis (see joyFloatToByte) back
 * into it's original float value
 */
static inline float joyByteToFloat(char val) {
    return (float)(reinterpret_cast<unsigned char&>(val) - 100.0) / 100.0;
}

/* Converts a byte encoded joystick axis (see joyFloatToByte) into
 * an int ranging from -100 to 100
 */
static inline int joyByteToInt(char val) {
    return (int)reinterpret_cast<unsigned char&>(val) - 100;
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
 */
#ifdef QT_CORE_LIB //////////////////////////////////
class SerialChannel3: public QObject {
    Q_OBJECT
#endif /////////////////////////////////////////////
#ifdef TARGET_LPC1768 //////////////////////////////
class SerialChannel3 {
#endif /////////////////////////////////////////////
public:
    /* Writes a message through the serial por
     * Messages are not null terminated, so the length must also be specified
     */
    inline void sendMessage(const char* message, int length) {
        #ifdef TARGET_LPC1768
        if (_receivedFirstHeartbeat) {
        #endif
        #ifdef QT_CORE_LIB
        if (_verified) {
        #endif
             sendMessage(message, length, SERIAL_MESSAGE_HEADER);
        }
    }

private:
    char _buffer[256];
    int _bufferLength;
    bool _messageAvailable;
    const char *_name;
    SERIAL_OBJECT *_serial;

    inline void writeMessageData(const char* message, int length) {
        for (int i = 0; i < length; i++) {
            SERIAL_PUTCHAR(_serial, message[i]);
        }
    }

    /* Writes a message through the serial port with the specified type header
     */
    void sendMessage(const char* message, int length, unsigned char header) {
        if (length < 253) {
            SERIAL_PUTCHAR(_serial, (char)header);
            SERIAL_PUTCHAR(_serial, (char)((unsigned char)length + 3));
            writeMessageData(message, length);
            SERIAL_PUTCHAR(_serial, (char)SERIAL_MESSAGE_FOOTER);
        }
    }

    /* Called when a valid message is received.
     */
    inline void resetMessageWatchdog() {
        #ifdef TARGET_LPC1768 ///////////////////////////////
        _lastReadTime = time(NULL);
        #endif //////////////////////////////////////////////
        #ifdef QT_CORE_LIB //////////////////////////////////
        _lastReadTime = QDateTime::currentMSecsSinceEpoch();
        #endif //////////////////////////////////////////////
    }

#ifdef QT_CORE_LIB ///////////////////////////////////////////
private slots:
#endif ///////////////////////////////////////////////////////
    void readSerial() {
        while (SERIAL_READABLE(_serial)) {
            if (_bufferLength == 256) {
                LOG_E("Buffer overflow while reading message");
                _bufferLength = 0;
            }
            SERIAL_GETCHAR(_serial, _buffer[_bufferLength]);
            _bufferLength++;
            if ((unsigned char)_buffer[_bufferLength - 1] == SERIAL_MESSAGE_FOOTER) {
                //a whole message has been read
                switch ((unsigned char)_buffer[0]) {
                #ifdef TARGET_LPC1768 ////////////////////////////////////////
                case SERIAL_MESSAGE_HEADER:
                    if (_bufferLength == (int)(unsigned char)_buffer[1]) { //verify length
                        if (_callback != NULL) {
                            _callback(_buffer + 2, _bufferLength - 3);
                        }
                        resetMessageWatchdog();
                    }
                    break;
                case SERIAL_HANDSHAKE_HEADER:
                    sendMessage(_name, strlen(_name) + 1, SERIAL_NAME_HEADER);
                    resetMessageWatchdog();
                    break;
                case SERIAL_HEARTBEAT_HEADER:
                    _receivedFirstHeartbeat = true;
                    resetMessageWatchdog();
                    break;
                #endif ////////////////////////////////////////////////////////
                #ifdef  QT_CORE_LIB ///////////////////////////////////////////
                case SERIAL_MESSAGE_HEADER:
                    if (_verified && (_bufferLength == (int)(unsigned char)_buffer[1])) { //verify length
                        emit messageReceived(_buffer + 2, _bufferLength - 3);
                        resetMessageWatchdog();
                    }
                    break;
                case SERIAL_NAME_HEADER:
                    if (!_verified) {
                        if (strcmp(_buffer + 2, _name) == 0) {
                            LOG_I(_serial->portName() + " is the correct serial port!!!!");
                            _verified = true;
                            KILL_TIMER(_handshakeTimerId);
                            KILL_TIMER(_searchTimerId);
                            START_TIMER(_heartbeatTimerId, SERIAL_HEARTBEAT_INTERVAL);
                            _state = ConnectedState;
                            emit stateChanged(_state);
                            resetMessageWatchdog();
                        }
                        else {
                            LOG_I("Got incorrect name " + QString::fromLatin1(_buffer + 2));
                            START_TIMER(_resetConnectionTimerId, 50);
                        }
                    }
                    else {
                        LOG_W("Got name message from an already verified serial port");
                        resetMessageWatchdog();
                    }
                    break;
                case SERIAL_LOG_HEADER:
                    if (_verified) {
                        switch (_buffer[1]) {
                        case SERIAL_LOG_DEBUG_ID:
                            LOG_D("MBED: " + QString(_buffer + 3));
                            break;
                        case SERIAL_LOG_INFO_ID:
                            LOG_I("MBED: " + QString(_buffer + 3));
                            break;
                        case SERIAL_LOG_WARNING_ID:
                            LOG_W("MBED: " + QString(_buffer + 3));
                            break;
                        case SERIAL_LOG_ERROR_ID:
                            LOG_E("MBED: " + QString(_buffer + 3));
                            break;
                        }
                        resetMessageWatchdog();
                    }
                    break;
                case SERIAL_HEARTBEAT_HEADER:
                    resetMessageWatchdog();
                    break;
                #endif /////////////////////////////////////////////////////////
                default:
                    LOG_W("Received corrupt or invalid message");
                    break;
                } //switch
                _bufferLength = 0;
            } //if
        } //while
    }

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
#ifdef QT_CORE_LIB

public:
    ~SerialChannel3() {
        delete _serial;
    }

    enum State {
        ConnectingState, ConnectedState
    };

    SerialChannel3(const char *name, QObject *parent, Logger *log): QObject(parent) {
        _log = log;
        _name = name;
        LOG_TAG = (QString)_name;
        _messageAvailable = false;
        _serialHandshakeIndex = 0;
        _bufferLength = 0;
        _serial = new QSerialPort(this);
        _state = ConnectingState;
        connect(_serial, SIGNAL(readyRead()),
               this, SLOT(readSerial()));
        connect(_serial, SIGNAL(error(QSerialPort::SerialPortError)),
                this, SLOT(serialError(QSerialPort::SerialPortError)));
        //begin looking for the correct serial port
        resetConnection();
    }

    inline State getState() const {
        return _state;
    }

private:
    int _searchTimerId = TIMER_INACTIVE;
    int _watchdogTimerId = TIMER_INACTIVE;
    int _heartbeatTimerId = TIMER_INACTIVE;
    int _resetConnectionTimerId = TIMER_INACTIVE;
    int  _handshakeTimerId = TIMER_INACTIVE;
    QList<QSerialPortInfo> _serialPorts;
    int _serialHandshakeIndex = 0;
    bool _verified = false;
    Logger *_log = NULL;
    State _state;
    qint64 _lastReadTime = 0;
    QString LOG_TAG;

    void resetConnection() {
        LOG_I("Attempting to reset the serial connection...");
        if (_serial->isOpen()) _serial->close();
        _bufferLength = 0;
        KILL_TIMER(_heartbeatTimerId);
        KILL_TIMER(_watchdogTimerId);
        KILL_TIMER(_handshakeTimerId);
        KILL_TIMER(_searchTimerId);
        START_TIMER(_searchTimerId, 500);
        _verified = false;
        if (_state != ConnectingState) {
            _state = ConnectingState;
            emit stateChanged(_state);
        }
    }

private slots:
    /* Slot that receives the QSerialPort's onError() signal
     */
    void serialError(QSerialPort::SerialPortError err) {
        if (err != QSerialPort::NoError) {
            if (_serial->isOpen()) _serial->close();
            _verified = false;
            LOG_E("Serial " + _serial->portName() + " experienced an error: " + _serial->errorString());
            if (_searchTimerId == TIMER_INACTIVE) {
                //wait a little bit before doing anything else, to prevent error loop
                START_TIMER(_resetConnectionTimerId, 1000);
            }
        }
    }

signals:
    void messageReceived(const char* message, int length);
    void stateChanged(SerialChannel3::State state);

protected:
    void timerEvent(QTimerEvent *e) {
        QObject::timerEvent(e);
        if (e->timerId() == _searchTimerId) {
            if (_serialHandshakeIndex >= _serialPorts.size()) {
                //we have searched all serial ports with no luck (or we're just starting out),
                //repopulate the serial list and try again
                _serialHandshakeIndex = 0;
                _serialPorts = QSerialPortInfo::availablePorts();
            }
            if (_serial->isOpen()) _serial->close();

            for (;_serialHandshakeIndex < _serialPorts.size(); _serialHandshakeIndex++) {

                #ifdef __linux__ ////////////////////////////////////////
                if (!_serialPorts[_serialHandshakeIndex].portName().startsWith("ttyACM")) continue;
                #endif //////////////////////////////////////////////////
                #ifdef __APPLE__ ////////////////////////////////////////
                if (!_serialPorts[_serialHandshakeIndex].portName().startsWith("tty.usbmodem")) continue;
                #endif //////////////////////////////////////////////////

                _serial->setPort(_serialPorts[_serialHandshakeIndex]);
                LOG_I("Trying serial port " + _serial->portName());
                if (_serial->open(QIODevice::ReadWrite)) {
                    LOG_I("Connected to serial port " + _serial->portName());
                    _lastReadTime = QDateTime::currentMSecsSinceEpoch();
                    _serial->setBaudRate(QSerialPort::Baud9600);
                    _serial->setDataBits(QSerialPort::Data8);
                    _serial->setParity(QSerialPort::NoParity);
                    _serial->setStopBits(QSerialPort::OneStop);
                    _verified = false;
                    KILL_TIMER(_searchTimerId);
                    START_TIMER(_handshakeTimerId, 100);
                    START_TIMER(_watchdogTimerId, 1000);
                    return;
                }

            }

        }
        else if (e->timerId() == _handshakeTimerId) {
            sendMessage(NULL, 0, SERIAL_HANDSHAKE_HEADER);
        }
        else if (e->timerId() == _heartbeatTimerId) {
            sendMessage(NULL, 0, SERIAL_HEARTBEAT_HEADER);
        }
        else if (e->timerId() == _watchdogTimerId) {
            if (QDateTime::currentMSecsSinceEpoch() - _lastReadTime >= SERIAL_IDLE_CONNECTION_TIMEOUT) {
                if (_verified) {
                    LOG_I("Serial port " + _serial->portName() + " is not responding");
                }
                else {
                    LOG_I("Serial port " + _serial->portName() + " did not respond in time");
                }
                START_TIMER(_resetConnectionTimerId, 100);
            }
        }
        else if (e->timerId() == _resetConnectionTimerId) {
            resetConnection();
            KILL_TIMER(_resetConnectionTimerId);
        }
    }

#endif
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
#ifdef TARGET_LPC1768

private:
    int _loopHeartbeatCounter;
    int _interval;
    time_t _lastReadTime;
    time_t _lastBeatTime;
    void (*_callback)(const char*, int);
    DigitalOut *_led;
    bool _receivedFirstHeartbeat;
    enum LogLevel {
        DebugLevel, InfoLevel, WarnLevel, ErrorLevel
    };

public:
    ~SerialChannel3() {
        delete _serial;
        delete _led;
    }

    SerialChannel3(const char *name, PinName tx, PinName rx, void(*callback)(const char*, int)) {
        _name = name;
        _callback = callback;
        _lastReadTime = time(NULL);
        _lastBeatTime = time(NULL);
        _loopHeartbeatCounter = 0;
        _bufferLength = 0;
        _messageAvailable = false;
        _led = new DigitalOut(LED4);
        _serial = new Serial(tx, rx);
        _receivedFirstHeartbeat = false;
        _serial->baud(9600);
        _serial->format(8, SerialBase::None, 1);
        //using an interrupt may be cauing the pwm delay issues
        _serial->attach(this, &SerialChannel3::readSerial);
    }

    /* Writes a log message from the mbed to the computer
     */
    void log(const char* message, unsigned char logLevel) {
        int length = strlen(message) + 1;
        if (length < 252) {
            SERIAL_PUTCHAR(_serial, (char)SERIAL_LOG_HEADER);
            SERIAL_PUTCHAR(_serial, (char)logLevel);
            SERIAL_PUTCHAR(_serial, (char)(unsigned char)(length + 4));
            writeMessageData(message, length);
            SERIAL_PUTCHAR(_serial, (char)SERIAL_MESSAGE_FOOTER);
        }
    }

    void check() {
        //readSerial();
        if (time(NULL) - _lastReadTime > SERIAL_IDLE_CONNECTION_TIMEOUT / 1000) {
            //We have gone too long without receiving a message, reset and hope
            //that fixes things
            for (int i = 0; i < 5; i++) { //fancy blinky
                *_led = 1;
                wait_ms(50);
                *_led = 0;
                wait_ms(50);
            }
            mbed_reset();
        }
        if (time(NULL) - _lastBeatTime > SERIAL_HEARTBEAT_INTERVAL / 1000) {
            sendMessage(NULL, 0, SERIAL_HEARTBEAT_HEADER);
            _lastBeatTime = time(NULL);
        }
    }
#endif
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
};

}

#endif // SERIALCHANNEL3_H
