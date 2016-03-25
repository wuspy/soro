#include "serialinterop.h"

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

using namespace Soro;

/* Writes a message through the serial port
 */
void SerialChannel::sendMessage(const char* message, int size) {
    if (size < 250) {
        SERIAL_PUTC((*_serial), SERIAL_MESSAGE_HEADER);
        SERIAL_PUTC((*_serial), (unsigned char)size);
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
            LOG_I("Got invalid serial header " + QString::number((unsigned char)head));
            #endif
            return read;
        }
    case __WAITING_FOR_SIZE:
        char size;
        if (SERIAL_READABLE((*_serial))) {
            SERIAL_GETC((*_serial), size);
            read++;
            _readIndex = 0;
            _size = (int)(unsigned char)size;
        }
        else break;
    default:
        for (; _readIndex < _size; _readIndex++) {
            if (SERIAL_READABLE((*_serial))) {
                SERIAL_GETC((*_serial), _buffer[_readIndex]);
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
                _messageAvailable = true;
            }
            _readIndex = __WAITING_FOR_HEADER;
        }
        break;
    }
    return read;
}

SerialChannel::~SerialChannel() {
    delete _serial;
#ifdef TARGET_LPC1768
    delete _led;
#endif
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
    connect(_serial, SIGNAL(error(QSerialPort::SerialPortError)),
            this, SLOT(serialError(QSerialPort::SerialPortError)));
    //begin looking for the correct serial port
    resetConnection();
}

/* Slot that receives the QSerialPort's readyRead() signal
 */
void SerialChannel::serialReadyRead() {
    int chars = read();
    if (chars > 0) {
        if (_verified) _lastReadTime = QDateTime::currentMSecsSinceEpoch();
        if (_messageAvailable) {
            if (!_verified) {
                //in case it is a handshake response
                size_t size = strnlen(_name, 100);
                if (size == 100) {
                    LOG_I("Connected serial port send garbage data");
                    resetConnection();
                    return;
                }
                _verified = memcmp(_buffer, _name, size);
                if (_verified) {
                    LOG_I("Serial port " + _serial->portName() + " has verified its identity, communication can now proceed");
                    _lastReadTime = QDateTime::currentMSecsSinceEpoch();
                    START_TIMER(_heartbeatTimerId, HEARTBEAT_INTERVAL);
                    _state = SerialChannel::ConnectedState;
                    emit stateChanged(_state);
                }
                else {
                    LOG_I("Connected to incorrect serial port (got name " + (QString)_name + ")");
                    resetConnection();
                }
            }
            else emit messageReceived(_buffer, _size);
        }
    }
}

void SerialChannel::serialError(QSerialPort::SerialPortError err) {
    if (err != QSerialPort::NoError) {
        LOG_E("Serial " + _serial->portName() + " experienced an error: " + _serial->errorString());
        resetConnection();
    }
}

void SerialChannel::timerEvent(QTimerEvent *e) {
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
                _serial->putChar(SERIAL_MESSAGE_HANDSHAKE);
                _verified = false;
                KILL_TIMER(_handshakeTimerId);
                START_TIMER(_watchdogTimerId, IDLE_CONNECTION_TIMEOUT);
                return;
            }
        }
    }
    else if (e->timerId() == _heartbeatTimerId) {
        _serial->putChar(SERIAL_MESSAGE_HEARTBEAT);
    }
    else if (e->timerId() == _watchdogTimerId) {
        if (QDateTime::currentMSecsSinceEpoch() - _lastReadTime > IDLE_CONNECTION_TIMEOUT) {
            if (_verified) {
                LOG_I("Serial port " + _serial->portName() + " is not responding");
            }
            else {
                LOG_I("Serial port " + _serial->portName() + " did not verify in time, disconnecting");
            }
            resetConnection();
        }
    }
}

void SerialChannel::resetConnection() {
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

SerialChannel::State SerialChannel::getState() const {
    return _state;
}

#endif
/************************************************************
 ************************************************************
 ************************************************************
 ************************************************************/
#ifdef TARGET_LPC1768

SerialChannel::SerialChannel(const char *name, PinName tx, PinName rx, int ms_interval) {
    _name = name;
    _size = 0;
    _interval = ms_interval;
    _readIndex = -1;
    _loopsWithoutMessage = 0;
    _loopHeartbeatCounter = 0;
    _messageAvailable = false;
    _led = new DigitalOut(LED4);
    _serial = new Serial(tx, rx);
    _serial->baud(9600);
    _serial->format(8, SerialBase::None, 1);
}

void SerialChannel::process() {
    _loopHeartbeatCounter++;
    if (_loopHeartbeatCounter * _interval > HEARTBEAT_INTERVAL) {
        SERIAL_PUTC((*_serial), SERIAL_MESSAGE_HEARTBEAT);
        _loopHeartbeatCounter = 0;
    }
    int chars = read();
    _loopsWithoutMessage = chars == 0 ? _loopsWithoutMessage + 1 : 0;
    if (_loopsWithoutMessage * _interval > IDLE_CONNECTION_TIMEOUT) {
        //We have gone too long without receiving a message, reset and hope
        //that fixes things
        *_led = 1;
        wait(1);
        mbed_reset();
    }
}

bool SerialChannel::getAvailableMessage(char *outMessage, int& outSize) {
    if (_messageAvailable) {
        outMessage = _buffer;
        outSize = _size;
        return true;
    }
    return false;
}
#endif
