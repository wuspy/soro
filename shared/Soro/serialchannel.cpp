#include "serialchannel.h"

#ifdef QT_CORE_LIB
//  Log macros
#   define LOG_D(X) if (_log != NULL) _log->d(_name, X)
#   define LOG_I(X) if (_log != NULL) _log->i(_name, X)
#   define LOG_W(X) if (_log != NULL) _log->w(_name, X)
#   define LOG_E(X) if (_log != NULL) _log->e(_name, X)
#endif
#ifdef TARGET_LPC1768
extern "C" void mbed_reset();
#endif

#define __WAITING_FOR_HEADER -2
#define __WAITING_FOR_SIZE -1
#define __WAITING_FOR_FOOTER -3

#define IDLE_CONNECTION_TIMEOUT 1000
#define HEARTBEAT_INTERVAL 250
#define VERIFY_TIMEOUT 1000

using namespace Soro;

/* Writes a message through the serial port
 */
void SerialChannel::sendMessage(const char* message, int size) {
    if (size < 250) {
        SERIAL_PUTC((*_serial), SERIAL_MESSAGE_HEADER);
        SERIAL_PUTC((*_serial), (char)(unsigned char)size);
        for (int i = 0; i < size; i++) {
            SERIAL_PUTC((*_serial), message[i]);
        }
        SERIAL_PUTC((*_serial), SERIAL_MESSAGE_FOOTER);
    }
}

/* Reads the next message from the serial port and returns true if successful
 */
int SerialChannel::read() {
    _messageAvailable = false;
    if (!SERIAL_READABLE((*_serial))) return 0;
    int read = 0;
    //No, I did not forget to break these case labels
    switch (_readIndex) {
    case __WAITING_FOR_HEADER:
        char head;
        SERIAL_GETC((*_serial), head);
        read++;
        switch ((unsigned char)head) {
        case SERIAL_MESSAGE_HEADER:
            _readIndex = __WAITING_FOR_SIZE;
            break;
        case SERIAL_MESSAGE_HANDSHAKE:
            sendMessage(_name, strlen(_name));
            return read;
        case SERIAL_MESSAGE_HEARTBEAT:
            return read;
        default:
            #ifdef QT_CORE_LIB
            qWarning() << "Got invalid serial header " << QString::number((unsigned char)head);
            #endif
            return read;
        }
    case __WAITING_FOR_SIZE:
        char size;
        if (SERIAL_READABLE((*_serial))) {
            SERIAL_GETC((*_serial), size);
            read++;
            _size = (int)(unsigned char)size;
        }
        else break;
    default:
        for (int i = _readIndex; i < _size; _readIndex++) {
            if (SERIAL_READABLE((*_serial))) {
                SERIAL_GETC((*_serial), _buffer[i]);
                read++;
            }
            else return read;
        }
        _readIndex = __WAITING_FOR_FOOTER;
    case __WAITING_FOR_FOOTER:
        if (SERIAL_READABLE((*_serial))) {
            char foot;
            SERIAL_GETC((*_serial), foot);
            read++;
            if ((unsigned char)foot == SERIAL_MESSAGE_FOOTER) {
                _readIndex = __WAITING_FOR_HEADER;
                _messageAvailable = true;
            }
            else {
                //message footer was incorrect, discard it and wait for frame alignment
                _readIndex = __WAITING_FOR_HEADER;
            }
        }
    }
    return read;
}

SerialChannel::~SerialChannel() {
    delete _serial;
}

/************************************************************
 ************************************************************
 ************************************************************
 ************************************************************/
#ifdef QT_CORE_LIB

SerialChannel::SerialChannel(const char *name, QObject *parent, Logger *log)
    : QObject(parent) {
    _log = log;
    _name = name;
    _size = 0;
    _readIndex = -1;
    _messageAvailable = false;
    _serialHandshakeIndex = 0;
    _serial = new QSerialPort(this);
    configureMbedSerial(_serial);
    _state = SerialChannel::ConnectingState;
    connect(_serial, SIGNAL(readyRead()),
            this, SLOT(serialReadyRead()));
}

/* Slot that receives the QSerialPort's readyRead() signal
 */
void SerialChannel::serialReadyRead() {
    int chars = read();
    if (chars > 0) {
        KILL_TIMER(_watchdogTimerId);
        START_TIMER(_watchdogTimerId, IDLE_CONNECTION_TIMEOUT);
        if (_messageAvailable) {
            if (!_verified) {
                //in case it is a handshake response
                _verified = memcmp(_buffer, _name, (size_t)qMin((size_t)_size, strlen(_name)));
                LOG_I("Serial port " + _serial->portName() + " has verified its identity, communication can now proceed");
                START_TIMER(_heartbeatTimerId, HEARTBEAT_INTERVAL);
                START_TIMER(_watchdogTimerId, IDLE_CONNECTION_TIMEOUT);
                _state = SerialChannel::ConnectedState;
                emit stateChanged(_state);
            }
            else emit messageReceived(_buffer, _size);
        }
    }
}

void SerialChannel::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _handshakeTimerId) {
        if (_serialHandshakeIndex >= _serialPorts.size()) {
            _serialHandshakeIndex = 0;
            _serialPorts = QSerialPortInfo::availablePorts();
        }
        if (_serial->isOpen()) _serial->close();
        for (;_serialHandshakeIndex < _serialPorts.size(); _serialHandshakeIndex++) {
            _serial->setPort(_serialPorts[_serialHandshakeIndex]);
            LOG_D("Trying serial port " + _serial->portName());
            if (_serial->open(QIODevice::ReadWrite)) {
                LOG_I("Connected to serial port " + _serial->portName());
                SERIAL_PUTC((*_serial), SERIAL_MESSAGE_HANDSHAKE);
                _verified = false;
                KILL_TIMER(_handshakeTimerId);
                START_TIMER(_verifyTimerId, VERIFY_TIMEOUT);
                return;
            }
            else {
                LOG_D("Could not connect to " + _serial->portName() + ": " + _serial->errorString());
            }
        }
    }
    else if (e->timerId() == _verifyTimerId) {
        if (!_verified) {
            LOG_I("Serial port " + _serial->portName() + " did not verify in time, disconnecting");
            START_TIMER(_handshakeTimerId, 100);
        }
        KILL_TIMER(_verifyTimerId); //single shot
    }
    else if (e->timerId() == _heartbeatTimerId) {
        SERIAL_PUTC((*_serial), SERIAL_MESSAGE_HEARTBEAT);
    }
    else if (e->timerId() == _watchdogTimerId) {
        KILL_TIMER(_heartbeatTimerId);
        KILL_TIMER(_watchdogTimerId);
        KILL_TIMER(_verifyTimerId)
        START_TIMER(_handshakeTimerId, 100);
        _state = SerialChannel::ConnectingState;
        emit stateChanged(_state);
    }
}

SerialChannel::State SerialChannel::getState() const {
    return _state;
}

#endif
/************************************************************
 ************************************************************
 ************************************************************
 ************************************************************/
#ifdef PLATFORM_LPC1768

SerialChannel::SerialChannel(const char *name, PinName tx, PinName rx, int interval) {
    _name = name;
    _size = 0;
    _bytesReceived = 0;
    _interval = interval;
    _readIndex = -1;
    _messageAvailable = false;
    _serial = new Serial(tx, rx);
    _serial->baud(9600);
    _serial->format(8, SerialBase::None, 1);
}

bool SerialChannel::messageAvailable(char *outMessage, int& outSize) {
    _loopHeartbeatCounter++;
    if (_loopHeartbeatCounter * _interval > HEARTBEAT_INTERVAL) {
        __INTEROP_SERIAL_PUTC(_serial, SERIAL_MESSAGE_HEARTBEAT);
        _loopHeartbeatCounter = 0;
    }
    int chars = read();
    _loopsWithoutMessage = chars == 0 ? _loopsWithoutMessage + 1 : 0;
    if (_messageAvailable) {
        outMessage = _buffer;
        outSize = _size;
        return true;
    }
    if (_loopsWithoutMessage * _interval > IDLE_CONNECTION_TIMEOUT) {
        //We have gone too long without receiving a message, reset and hope
        //that fixes things
        led = 1;
        wait(1);
        mbed_reset();
    }
    return false;
}
#endif
