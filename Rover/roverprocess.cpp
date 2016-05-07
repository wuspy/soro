#include "roverprocess.h"

#define LOG_TAG "Rover"

namespace Soro {
namespace Rover {

RoverProcess::RoverProcess(QObject *parent) : QObject(parent) {
    _log = new Logger(this);
    _log->setLogfile(QCoreApplication::applicationDirPath() + "/rover_" + QDateTime::currentDateTime().toString("M-dd_h:mm:AP") + ".log");
    _log->RouteToQtLogger = true;
    _log->MaxQtLoggerLevel = LOG_LEVEL_DEBUG;
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("Starting up...");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    //Must initialize from the event loop
    START_TIMER(_initTimerId, 1);
}

void RoverProcess::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _initTimerId) {
        KILL_TIMER(_initTimerId); //single shot

        LOG_I("**************Loading configuration from soro.ini****************");
        QString err = QString::null;
        if (!_soroIniConfig.load(&err)) {
            LOG_E(err);
            exit(1); return;
        }
        _soroIniConfig.applyLogLevel(_log);
        LOG_I("Configuration has been loaded successfully");

        if (_soroIniConfig.ServerSide == SoroIniLoader::RoverEndPoint) {
            LOG_I("***********Initializing core network system as server**************");
            //we are the server
            _armChannel = new Channel(this, _soroIniConfig.ArmChannelPort, CHANNEL_NAME_ARM,
                                      Channel::UdpProtocol, QHostAddress::Any, _log);
            _driveChannel = new Channel(this, _soroIniConfig.DriveChannelPort, CHANNEL_NAME_DRIVE,
                                      Channel::UdpProtocol, QHostAddress::Any, _log);
            _gimbalChannel = new Channel(this, _soroIniConfig.GimbalChannelPort, CHANNEL_NAME_GIMBAL,
                                      Channel::UdpProtocol, QHostAddress::Any, _log);
            _sharedChannel = new Channel(this, _soroIniConfig.SharedChannelPort, CHANNEL_NAME_SHARED,
                                      Channel::TcpProtocol, QHostAddress::Any, _log);
        }
        else {
            LOG_I("***********Initializing core network system as client**************");
            //mission control is the server
            _armChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.ArmChannelPort), CHANNEL_NAME_ARM,
                                      Channel::UdpProtocol, QHostAddress::Any, _log);
            _driveChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.DriveChannelPort), CHANNEL_NAME_DRIVE,
                                      Channel::UdpProtocol, QHostAddress::Any, _log);
            _gimbalChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.GimbalChannelPort), CHANNEL_NAME_GIMBAL,
                                      Channel::UdpProtocol, QHostAddress::Any, _log);
            _sharedChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.SharedChannelPort), CHANNEL_NAME_SHARED,
                                      Channel::TcpProtocol, QHostAddress::Any, _log);
        }

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

        _armChannel->open();
        _driveChannel->open();
        _gimbalChannel->open();
        _sharedChannel->open();

        LOG_I("*****************Initializing MBED systems*******************");

        // create mbed channels
        _armControllerMbed = new MbedChannel(SocketAddress(QHostAddress::Any, _soroIniConfig.ArmMbedPort), MBED_ID_ARM, this, _log);
        _driveGimbalControllerMbed = new MbedChannel(SocketAddress(QHostAddress::Any, _soroIniConfig.DriveMbedPort), MBED_ID_DRIVE_CAMERA, this, _log);

        // observers for mbed connectivity changes
        connect(_armControllerMbed, SIGNAL(stateChanged(MbedChannel*,MbedChannel::State)),
                this, SLOT(mbedChannelStateChanged(MbedChannel*,MbedChannel::State)));
        connect(_driveGimbalControllerMbed, SIGNAL(stateChanged(MbedChannel*,MbedChannel::State)),
                this, SLOT(mbedChannelStateChanged(MbedChannel*,MbedChannel::State)));

        // observers for network channels message received
        connect(_armChannel, SIGNAL(messageReceived(Channel*, const char*, Channel::MessageSize)),
                 this, SLOT(armChannelMessageReceived(Channel*, const char*, Channel::MessageSize)));
        connect(_driveChannel, SIGNAL(messageReceived(Channel*, const char*, Channel::MessageSize)),
                 this, SLOT(driveChannelMessageReceived(Channel*, const char*, Channel::MessageSize)));
        connect(_gimbalChannel, SIGNAL(messageReceived(Channel*, const char*, Channel::MessageSize)),
                 this, SLOT(gimbalChannelMessageReceived(Channel*, const char*, Channel::MessageSize)));
        connect(_sharedChannel, SIGNAL(messageReceived(Channel*, const char*, Channel::MessageSize)),
                 this, SLOT(sharedChannelMessageReceived(Channel*, const char*, Channel::MessageSize)));

        // observers for network channel connectivity changes
        connect(_sharedChannel, SIGNAL(stateChanged(Channel*,Channel::State)),
                this, SLOT(sharedChannelStateChanged(Channel*,Channel::State)));

        LOG_I("*****************Initializing GPS system*******************");

        _gpsServer = new GpsServer(this, SocketAddress(QHostAddress::Any, 5499), _log);

        LOG_I("*****************Initializing Video system*******************");

        LOG_I("Searching for flycapture cameras");
        FlycapEnumerator flycapEnum;
        int flycapCount = flycapEnum.loadCameras();
        LOG_I("Number of flycap cameras detected: " + QString::number(flycapCount));

        foreach (FlyCapture2::PGRGuid guid, flycapEnum.listByGuid()) {
            LOG_I("Found flycapture camera *-" + QString::number(guid.value[3]));
            // create associated video server
            VideoServer *server = new VideoServer("Camera " + QString::number(_videoServers.size() + 1),
                                                  SocketAddress(QHostAddress::Any, _soroIniConfig.FirstVideoPort + _videoServers.size()), _log, this);
            _videoServers.append(server);
            _flycapCameras.insert(_videoServers.size() - 1, guid);
            connect(server, SIGNAL(stateChanged(VideoServer*, VideoServer::State)), this, SLOT(videoServerStateChanged(VideoServer*, VideoServer::State)));
            connect(server, SIGNAL(error(VideoServer*,QString)), this, SLOT(videoServerError(VideoServer*,QString)));
        }

        LOG_I("Searching for UVD cameras (" + QString::number(_soroIniConfig.BlacklistedUvdCameras.size()) + " blacklisted)");
        UvdCameraEnumerator uvdEnum;
        int uvdCount = uvdEnum.loadCameras();\
        LOG_I("Number of UVD\'s/webcams detected: " + QString::number(uvdCount));

        foreach (QString videoDevice, uvdEnum.listByDeviceName()) {
            bool blacklisted = false;
            foreach (QString blacklistedDevice, _soroIniConfig.BlacklistedUvdCameras) {
                if (videoDevice.mid(videoDevice.size() - 1).compare(blacklistedDevice.mid(blacklistedDevice.size() - 1)) == 0) {
                    LOG_I("Found UVD device " + videoDevice + ", however it is blacklisted");
                    blacklisted = true;
                    break;
                }
            }
            if (blacklisted) continue;
            LOG_I("Found UVD/Webcam device at " + videoDevice);
            // create associated video server
            VideoServer *server = new VideoServer("Camera " + QString::number(_videoServers.size() + 1),
                                                  SocketAddress(QHostAddress::Any, _soroIniConfig.FirstVideoPort + _videoServers.size()), _log, this);
            _videoServers.append(server);
            _uvdCameras.insert(_videoServers.size() - 1, videoDevice);
            connect(server, SIGNAL(stateChanged(VideoServer*, VideoServer::State)), this, SLOT(videoServerStateChanged(VideoServer*, VideoServer::State)));
            connect(server, SIGNAL(error(VideoServer*,QString)), this, SLOT(videoServerError(VideoServer*,QString)));
        }

        LOG_I("**************Streaming first 3 cameras*****************");

        for (int cameraID = 0; cameraID < qMin(3, _videoServers.size()); cameraID++) {
            if (_flycapCameras.contains(cameraID)) {
                _videoServers[cameraID]->start(_flycapCameras[cameraID], x264());
            }
            else if (_uvdCameras.contains(cameraID)) {
                _videoServers[cameraID]->start(_uvdCameras[cameraID], x264());
            }
        }

        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("Initialization complete");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
    }
}

// observers for video servers

void RoverProcess::sharedChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    if (state == Channel::ConnectedState) {
        // send all status information since we just connected
        sendSystemStatusMessage();
    }
}

void RoverProcess::mbedChannelStateChanged(MbedChannel *channel, MbedChannel::State state) {
    Q_UNUSED(channel); Q_UNUSED(state);
    sendSystemStatusMessage();
}

void RoverProcess::sendSystemStatusMessage() {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    SharedMessageType messageType = SharedMessage_RoverStatusUpdate;
    bool armState = _armControllerMbed->getState() == MbedChannel::ConnectedState;
    bool driveGimbalState = _driveGimbalControllerMbed->getState() == MbedChannel::ConnectedState;

    stream << reinterpret_cast<quint32&>(messageType);
    stream << armState;
    stream << driveGimbalState;

    _sharedChannel->sendMessage(message.constData(), message.size());
}

// observers for network channels message received

void RoverProcess::armChannelMessageReceived(Channel * channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    switch (message[0]) {
    case ArmMessage::Header_Gamepad:
    case ArmMessage::Header_Master:
        _armControllerMbed->sendMessage(message, (int)size);
        break;
    default:
        LOG_E("Received invalid message from mission control on arm control channel");
        break;
    }
}

void RoverProcess::driveChannelMessageReceived(Channel * channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    if (message[0] == DriveMessage::Header) {
        _driveGimbalControllerMbed->sendMessage(message, (int)size);
    }
    else {
        LOG_E("Received invalid message from mission control on drive control channel");
    }
}

void RoverProcess::gimbalChannelMessageReceived(Channel * channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    if (message[0] == GimbalMessage::Header) {
        _driveGimbalControllerMbed->sendMessage(message, (int)size);
    }
    else {
        LOG_E("Received invalid message from mission control on gimbal control channel");
    }
}

void RoverProcess::sharedChannelMessageReceived(Channel * channel, const char *message, Channel::MessageSize size) {
    Q_UNUSED(channel);
    QByteArray byteArray = QByteArray::fromRawData(message, size);
    QDataStream stream(byteArray);
    SharedMessageType messageType;

    stream >> reinterpret_cast<quint32&>(messageType);
    switch (messageType) {
    case SharedMessage_RequestActivateCamera: {
        qint32 camera;
        StreamFormat format;
        stream >> camera;
        stream >> format;
        VideoServer *server = _videoServers[camera];

        LOG_I("Camera " + server->getCameraName() + " is about to be streamed");
        if (_flycapCameras.contains(camera)) {
            //this camera is flycap, we must set the framerate on it manually
            server->start(_flycapCameras[camera], format);
        }
        else {
            server->start(_uvdCameras[camera], format);
        }
    }
        break;
    case SharedMessage_RequestDeactivateCamera: {
        qint32 camera;
        stream >> camera;
        VideoServer *server = _videoServers[camera];
        if (server == NULL) {
            LOG_E("Request to deactivate a null video server");
            return;
        }
        if (server->getState() != VideoServer::IdleState) {
            LOG_I("Camera " + server->getCameraName() + " is about to be shut down");
            server->stop();
        }
    }
    case SharedMessage_MissionControlConnected: {
        // resend our current staus for the new mission control
        sendSystemStatusMessage();
    }
        break;
    default:
        break;
    }
}

RoverProcess::~RoverProcess() {
    if (_log != NULL) delete _log;
    if (_armChannel != NULL) delete _armChannel;
    if (_driveChannel != NULL) delete _driveChannel;
    if (_gimbalChannel != NULL) delete _gimbalChannel;
    if (_sharedChannel != NULL) delete _sharedChannel;
    if (_armControllerMbed != NULL) delete _armControllerMbed;
    if (_driveGimbalControllerMbed != NULL) delete _driveGimbalControllerMbed;
    if (_gpsServer != NULL) delete _gpsServer;
}

} // namespace Rover
} // namespace Soro
