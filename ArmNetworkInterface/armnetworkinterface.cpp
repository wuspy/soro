#include "armnetworkinterface.h"

ArmNetworkInterface::ArmNetworkInterface(QObject *parent) : QObject(parent) {
    qDebug() << "Starting ArmNetworkInterface";

    QString appPath = QCoreApplication::applicationDirPath();

    _log = new Logger(this);
    _log->setLogfile(appPath + "/arm_log_client.txt");
    _log->RouteToQtLogger = true;
    _log->MaxLevel = LOG_LEVEL_WARN;

    _armChannel = new Channel(this, appPath + "/arm_channel_client.ini", _log);
    connect(_armChannel, SIGNAL(messageReceived(Channel*,QByteArray)),
             this, SLOT(channelMessageReceived(Channel*,QByteArray)));

    //Setup arm channel
    if (_armChannel->getState() == Channel::ErrorState) {
        qFatal("Configuration file does not exist!");
    }
    else {
        _armChannel->open();
        qDebug() << "Attempting to open channel...";
    }

    _serial = new QSerialPort("COM3", this);
    _serial->setParity(QSerialPort::NoParity);
    _serial->setStopBits(QSerialPort::OneStop);
    _serial->setDataBits(QSerialPort::Data8);
    connect(_serial, SIGNAL(readyRead()),
            this, SLOT(serialReadyRead()));
    if (_serial->open(QIODevice::ReadWrite)) {
        qDebug() << "mbed serial is opened";
    }
    else {
        qCritical() << "Could not open mbed serial";
    }
}

void ArmNetworkInterface::channelMessageReceived(Channel *channel, const QByteArray &message) {
    if (!message.isEmpty()) {
        //joystick implememtation
        signed char lx = message[0];
        signed char ly = message[1];
        signed char rx = message[2];
        signed char ry = message[3];
        unsigned char bx = message[4];
        unsigned char ba = message[5];
        unsigned char bb = message[6];
        unsigned char by = message[7];
        unsigned char lb = message[8];
        unsigned char rb = message[9];
        unsigned char lt = message[10];
        unsigned char rt = message[11];
        _serial->putChar((unsigned char)255); //start of block
        _serial->putChar(lx);
        _serial->putChar(ly);
        _serial->putChar(rx);
        _serial->putChar(ry);
        _serial->putChar(bx);
        _serial->putChar(ba);
        _serial->putChar(bb);
        _serial->putChar(by);
        _serial->putChar(lt);
        _serial->putChar(rt);
        _serial->putChar(lb);
        _serial->putChar(rb);

        //keyboard implementation
        /*char c = message[0];
        if (_serial->isOpen()) {
            _serial->putChar(c);
            qDebug() << "Sending char " << c << " to mbed";
        }*/
    }
}

ArmNetworkInterface::~ArmNetworkInterface() {
    delete _log;
    delete _armChannel;
}
