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

        //create mbed channels
        _armControllerMbed = new MbedChannel(SocketAddress(QHostAddress::Any, _soroIniConfig.ArmMbedPort), MBED_ID_ARM, this, _log);
        _driveControllerMbed = new MbedChannel(SocketAddress(QHostAddress::Any, _soroIniConfig.DriveMbedPort), MBED_ID_DRIVE, this, _log);
        _gimbalControllerMbed = new MbedChannel(SocketAddress(QHostAddress::Any, _soroIniConfig.GimbalMbedPort), MBED_ID_GIMBAL, this, _log);

        //observers for network channels message received
        connect(_armChannel, SIGNAL(messageReceived(const char*, Channel::MessageSize)),
                 this, SLOT(armChannelMessageReceived(const char*, Channel::MessageSize)));
        connect(_driveChannel, SIGNAL(messageReceived(const char*, Channel::MessageSize)),
                 this, SLOT(driveChannelMessageReceived(const char*, Channel::MessageSize)));
        connect(_gimbalChannel, SIGNAL(messageReceived(const char*, Channel::MessageSize)),
                 this, SLOT(gimbalChannelMessageReceived(const char*, Channel::MessageSize)));
        connect(_sharedChannel, SIGNAL(messageReceived(const char*, Channel::MessageSize)),
                 this, SLOT(sharedChannelMessageReceived(const char*, Channel::MessageSize)));

        LOG_I("*****************Initializing GPS system*******************");

        _gpsServer = new GpsServer(this, SocketAddress(QHostAddress::Any, 5499), _log);

        LOG_I("*****************Initializing Video system*******************");

        int cameraIndex = 0;

        LOG_I("Searching for flycapture cameras");
        FlycapEnumerator flycapEnum;
        int flycapCount = flycapEnum.loadCameras();
        LOG_I("Number of flycap cameras detected: " + QString::number(flycapCount));
        if (flycapCount < _soroIniConfig.FlyCapture2CameraCount) {
            LOG_E("The configuration files says there should be MORE flycapture cameras connected than this, something may be wrong!!!");
        }
        foreach (FlyCapture2::PGRGuid guid, flycapEnum.listByGuid()) {
            LOG_I("Found flycapture camera *-" + QString::number(guid.value[3]));
            if (cameraIndex == flycapCount) {
                LOG_E("The configuration file says there should be LESS flycapture cameras connected than this, something may be wrong!!!");
                break;
            }
            FlycapCamera *source = new FlycapCamera(guid, _log, this);
            source->setFramerate(15);
            // create associated video server
            VideoServer *server = new VideoServer(source->element(), "Camera " + QString::number(cameraIndex + 1) + " (Blackfly)",
                                                  SocketAddress(QHostAddress::Any, _soroIniConfig.FirstVideoPort + cameraIndex), _log, this);
            _flycapCameras.insert(cameraIndex, source);
            _videoServers.insert(cameraIndex, server);
            _videoFormats.insert(cameraIndex, StreamFormat());
            connect(server, SIGNAL(stateChanged(VideoServer::State)), this, SLOT(videoServerStateChanged(VideoServer::State)));
            cameraIndex++;
        }
        cameraIndex = _soroIniConfig.FlyCapture2CameraCount;
        LOG_I("Searching for UVD cameras (" + QString::number(_soroIniConfig.BlacklistedUvdCameras.size()) + " blacklisted)");
        UvdCameraEnumerator uvdEnum;
        int uvdCount = uvdEnum.loadCameras();\
        LOG_I("Number of UVD\'s/webcams detected: " + QString::number(uvdCount));
        if (uvdCount < _soroIniConfig.UVDCameraCount) {
            LOG_E("The configuration files says there should be MORE UVD\'s/webcams connected than this, something may be wrong!!!");
        }
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
            if (cameraIndex == _soroIniConfig.FlyCapture2CameraCount + _soroIniConfig.UVDCameraCount) {
                LOG_E("The configuration file says there should be LESS UVD\'s/webcams connected than this, something may be wrong!!!");
                break;
            }
            LOG_I("Found UVD/Webcam device at " + videoDevice);
            QGst::ElementPtr source = QGst::ElementFactory::make("v4l2src");
            source->setProperty("device", videoDevice);
            // create associated video server
            VideoServer *server = new VideoServer(source, "Camera " + QString::number(cameraIndex + 1) + " (Webcam)",
                                                  SocketAddress(QHostAddress::Any, _soroIniConfig.FirstVideoPort + cameraIndex), _log, this);
            _uvdCameras.insert(cameraIndex, source);
            _videoServers.insert(cameraIndex, server);
            _videoFormats.insert(cameraIndex, StreamFormat());
            connect(server, SIGNAL(stateChanged(VideoServer::State)), this, SLOT(videoServerStateChanged(VideoServer::State)));
            cameraIndex++;
        }

        LOG_I("Starting default video streams");

        if (_videoFormats.size() > 0) {
            _videoFormats[0] = streamFormat_Mjpeg_960x720_15FPS_Q50();
            if (_videoFormats.size() > 1) {
                _videoFormats[1] = streamFormat_Mjpeg_960x720_15FPS_Q50();
                if (_videoFormats.size() > 2) {
                    _videoFormats[2] = streamFormat_Mjpeg_960x720_15FPS_Q50();
                }
            }
        }
        syncVideoStreams();

        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("Initialization complete");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
        LOG_I("-------------------------------------------------------");
    }
}

void RoverProcess::syncVideoStreams() {
    LOG_I("Syncing video stream states...");
    foreach (int i, _videoServers.keys()) {
        VideoServer *server = _videoServers[i];
        StreamFormat format = _videoFormats[i];
        if (format.Encoding == UnknownEncoding) {
            if (server->getState() != VideoServer::IdleState) {
                LOG_I("Camera " + server->getCameraName() + " is about to be shut down");
                server->stop();
            }
        }
        else if (server->getState() == VideoServer::IdleState) {
            LOG_I("Camera " + server->getCameraName() + " is about to be streamed");
            if (_flycapCameras.contains(i)) {
                //this camera is flycap, we must set the framerate on it manually
                _flycapCameras[i]->setFramerate(format.Framerate);
                //give the server a format with no framerate specified, since the flycap camera will regulate that on its own
                StreamFormat noFrameratFormat(format);
                noFrameratFormat.Framerate = 0;
                server->start(noFrameratFormat);
            }
            else {
                server->start(format);
            }

        }
    }
}

// observers for video servers

void RoverProcess::videoServerStateChanged(VideoServer::State state) {
    switch (state) {
    case VideoServer::IdleState:
        syncVideoStreams();
        break;
    }
}

// observers for network channels message received

void RoverProcess::armChannelMessageReceived(const char *message, Channel::MessageSize size) {
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

void RoverProcess::driveChannelMessageReceived(const char *message, Channel::MessageSize size) {
    if (message[0] == DriveMessage::Header) {
        _driveControllerMbed->sendMessage(message, (int)size);
    }
    else {
        LOG_E("Received invalid message from mission control on drive control channel");
    }
}

void RoverProcess::gimbalChannelMessageReceived(const char *message, Channel::MessageSize size) {
    if (message[0] == GimbalMessage::Header) {
        _gimbalControllerMbed->sendMessage(message, (int)size);
    }
    else {
        LOG_E("Received invalid message from mission control on gimbal control channel");
    }
}

void RoverProcess::sharedChannelMessageReceived(const char *message, Channel::MessageSize size) {
    //TODO
}

RoverProcess::~RoverProcess() {
    if (_log != NULL) delete _log;
    if (_armChannel != NULL) delete _armChannel;
    if (_driveChannel != NULL) delete _driveChannel;
    if (_gimbalChannel != NULL) delete _gimbalChannel;
    if (_sharedChannel != NULL) delete _sharedChannel;
    if (_armControllerMbed != NULL) delete _armControllerMbed;
    if (_armControllerMbed != NULL) delete _driveControllerMbed;
    if (_armControllerMbed != NULL) delete _gimbalControllerMbed;
    if (_gpsServer != NULL) delete _gpsServer;
}

} // namespace Rover
} // namespace Soro
