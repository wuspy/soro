#ifndef SERIALCHANNEL2_H
#define SERIALCHANNEL2_H

#define SERIALCHANNEL_H

#ifdef QT_CORE_LIB
#   include <QSerialPort>
#   include <QSerialPortInfo>
#   include <QTimerEvent>
#   include <QtCore>
#   include <QDebug>
#   include <iostream>
#   include "soroutil.h"
#   include "logger.h"
#   define SERIAL_PRINTF(s, m) s.write(m)
#   define SERIAL_READLN(s, m) s.readLine(m, 1000)
#   define SERIAL_GETCHAR(s, c) s.getChar(&c)
#   define SERIAL_PUTCHAR(s, c) s.putChar(c)
#   define SERIAL_READABLE(s) s.bytesAvailable() > 0
#   define SERIAL_WRITEABLE(s) s.isOpen()
#   define SERIAL_OBJECT QSerialPort
#endif
#ifdef TARGET_LPC1768
#   include "mbed.h"
#   define SERIAL_PRINTF(s, m) s.printf(m)
#   define SERIAL_READLN(s, m) s.scanf("%s", m)
#   define SERIAL_GETCHAR(s, c) c = s.getc()
#   define SERIAL_PUTCHAR(s, c) s.putc(c)
#   define SERIAL_READABLE(s) s.readable()
#   define SERIAL_WRITEABLE(s) s.writeable()
#   define SERIAL_OBJECT Serial
extern "C" void mbed_reset();
#   define LOG_D(m) sendMessage(m, strlen(m) + 1, SERIAL_LOG_HEADER)
#   define LOG_I(m) sendMessage(m, strlen(m) + 1, SERIAL_LOG_HEADER)
#   define LOG_W(m) sendMessage(m, strlen(m) + 1, SERIAL_LOG_HEADER)
#   define LOG_E(m) sendMessage(m, strlen(m) + 1, SERIAL_LOG_HEADER)
#endif

#define SERIAL_MESSAGE_HEADER (unsigned char)255
#define SERIAL_HEARTBEAT_HEADER (unsigned char)254
#define SERIAL_HANDSHAKE_HEADER (unsigned char)253
#define SERIAL_NAME_HEADER (unsigned char)252
#define SERIAL_LOG_HEADER (unsigned char)251
#define SERIAL_MESSAGE_FOOTER (unsigned char)250

#define SERIAL_IDLE_CONNECTION_TIMEOUT 3000
#define SERIAL_HEARTBEAT_INTERVAL 1000

using namespace std;

#include <cstring>
#include <cstdio>

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
     */
#ifdef QT_CORE_LIB
    class SerialChannel3: public QObject {
        Q_OBJECT
#endif
#ifdef TARGET_LPC1768
    class SerialChannel3 {
#endif
    public:
        /* Writes a message verbatim through the serial port
         */
        inline void sendMessage(const char* message, int length) {
             sendMessage(message, length, SERIAL_MESSAGE_HEADER);
        }

    private:
        char _buffer[256];
        int _bufferLength;
        bool _messageAvailable;
        const char *_name;
        SERIAL_OBJECT *_serial;

        void sendMessage(const char* message, int length, unsigned char header) {
            if ((SERIAL_WRITEABLE((*_serial))) & (length < 253)) {
                SERIAL_PUTCHAR((*_serial), (char)header);
                SERIAL_PUTCHAR((*_serial), (char)((unsigned char)length + 3));
                for (int i = 0; i < length; i++) {
                    SERIAL_PUTCHAR((*_serial), message[i]);
                }
                SERIAL_PUTCHAR((*_serial), (char)SERIAL_MESSAGE_FOOTER);
            }
        }

#ifdef QT_CORE_LIB
    private slots:
#endif
        void readSerial() {
            while (SERIAL_READABLE((*_serial))) {
                SERIAL_GETCHAR((*_serial), _buffer[_bufferLength]);
                _bufferLength++;
                if (_bufferLength == 256) {
                    LOG_E("Buffer overflow while reading message");
                    _bufferLength = 0;
                    return;
                }
                if ((unsigned char)_buffer[_bufferLength - 1] == SERIAL_MESSAGE_FOOTER) {
                    //a whole message has been read
                    switch ((unsigned char)_buffer[0]) {
                    case SERIAL_HEARTBEAT_HEADER:
                        break;
#ifdef TARGET_LPC1768
                    case SERIAL_MESSAGE_HEADER:
                        if (_bufferLength == (int)(unsigned char)_buffer[1]) { //verify length
                            _callback(_buffer, _bufferLength);
                        }
                        break;
                    case SERIAL_HANDSHAKE_HEADER:
                        sendMessage(_name, strlen(_name) + 1, SERIAL_NAME_HEADER);
                        break;
                    default: return;
                    }
                    _lastReadTime = time(NULL);
#endif
#ifdef  QT_CORE_LIB
                    case SERIAL_MESSAGE_HEADER:
                        if (_bufferLength == (int)(unsigned char)_buffer[1]) { //verify length
                            emit messageReceived(_buffer + 2, _bufferLength);
                        }
                        break;
                    case SERIAL_NAME_HEADER:
                        if (!_verified) {
                            if (strcmp(_buffer + 2, _name) == 0) {
                                LOG_I(_serial->portName() + " is the correct serial port!!!!");
                                _verified = true;
                                START_TIMER(_heartbeatTimerId, SERIAL_HEARTBEAT_INTERVAL);
                                _state = ConnectedState;
                                emit stateChanged(_state);
                            }
                            else {
                                LOG_I("Got incorrect name " + QString::fromLatin1(_buffer + 2));
                                START_TIMER(_resetConnectionTimerId, 50);
                                return;
                            }
                        }
                        else {
                            LOG_W("Got name message from an already verified serial port");
                        }
                        break;
                    case SERIAL_LOG_HEADER:
                        break;
                    default: return;
                    }
                    _lastReadTime = QDateTime::currentMSecsSinceEpoch();
#endif
                    _bufferLength = 0;
                }
            }
        }

/************************************************************
 ************************************************************
 ************************************************************
 ************************************************************/
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
            LOG_TAG = (QString)_name + "(Serial)";
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
        int _handshakeTimerId = TIMER_INACTIVE;
        int _watchdogTimerId = TIMER_INACTIVE;
        int _heartbeatTimerId = TIMER_INACTIVE;
        int _resetConnectionTimerId = TIMER_INACTIVE;
        QList<QSerialPortInfo> _serialPorts;
        int _serialHandshakeIndex = 0;
        bool _verified = false;
        Logger *_log = NULL;
        State _state;
        qint64 _lastReadTime = 0;
        QString LOG_TAG;

        void resetConnection() {
            if (_serial->isOpen()) _serial->close();
            KILL_TIMER(_heartbeatTimerId);
            KILL_TIMER(_watchdogTimerId);
            START_TIMER(_handshakeTimerId, 100);
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
                LOG_E("Serial " + _serial->portName() + " experienced an error: " + _serial->errorString());
                START_TIMER(_resetConnectionTimerId, 50);
            }
        }

    signals:
        void messageReceived(const char* message, int length);
        void stateChanged(SerialChannel3::State state);

    protected:
        void timerEvent(QTimerEvent *e) {
            QObject::timerEvent(e);
            if (e->timerId() == _handshakeTimerId) {
                if (_serialHandshakeIndex >= _serialPorts.size()) {
                    //we have searched all serial ports with no luck (or we're just starting out),
                    //repopulate the serial list and try again
                    _serialHandshakeIndex = 0;
                    _serialPorts = QSerialPortInfo::availablePorts();
                }
                if (_serial->isOpen()) _serial->close();
                for (;_serialHandshakeIndex < _serialPorts.size(); _serialHandshakeIndex++) {
#ifdef __linux__
                if (!_serialPorts[_serialHandshakeIndex].portName().startsWith("ttyACM")) continue;
#endif
                    _serial->setPort(_serialPorts[_serialHandshakeIndex]);
                    LOG_D("Trying serial port " + _serial->portName());
                    if (_serial->open(QIODevice::ReadWrite)) {
                        LOG_I("Connected to serial port " + _serial->portName());
                        _lastReadTime = QDateTime::currentMSecsSinceEpoch();
                        _serial->setBaudRate(QSerialPort::Baud115200);
                        _serial->setDataBits(QSerialPort::Data8);
                        _serial->setParity(QSerialPort::NoParity);
                        _serial->setStopBits(QSerialPort::OneStop);
                        _verified = false;
                        sendMessage(NULL, 0, SERIAL_HANDSHAKE_HEADER);
                        KILL_TIMER(_handshakeTimerId);
                        START_TIMER(_watchdogTimerId, SERIAL_IDLE_CONNECTION_TIMEOUT);
                        return;
                    }
                }
            }
            else if (e->timerId() == _heartbeatTimerId) {
                sendMessage(NULL, 0, SERIAL_HEARTBEAT_HEADER);
            }
            else if (e->timerId() == _watchdogTimerId) {

                if (QDateTime::currentMSecsSinceEpoch() - _lastReadTime > SERIAL_IDLE_CONNECTION_TIMEOUT) {
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
/************************************************************
 ************************************************************
 ************************************************************
 ************************************************************/
#ifdef TARGET_LPC1768

    private:
        int _loopHeartbeatCounter;
        int _interval;
        time_t _lastReadTime;
        time_t _lastBeatTime;
        void (*_callback)(const char*, int);
        DigitalOut *_led;
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
            _serial->baud(115200);
            _serial->format(8, SerialBase::None, 1);
        }

        void checkMessages() {
            readSerial();
            if (time(NULL) - _lastReadTime > SERIAL_IDLE_CONNECTION_TIMEOUT / 1000) {
                //We have gone too long without receiving a message, reset and hope
                //that fixes things
                *_led = 1;
                wait(1);
                mbed_reset();
            }
            if (time(NULL) - _lastBeatTime > SERIAL_HEARTBEAT_INTERVAL / 1000) {
                sendMessage(NULL, 0, SERIAL_HEARTBEAT_HEADER);
                _lastBeatTime = time(NULL);
            }
        }
#endif
    };

}

#endif // SERIALCHANNEL_H
