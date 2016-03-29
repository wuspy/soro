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
#   define SERIAL_READABLE(s) s.canReadLine()
#   define SERIAL_WRITEABLE(s) s.isOpen()
#   define SERIAL_OBJECT QSerialPort
#endif
#ifdef TARGET_LPC1768
#   include "mbed.h"
#   define SERIAL_PRINTF(s, m) s.printf(m)
#   define SERIAL_READLN(s, m) s.scanf("%s", m)
#   define SERIAL_READABLE(s) s.readable()
#   define SERIAL_WRITEABLE(s) s.writeable()
#   define SERIAL_OBJECT Serial
extern "C" void mbed_reset();
#   define LOG_D(m) /* m */
#   define LOG_I(m) /* m */
#   define LOG_W(m) /* m */
#   define LOG_E(m) /* m */
#endif

#define SERIAL_MESSAGE_HEADER 'M'
#define SERIAL_HEARTBEAT_HEADER 'B'
#define SERIAL_HANDSHAKE_HEADER 'H'
#define SERIAL_NAME_HEADER 'N'
#define SERIAL_LOG_DEBUG_HEADER 'D'
#define SERIAL_LOG_INFO_HEADER 'I'
#define SERIAL_LOG_WARN_HEADER 'W'
#define SERIAL_LOG_ERROR_HEADER 'E'

#define SERIAL_IDLE_CONNECTION_TIMEOUT 2000
#define SERIAL_HEARTBEAT_INTERVAL 250

using namespace std;


#include <cstring>
#include <cstdio>

namespace Soro {

    /* This class handles everything about communication between a Qt and mbed enviornment over a serial port.
     * It will handle finding the correct tty device (Qt side) and monitoring the connection to ensure it is still
     * alive (if a connection appear to be dead, the mbed will reset itself)
     */
#ifdef QT_CORE_LIB
    class SerialChannel2: public QObject {
        Q_OBJECT
#endif
#ifdef TARGET_LPC1768
    class SerialChannel2 {
#endif
    public:
        /* Writes a message verbatim through the serial port
         */
        inline void sendMessage(const char* message) {
             sendMessage(message, SERIAL_MESSAGE_HEADER);
        }

    private:
        char _buffer[256];
        char _sendBuffer[256];
        bool _messageAvailable;
        const char *_name;
        SERIAL_OBJECT *_serial;

        void sendMessage(const char* message, char header) {
            int length = strlen(message) + 3;
            _sendBuffer[0] = 'F';
            _sendBuffer[1] = header;
            if (length > 3) {
                strcpy(&_sendBuffer[2], message);
            }
            _sendBuffer[length - 1] = '\n'; //add newline (for bullshit odroid reasons)
            _sendBuffer[length] = '\0'; //null-terminate the string
            if (SERIAL_WRITEABLE((*_serial))) {
                SERIAL_PRINTF((*_serial), _sendBuffer);
            }
        }

#ifdef QT_CORE_LIB
    private slots:
#endif
        void readSerial() {
            while (SERIAL_READABLE((*_serial))) {
                SERIAL_READLN((*_serial), _buffer);
                if (_buffer[0] != 'F') {
                    LOG_I("Frame alignment has been lost");
                    return; //invalid header char
                }
                int len = strlen(_buffer);
                if (_buffer[len - 1] == '\n') {
                    _buffer[len - 1] = '\0'; //null-terminate the message to get rid of newline char
                }
                switch (_buffer[1]) {
#ifdef QT_CORE_LIB
                case SERIAL_MESSAGE_HEADER:
                    if (_verified) {
                        emit messageReceived(_buffer + 2);
                    }
                    break;
                case SERIAL_NAME_HEADER:
                    if (!_verified) {
                        if (strcmp(_buffer + 2, _name) == 0) {
                            LOG_I(_serial->portName() + " is the correct serial port!!!!");
                            _verified = true;
                            START_TIMER(_heartbeatTimerId, SERIAL_HEARTBEAT_INTERVAL);
                            _state = SerialChannel2::ConnectedState;
                            _state = ConnectedState;
                            emit stateChanged(_state);
                        }
                        else {
                            LOG_I("Got incorrect name " + QString::fromLatin1(_buffer + 2));
                            return;
                        }
                    }
                    break;
                case SERIAL_LOG_DEBUG_HEADER:
                    LOG_D("MBED says: " + QString(_buffer + 2));
                    break;
                case SERIAL_LOG_INFO_HEADER:
                    LOG_I("MBED says: " + QString(_buffer + 2));
                    break;
                case SERIAL_LOG_WARN_HEADER:
                    LOG_W("MBED says: " + QString(_buffer + 2));
                    break;
                case SERIAL_LOG_ERROR_HEADER:
                    LOG_E("MBED says: " + QString(_buffer + 2));
                    break;
                case SERIAL_HEARTBEAT_HEADER: break;
                default:
                    LOG_I("Connected serial port send invalid header");
                    return;
                }
                if (_verified) {
                    _lastReadTime = QDateTime::currentMSecsSinceEpoch();
                }
#endif
#ifdef TARGET_LPC1768
                case SERIAL_MESSAGE_HEADER:
                    _messageAvailable = true;
                    break;
                case SERIAL_HANDSHAKE_HEADER:
                    sendMessage(_name, SERIAL_NAME_HEADER);
                    break;
                case SERIAL_HEARTBEAT_HEADER: break;
                default: return;
                }
                _loopsWithoutMessage = 0;
#endif
            }
        }

/************************************************************
 ************************************************************
 ************************************************************
 ************************************************************/
#ifdef QT_CORE_LIB

    public:
        ~SerialChannel2() {
            delete _serial;
        }

        enum State {
            ConnectingState, ConnectedState
        };

        SerialChannel2(const char *name, QObject *parent, Logger *log): QObject(parent) {
            _log = log;
            _name = name;
            LOG_TAG = (QString)_name + "(Serial)";
            _messageAvailable = false;
            _serialHandshakeIndex = 0;
            _serial = new QSerialPort(this);
            _state = SerialChannel2::ConnectingState;
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
                START_TIMER(_resetConnectionTimerId, 100);
            }
        }

    signals:
        void messageReceived(const char* message);
        void stateChanged(SerialChannel2::State state);

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
                        sendMessage("\0", SERIAL_HANDSHAKE_HEADER);
                        KILL_TIMER(_handshakeTimerId);
                        START_TIMER(_watchdogTimerId, SERIAL_IDLE_CONNECTION_TIMEOUT);
                        return;
                    }
                }
            }
            else if (e->timerId() == _heartbeatTimerId) {
                sendMessage("\0", SERIAL_HEARTBEAT_HEADER);
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
        int _loopsWithoutMessage;
        int _loopHeartbeatCounter;
        int _interval;
        DigitalOut *_led;
        enum LogLevel {
            DebugLevel, InfoLevel, WarnLevel, ErrorLevel
        };

    public:
        ~SerialChannel2() {
            delete _serial;
            delete _led;
        }

        SerialChannel2(const char *name, PinName tx, PinName rx, int ms_interval) {
            _name = name;
            _interval = ms_interval;
            _loopsWithoutMessage = 0;
            _loopHeartbeatCounter = 0;
            _messageAvailable = false;
            _led = new DigitalOut(LED4);
            _serial = new Serial(tx, rx);
            _serial->baud(115200);
            _serial->format(8, SerialBase::None, 1);
        }

        const char* getAvailableMessage() {
            if (_messageAvailable) {
                _messageAvailable = false;
                return &_buffer[2];
            }
            return NULL;
        }

        bool checkMessages() {
            _loopHeartbeatCounter++;
            _loopsWithoutMessage++;
            readSerial();
            if (_loopHeartbeatCounter * _interval > SERIAL_HEARTBEAT_INTERVAL) {
                sendMessage("\0", SERIAL_HEARTBEAT_HEADER);
                _loopHeartbeatCounter = 0;
            }
            if (_loopsWithoutMessage * _interval > SERIAL_IDLE_CONNECTION_TIMEOUT) {
                //We have gone too long without receiving a message, reset and hope
                //that fixes things
                *_led = 1;
                wait(1);
                mbed_reset();
            }
            return _messageAvailable;
        }
#endif
    };

}

#endif // SERIALCHANNEL_H
