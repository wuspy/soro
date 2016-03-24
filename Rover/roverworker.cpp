#include "roverworker.h"

#define LOG_TAG "Rover"

using namespace Soro::Rover;

RoverWorker::RoverWorker(QObject *parent) : QObject(parent) {
    QString appPath = QCoreApplication::applicationDirPath();

    _log = new Logger(this);
    _log->setLogfile(appPath + "/rover.log");
    _log->RouteToQtLogger = true;
    _log->i(LOG_TAG, "-------------------------------------------------------");
    _log->i(LOG_TAG, "-------------------------------------------------------");
    _log->i(LOG_TAG, "-------------------------------------------------------");
    _log->i(LOG_TAG, "Starting up...");

    //Must initialize from the event loop
    START_TIMER(_initTimerId, 1);
}

void RoverWorker::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _initTimerId) {
        KILL_TIMER(_initTimerId); //single shot

        SoroIniConfig config;
        QString err = QString::null;
        if (!config.parse(&err)) {
            _log->e(LOG_TAG, err);
            exit(1); return;
        }
        _armVideoPort = config.armVideoPort;
        _driveVideoPort = config.driveVideoPort;
        _gimbalVideoPort = config.gimbalVideoPort;
        _log->i(LOG_TAG, "Configuration has been loaded successfully");

        //create network channels
        _armChannel = new Channel(this, SocketAddress(config.serverAddress, config.armChannelPort), CHANNEL_NAME_ARM,
                                  Channel::UdpProtocol, Channel::ClientEndPoint, QHostAddress::Any, _log);
        _driveChannel = new Channel(this, SocketAddress(config.serverAddress, config.driveChannelPort), CHANNEL_NAME_DRIVE,
                                  Channel::UdpProtocol, Channel::ClientEndPoint, QHostAddress::Any, _log);
        _gimbalChannel = new Channel(this, SocketAddress(config.serverAddress, config.gimbalChannelPort), CHANNEL_NAME_GIMBAL,
                                  Channel::UdpProtocol, Channel::ClientEndPoint, QHostAddress::Any, _log);
        _sharedChannel = new Channel(this, SocketAddress(config.serverAddress, config.sharedChannelPort), CHANNEL_NAME_SHARED,
                                  Channel::TcpProtocol, Channel::ClientEndPoint, QHostAddress::Any, _log);

        if (_armChannel->getState() == Channel::ErrorState) {
            _log->e(LOG_TAG, "The arm channel experienced a fatal error during initialization");
            exit(1); return;
        }
        if (_driveChannel->getState() == Channel::ErrorState) {
            _log->e(LOG_TAG, "The drive channel experienced a fatal error during initialization");
            exit(1); return;
        }
        if (_gimbalChannel->getState() == Channel::ErrorState) {
            _log->e(LOG_TAG, "The gimbal channel experienced a fatal error during initialization");
            exit(1); return;
        }
        if (_sharedChannel->getState() == Channel::ErrorState) {
            _log->e(LOG_TAG, "The shared channel experienced a fatal error during initialization");
            exit(1); return;
        }
        _log->i(LOG_TAG, "All network channels initialized successfully");

        //create serial (mbed) channels
        _armControllerSerial = new SerialChannel("ARM", this, _log);
        _driveControllerSerial = new SerialChannel("DRIVE", this, _log);
        _gimbalControllerSerial = new SerialChannel("GIMBAL", this, _log);
        _log->i(LOG_TAG, "All serial channels initialized successfully");

        //observers for network channels message received
        connect(_armChannel, SIGNAL(messageReceived(const QByteArray)),
                 this, SLOT(armChannelMessageReceived(const QByteArray)));
        connect(_driveChannel, SIGNAL(messageReceived(const QByteArray)),
                 this, SLOT(driveChannelMessageReceived(const QByteArray)));
        connect(_gimbalChannel, SIGNAL(messageReceived(const QByteArray)),
                 this, SLOT(gimbalChannelMessageReceived(const QByteArray)));
        connect(_sharedChannel, SIGNAL(messageReceived(const QByteArray)),
                 this, SLOT(sharedChannelMessageReceived(const QByteArray)));
        //observers for network connectivity changes
        connect(_armChannel, SIGNAL(messageReceived(const QByteArray)),
                 this, SLOT(armChannelMessageReceived(const QByteArray)));
        connect(_driveChannel, SIGNAL(messageReceived(const QByteArray)),
                 this, SLOT(driveChannelMessageReceived(const QByteArray)));
        connect(_gimbalChannel, SIGNAL(messageReceived(const QByteArray)),
                 this, SLOT(gimbalChannelMessageReceived(const QByteArray)));
        connect(_sharedChannel, SIGNAL(messageReceived(const QByteArray)),
                 this, SLOT(sharedChannelMessageReceived(const QByteArray)));
        //observers for serial (mbed) messages received
        connect(_armControllerSerial, SIGNAL(messageReceived(const char*,int)),
                this, SLOT(armControllerMessageReceived(const char*,int)));
        connect(_driveControllerSerial, SIGNAL(messageReceived(const char*,int)),
                this, SLOT(driveControllerMessageReceived(const char*,int)));
        connect(_gimbalControllerSerial, SIGNAL(messageReceived(const char*,int)),
                this, SLOT(gimbalControllerMessageReceived(const char*,int)));
        //observers for serial (mbed) connectivity state changes
        connect(_armControllerSerial, SIGNAL(stateChanged(SerialChannel::State)),
                this, SLOT(armControllerChannelStateChanged(SerialChannel::State)));
        connect(_driveControllerSerial, SIGNAL(stateChanged(SerialChannel::State)),
                this, SLOT(driveControllerChannelStateChanged(SerialChannel::State)));
        connect(_gimbalControllerSerial, SIGNAL(stateChanged(SerialChannel::State)),
                this, SLOT(gimbalControllerChannelStateChanged(SerialChannel::State)));

        _armChannel->open();
        _driveChannel->open();
        _gimbalChannel->open();
        _sharedChannel->open();

        _log->i(LOG_TAG, "Waiting for connections...");
    }
}

//observers for serial (mbed) connectivity state changes

void RoverWorker::armControllerChannelStateChanged(SerialChannel::State state) {
    switch (state) {
    case SerialChannel::ConnectedState:
        _log->i(LOG_TAG, "Arm controller (mbed) is trying to connect...");
        break;
    case SerialChannel::ConnectingState:
        _log->i(LOG_TAG, "Arm controller (mbed) is connected");
        break;
    }
}

void RoverWorker::driveControllerChannelStateChanged(SerialChannel::State state) {
    switch (state) {
    case SerialChannel::ConnectedState:
        _log->i(LOG_TAG, "Drive controller (mbed) is trying to connect...");
        break;
    case SerialChannel::ConnectingState:
        _log->i(LOG_TAG, "Drive controller (mbed) is connected");
        break;
    }
}

void RoverWorker::gimbalControllerChannelStateChanged(SerialChannel::State state) {
    switch (state) {
    case SerialChannel::ConnectedState:
        _log->i(LOG_TAG, "Gimbal controller (mbed) is trying to connect...");
        break;
    case SerialChannel::ConnectingState:
        _log->i(LOG_TAG, "Gimbal controller (mbed) is connected");
        break;
    }
}

//observers for serial (mbed) messages received

void RoverWorker::armControllerMessageReceived(const char *message, int size) {
    _log->i("Arm Controller (mbed)", QString::fromLatin1(message, size));
}

void RoverWorker::driveControllerMessageReceived(const char *message, int size) {
    _log->i("Drive Controller (mbed)", QString::fromLatin1(message, size));
}

void RoverWorker::gimbalControllerMessageReceived(const char *message, int size) {
    _log->i("Gimbal Controller (mbed)", QString::fromLatin1(message, size));
}

//observers for network channels message received

void RoverWorker::armChannelStateChanged(Channel::State state) {
    //TODO
}

void RoverWorker::driveChannelStateChanged(Channel::State state) {
    //TODO
}

void RoverWorker::gimbalChannelStateChanged(Channel::State state) {
    //TODO
}

void RoverWorker::sharedChannelStateChanged(Channel::State state) {
    //TODO
}

//observers for network channels message received

void RoverWorker::armChannelMessageReceived(const QByteArray &message) {
    _armControllerSerial->sendMessage(message.constData(), message.size());
}

void RoverWorker::driveChannelMessageReceived(const QByteArray &message) {
    _driveControllerSerial->sendMessage(message.constData(), message.size());
}

void RoverWorker::gimbalChannelMessageReceived(const QByteArray &message) {
    _gimbalControllerSerial->sendMessage(message.constData(), message.size());
}

void RoverWorker::sharedChannelMessageReceived(const QByteArray &message) {
    //TODO
}

RoverWorker::~RoverWorker() {
    if (_log != NULL) delete _log;
    if (_armChannel != NULL) delete _armChannel;
    if (_driveChannel != NULL) delete _driveChannel;
    if (_gimbalChannel != NULL) delete _gimbalChannel;
    if (_sharedChannel != NULL) delete _sharedChannel;
    if (_armControllerSerial != NULL) delete _armControllerSerial;
    if (_armControllerSerial != NULL) delete _driveControllerSerial;
    if (_armControllerSerial != NULL) delete _gimbalControllerSerial;
}
