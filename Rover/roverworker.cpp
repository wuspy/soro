#include "roverworker.h"

#define LOG_TAG "Rover"

using namespace Soro::Rover;

RoverWorker::RoverWorker(QObject *parent) : QObject(parent) {
    QString appPath = QCoreApplication::applicationDirPath();

    _log = new Logger(this);
    _log->setLogfile(appPath + "/rover_" + QDateTime::currentDateTime().toString("M-dd_h:mm:AP") + ".log");
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
        //_armControllerSerial = new SerialChannel3(ARM_SERIAL_CHANNEL_NAME, this, _log);
        _driveControllerSerial = new SerialChannel3(DRIVE_SERIAL_CHANNEL_NAME, this, _log);
        //_gimbalControllerSerial = new SerialChannel3(GIMBAL_SERIAL_CHANNEL_NAME, this, _log);

        //_armControllerSerial->addCompetitor(_driveControllerSerial);
        //_armControllerSerial->addCompetitor(_gimbalControllerSerial);
        //_driveControllerSerial->addCompetitor(_gimbalControllerSerial);

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

        _armChannel->open();
        _driveChannel->open();
        _gimbalChannel->open();
        _sharedChannel->open();

        _gpsServer = new GpsServer(this, SocketAddress(QHostAddress::Any, 5499), _log);

        LOG_I("Waiting for connections...");
    }
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
        _armControllerSerial->sendMessage(message.constData(), ArmMessage::size(message.constData()));
    }
}

void RoverWorker::driveChannelMessageReceived(const QByteArray &message) {
    if (DriveMessage::hasValidHeader(message.constData())) {
        _driveControllerSerial->sendMessage(message.constData(), DriveMessage::size(message.constData()));
    }
}

void RoverWorker::gimbalChannelMessageReceived(const QByteArray &message) {
    if (GimbalMessage::hasValidHeader(message.constData())) {
        _gimbalControllerSerial->sendMessage(message.constData(), GimbalMessage::size(message.constData()));
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
    if (_gpsServer != NULL) delete _gpsServer;
}
