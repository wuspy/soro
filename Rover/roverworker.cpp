#include "roverworker.h"

#define LOG_TAG "Rover"

using namespace Soro::Rover;

RoverWorker::RoverWorker(QObject *parent) : QObject(parent) {
    QString appPath = QCoreApplication::applicationDirPath();

    _log = new Logger(this);
    _log->setLogfile(appPath + "/rover.log");
    _log->RouteToQtLogger = true;
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("Starting up...");

    //Must initialize from the event loop
    START_TIMER(_initTimerId, 1);
}

void RoverWorker::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _initTimerId) {
        KILL_TIMER(_initTimerId); //single shot

        QString err = QString::null;
        if (!_soroIniConfig.load(&err)) {
            LOG_E(err);
            exit(1); return;
        }
        _soroIniConfig.applyLogLevel(_log);
        LOG_I("Configuration has been loaded successfully");
        Channel::EndPoint commEndPoint =
                _soroIniConfig.ServerSide == SoroIniConfig::RoverEndPoint ?
                    Channel::ServerEndPoint : Channel::ClientEndPoint;

        //create network channels
        _armChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.ArmChannelPort), CHANNEL_NAME_ARM,
                                  Channel::UdpProtocol, commEndPoint, QHostAddress::Any, _log);
        _driveChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.DriveChannelPort), CHANNEL_NAME_DRIVE,
                                  Channel::UdpProtocol, commEndPoint, QHostAddress::Any, _log);
        _gimbalChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.GimbalChannelPort), CHANNEL_NAME_GIMBAL,
                                  Channel::UdpProtocol, commEndPoint, QHostAddress::Any, _log);
        _sharedChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.SharedChannelPort), CHANNEL_NAME_SHARED,
                                  Channel::TcpProtocol, commEndPoint, QHostAddress::Any, _log);

        if (_armChannel->getState() == Channel::ErrorState) {
            LOG_E("The arm channel experienced a fatal error during initialization");
            exit(1); return;
        }
        if (_driveChannel->getState() == Channel::ErrorState) {
            LOG_E("The drive channel experienced a fatal error during initialization");
            exit(1); return;
        }
        if (_gimbalChannel->getState() == Channel::ErrorState) {
            LOG_E("The gimbal channel experienced a fatal error during initialization");
            exit(1); return;
        }
        if (_sharedChannel->getState() == Channel::ErrorState) {
            LOG_E("The shared channel experienced a fatal error during initialization");
            exit(1); return;
        }
        LOG_I("All network channels initialized successfully");

        //create serial (mbed) channels
        //_armControllerSerial = new SerialChannel2(ARM_SERIAL_CHANNEL_NAME, this, _log);
        _driveControllerSerial = new SerialChannel2(DRIVE_SERIAL_CHANNEL_NAME, this, _log);
        //_gimbalControllerSerial = new SerialChannel2(GIMBAL_SERIAL_CHANNEL_NAME, this, _log);
        LOG_I("All serial channels initialized successfully");

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
        //connect(_armControllerSerial, SIGNAL(stateChanged(SerialChannel::State)),
        //        this, SLOT(armControllerChannelStateChanged(SerialChannel::State)));
        connect(_driveControllerSerial, SIGNAL(stateChanged(SerialChannel::State)),
                this, SLOT(driveControllerChannelStateChanged(SerialChannel::State)));
        //connect(_gimbalControllerSerial, SIGNAL(stateChanged(SerialChannel::State)),
        //        this, SLOT(gimbalControllerChannelStateChanged(SerialChannel::State)));

        _armChannel->open();
        _driveChannel->open();
        _gimbalChannel->open();
        _sharedChannel->open();

        LOG_I("Waiting for connections...");
    }
}

//observers for serial (mbed) connectivity state changes

void RoverWorker::armControllerChannelStateChanged(SerialChannel::State state) {
    switch (state) {
    case SerialChannel::ConnectedState:
        LOG_I("Arm controller (mbed) is trying to connect...");
        break;
    case SerialChannel::ConnectingState:
        LOG_I("Arm controller (mbed) is connected");
        break;
    }
}

void RoverWorker::driveControllerChannelStateChanged(SerialChannel::State state) {
    switch (state) {
    case SerialChannel::ConnectedState:
        LOG_I("Drive controller (mbed) is trying to connect...");
        break;
    case SerialChannel::ConnectingState:
        LOG_I("Drive controller (mbed) is connected");
        break;
    }
}

void RoverWorker::gimbalControllerChannelStateChanged(SerialChannel::State state) {
    switch (state) {
    case SerialChannel::ConnectedState:
        LOG_I("Gimbal controller (mbed) is trying to connect...");
        break;
    case SerialChannel::ConnectingState:
        LOG_I("Gimbal controller (mbed) is connected");
        break;
    }
}

//observers for serial (mbed) messages received

void RoverWorker::armControllerMessageReceived(const char *message, int size) {
    if (_log != NULL) _log->i("Arm Controller (mbed)", QString::fromLatin1(message, size));
}

void RoverWorker::driveControllerMessageReceived(const char *message, int size) {
    if (_log != NULL) _log->i("Drive Controller (mbed)", QString::fromLatin1(message, size));
}

void RoverWorker::gimbalControllerMessageReceived(const char *message, int size) {
    if (_log != NULL) _log->i("Gimbal Controller (mbed)", QString::fromLatin1(message, size));
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
    if ((state == Channel::ConnectedState) && (_soroIniConfig.VideoServerAddress == QHostAddress::Null)) {
        //If the rover acts as a server, the only way we can deduce where to send the video stream is by waiting
        //for mission control to connect
        _soroIniConfig.VideoServerAddress = _sharedChannel->getPeerAddress().address;
    }
    //TODO
}

//observers for network channels message received

void RoverWorker::armChannelMessageReceived(const QByteArray &message) {
    if (ArmMessage::messageType(message) != ArmMessage::INVALID) {
        _armControllerSerial->sendMessage(message.constData());
    }
}

void RoverWorker::driveChannelMessageReceived(const QByteArray &message) {
    if (DriveMessage::hasValidHeader(message.constData())) {
        _driveControllerSerial->sendMessage(message.constData());
    }
}

void RoverWorker::gimbalChannelMessageReceived(const QByteArray &message) {
    if (GimbalMessage::hasValidHeader(message.constData())) {
        _gimbalControllerSerial->sendMessage(message.constData());
    }
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
