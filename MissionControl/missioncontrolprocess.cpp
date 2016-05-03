#include "missioncontrolprocess.h"

#define LOG_TAG "Mission Control"

//config
#define MASTER_ARM_INI_PATH QCoreApplication::applicationDirPath() + "/config/master_arm.ini"
//https://github.com/gabomdq/SDL_GameControllerDB
#define SDL_MAP_FILE_PATH QCoreApplication::applicationDirPath() + "/config/gamecontrollerdb.txt"

#define CONTROL_SEND_INTERVAL 50

#define CHANNEL_NAME_SUBNET_SHARED "Soro_MissionControlChannel"

namespace Soro {
namespace MissionControl {

MissionControlProcess::MissionControlProcess(VideoStreamWidget *topVideo, VideoStreamWidget *bottomVideo,
                                             bool masterSubnetNode, MissionControlProcess::Role role, QMainWindow *presenter) : QObject(presenter) {
    _masterNode = masterSubnetNode;
    _role = role;
    _topVideoWidget = topVideo;
    _bottomVideoWidget = bottomVideo;
    _log = new Logger(this);
    _log->setLogfile(QCoreApplication::applicationDirPath() + "/mission_control" + QDateTime::currentDateTime().toString("M-dd_h:mm_AP") + ".log");
    _log->RouteToQtLogger = true;
    _log->MaxQtLoggerLevel = LOG_LEVEL_DEBUG;
}

void MissionControlProcess::init() {
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("Starting up...");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    /***************************************
     * This code handles the initialization and reading the configuration file
     * This has to be run after the event loop has been started
     */
    //parse soro.ini configuration
    QString err = QString::null;
    if (!_soroIniConfig.load(&err)) {
        LOG_E(err);
        emit error(err);
        return;
    }
    _soroIniConfig.applyLogLevel(_log);
    LOG_I("Configuration has been loaded successfully");

    switch (_role) {
    case ArmOperator:
        arm_loadMasterArmConfig();
        _masterArmChannel = new MbedChannel(SocketAddress(QHostAddress::Any, _soroIniConfig.MasterArmPort), MBED_ID_MASTER_ARM, _log);
        connect(_masterArmChannel, SIGNAL(messageReceived(const char*,int)),
                this, SLOT(arm_masterArmMessageReceived(const char*,int)));
        connect(_masterArmChannel, SIGNAL(stateChanged(MbedChannel::State)),
                this, SIGNAL(arm_masterArmStateChanged(MbedChannel::State)));
        if (_soroIniConfig.ServerSide == SoroIniLoader::MissionControlEndPoint) {
            _controlChannel = new Channel(this, _soroIniConfig.ArmChannelPort, CHANNEL_NAME_ARM,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        else {
            _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.ArmChannelPort), CHANNEL_NAME_ARM,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        break;
    case Driver:
        initSDL();
        if (_soroIniConfig.ServerSide == SoroIniLoader::MissionControlEndPoint) {
            _controlChannel = new Channel(this, _soroIniConfig.DriveChannelPort, CHANNEL_NAME_DRIVE,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        else {
            _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.DriveChannelPort), CHANNEL_NAME_DRIVE,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        break;
    case CameraOperator:
        initSDL();
        if (_soroIniConfig.ServerSide == SoroIniLoader::MissionControlEndPoint) {
            _controlChannel = new Channel(this, _soroIniConfig.GimbalChannelPort, CHANNEL_NAME_GIMBAL,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        else {
            _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.GimbalChannelPort), CHANNEL_NAME_GIMBAL,
                    Channel::UdpProtocol, QHostAddress::Any, _log);
        }
        break;
    case Spectator:
        //no control connections to create since spectators don't control anything
        break;
    }

    _broadcastSocket = new QUdpSocket(this);
    if (_masterNode) {
        LOG_I("Setting up as master subnet node");
        //create the main shared channel to connect to the rover
        if (_soroIniConfig.ServerSide == SoroIniLoader::MissionControlEndPoint) {
            _sharedChannel = new Channel(this, _soroIniConfig.SharedChannelPort, CHANNEL_NAME_SHARED,
                    Channel::TcpProtocol, QHostAddress::Any, _log);
        }
        else {
            _sharedChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.SharedChannelPort), CHANNEL_NAME_SHARED,
                    Channel::TcpProtocol, QHostAddress::Any, _log);
        }
        _sharedChannel->open();
        connect(_sharedChannel, SIGNAL(messageReceived(const char*,Channel::MessageSize)),
                this, SLOT(roverSharedChannelMessageReceived(const char*,Channel::MessageSize)));
        connect(_sharedChannel, SIGNAL(stateChanged(Channel::State)),
                this, SLOT(roverSharedChannelStateChanged(Channel::State)));
        //create the udp broadcast receive port to listen to other mission control nodes trying to connect
        if (!_broadcastSocket->bind(QHostAddress::Any, _soroIniConfig.McBroadcastPort)) {
            emit error("Unable to bind subnet broadcast port on " + QString::number(_soroIniConfig.McBroadcastPort));
            return;
        }
        if (!_broadcastSocket->open(QIODevice::ReadWrite)) {
            emit error("Unable to open subnet broadcast port on " + QString::number(_soroIniConfig.McBroadcastPort));
            return;
        }
        connect(_broadcastSocket, SIGNAL(readyRead()),
                this, SLOT(broadcastSocketReadyRead()));
        //start a timer to monitor inactive shared channels
        START_TIMER(_pruneSharedChannelsTimerId, 10000);
    }
    else {
        LOG_I("Setting up as slave subnet node");
        //create a tcp channel on a random port to act as a server, and
        //create a udp socket on the same port to send out broadcasts with the
        //server's information so the master node can connect
        _sharedChannel = new Channel(this, 0, CHANNEL_NAME_SUBNET_SHARED,
                Channel::TcpProtocol, QHostAddress::Any, _log);
        _sharedChannel->open();
        connect(_sharedChannel, SIGNAL(stateChanged(Channel::State)),
                this, SLOT(slaveSharedChannelStateChanged(Channel::State)));
        connect(_sharedChannel, SIGNAL(messageReceived(const char*,Channel::MessageSize)),
                this, SLOT(handleSharedChannelMessage(const char*,Channel::MessageSize)));
        if (!_broadcastSocket->bind(QHostAddress::Any, _sharedChannel->getHostAddress().port)) {
            emit error("Unable to bind subnet broadcast port on " + QString::number(_sharedChannel->getHostAddress().port));
            return;
        }
        if (!_broadcastSocket->open(QIODevice::ReadWrite)) {
            emit error("Unable to open subnet broadcast port on " + QString::number(_sharedChannel->getHostAddress().port));
            return;
        }
        START_TIMER(_broadcastSharedChannelInfoTimerId, 1000);
    }
    //also connect the shared channel's state changed event to this signal
    //so the UI can be updated. This signal is also broadcast when the rover
    //becomes disconnected from the master mission control, and they inform us
    //of that event since we are connected to the rover through them.
    connect(_sharedChannel, SIGNAL(stateChanged(Channel::State)),
            this, SIGNAL(sharedChannelStateChanged(Channel::State)));
    if (_controlChannel != NULL) {
        connect(_controlChannel, SIGNAL(stateChanged(Channel::State)),
                this, SIGNAL(controlChannelStateChanged(Channel::State)));
        connect(_controlChannel, SIGNAL(statisticsUpdate(int,quint64,quint64,int,int)),
                this, SIGNAL(controlChannelStatsUpdate(int, quint16, quint64, int, int)));
        _controlChannel->open();
        if (_controlChannel->getState() == Channel::ErrorState) {
            emit error("The control channel experienced a fatal error. This is most likely due to a configuration problem.");
            return;
        }
    }

    if (_sharedChannel->getState() == Channel::ErrorState) {
        emit error("The shared data channel experienced a fatal error. This is most likely due to a configuration problem.");
        return;
    }

    LOG_I("***************Initializing Video system******************");

    if (_masterNode) {
        LOG_I("Creating video clients");
        _armVideoClient = new VideoClient(VIDEOSTREAM_NAME_ARM, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.ArmVideoPort), QHostAddress::Any, _log, this);
        _driveVideoClient = new VideoClient(VIDEOSTREAM_NAME_DRIVE, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.DriveVideoPort), QHostAddress::Any, _log, this);
        _gimbalVideoClient = new VideoClient(VIDEOSTREAM_NAME_GIMBAL, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.GimbalVideoPort), QHostAddress::Any, _log, this);
        _fisheyeVideoClient = new VideoClient(VIDEOSTREAM_NAME_FISHEYE, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.FisheyeVideoPort), QHostAddress::Any, _log, this);

        connect(_gimbalVideoClient, SIGNAL(serverError()),
                this, SLOT(gimbalVideoClientError()));
        connect(_gimbalVideoClient, SIGNAL(serverEos()),
                this, SLOT(gimbalVideoClientEos()));
        connect(_gimbalVideoClient, SIGNAL(stateChanged(VideoClient::State)),
                this, SLOT(gimbalVideoClientStateChanged(VideoClient::State)));
        connect(_gimbalVideoClient, SIGNAL(stopped()),
                this, SLOT(gimbalVideoClientStop()));

        _gimbalVideoClient->addForwardingAddress(SocketAddress(QHostAddress::LocalHost, _soroIniConfig.GimbalVideoPort));
    }
    else {

    }
}

void MissionControlProcess::gimbalVideoClientEos() {
     _topVideoWidget->endStream("This stream is over.");
}

void MissionControlProcess::gimbalVideoClientError() {
     _topVideoWidget->endStream("This rover experienced an error.");
}

void MissionControlProcess::gimbalVideoClientStateChanged(VideoClient::State state) {
    switch (state) {
    case VideoClient::StreamingState:
        _topVideoWidget->configure(SocketAddress(QHostAddress::LocalHost, _soroIniConfig.GimbalVideoPort), VideoEncoding::MJPEG);
        break;
    case VideoClient::ConnectingState:
        _topVideoWidget->endStream("Connection to the rover has been lost.");
        break;
    }
}

void MissionControlProcess::gimbalVideoClientStop() {
    _topVideoWidget->endStream("This stream has been turned off.");
}

void MissionControlProcess::broadcastSocketReadyRead() {
    SocketAddress peer;
    while (_broadcastSocket->hasPendingDatagrams()) {
        int len = _broadcastSocket->readDatagram(_buffer, strlen(CHANNEL_NAME_SUBNET_SHARED) + 1, &peer.host, &peer.port);
        if ((size_t)len < strlen(CHANNEL_NAME_SUBNET_SHARED) + 1) continue;
        if (strcmp(_buffer, CHANNEL_NAME_SUBNET_SHARED) == 0) {
            //found a new mission control trying to connect
            //make sure they're not already added (delayed UDP packet or something)
            bool added = false;
            foreach (Channel *c, _sharedChannelNodes) {
                if (c->getProvidedServerAddress() == peer) {
                    added = true;
                    break;
                }
            }
            if (!added) {
                //not already added, create a channel for them
                LOG_I("Creating new channel for node " + peer.toString());
                Channel *channel = new Channel(this, peer, CHANNEL_NAME_SUBNET_SHARED,
                                   Channel::TcpProtocol, _broadcastSocket->localAddress(), _log);
                channel->open();
                _sharedChannelNodes.append(channel);
                connect(channel, SIGNAL(messageReceived(const char*,Channel::MessageSize)),
                        this, SLOT(nodeSharedChannelMessageReceived(const char*,Channel::MessageSize)));
            }
        }
    }
}

void MissionControlProcess::roverSharedChannelStateChanged(Channel::State state) {
    //broadcast to other mission controls that the rover isn't connected
    //TODO
}

void MissionControlProcess::handleSharedChannelMessage(const char *message, Channel::MessageSize size) {
    //TODO
}

void MissionControlProcess::roverSharedChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown, int rateUp, int rateDown) {
    //TODO
}

void MissionControlProcess::roverSharedChannelMessageReceived(const char *message, Channel::MessageSize size) {
    //message from the rover, rebroadcast to all other mission controls
    handleSharedChannelMessage(message, size);
    foreach (Channel *c, _sharedChannelNodes) {
        c->sendMessage(message, size);
    }
}

void MissionControlProcess::nodeSharedChannelMessageReceived(const char *message, Channel::MessageSize size) {
    //resend to all other mission controls (including the sender) and the rover
    handleSharedChannelMessage(message, size);
    _sharedChannel->sendMessage(message, size);
    foreach (Channel *c, _sharedChannelNodes) {
        c->sendMessage(message, size);
    }
}

void MissionControlProcess::slaveSharedChannelStateChanged(Channel::State state) {
    LOG_D("slaveSharedChannelStateChanged()");
    switch (state) {
    case Channel::ConnectedState:
        //connected to the master mission control, stop broadcasting
        KILL_TIMER(_broadcastSharedChannelInfoTimerId);
        break;
    case Channel::ConnectingState:
        //lost connection to the master mission control, start broadcasting
        START_TIMER(_broadcastSharedChannelInfoTimerId, 1000);
        break;
    case Channel::ErrorState:
        emit error("The shared channel experienced a fatal error");
        break;
    }
}

void MissionControlProcess::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _controlSendTimerId) {
        /***************************************
         * This timer sends gamepad data to the rover at a regular interval
         * Not applicable for master arm control, the mbed controls the
         * packet rate in that scenario
         */
        if (_gameController) {
            SDL_GameControllerUpdate();
            if (!SDL_GameControllerGetAttached(_gameController)) {
                delete _gameController;
                _gameController = NULL;
                START_TIMER(_inputSelectorTimerId, 1000);
                emit gamepadChanged(NULL);
                return;
            }
            switch (_role) {
            case ArmOperator:
                //send the rover an arm gamepad packet
                ArmMessage::setGamepadData(_buffer,
                                           SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTX),
                                           SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTY),
                                           SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_RIGHTY),
                                           SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_TRIGGERLEFT),
                                           SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT),
                                           SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER),
                                           SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER),
                                           SDL_GameControllerGetButton(_gameController, SDL_CONTROLLER_BUTTON_Y));
                _controlChannel->sendMessage(_buffer, ArmMessage::RequiredSize_Gamepad);
                break;
            case Driver:
                //send the rover a drive gamepad packet
                switch (_driveGamepadMode) {
                case SingleStick:
                    DriveMessage::setGamepadData_DualStick(_buffer,
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTX),
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTY),
                                                 _driveMiddleSkidSteerFactor);
                    break;
                case DualStick:
                    DriveMessage::setGamepadData_DualStick(_buffer,
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTY),
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_RIGHTY),
                                                 _driveMiddleSkidSteerFactor);
                    break;
                }
                _controlChannel->sendMessage(_buffer, DriveMessage::RequiredSize);
                break;
            case CameraOperator:
                //send the rover a gimbal gamepad packet
                GimbalMessage::setGamepadData(_buffer,
                                              SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTX),
                                              SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTY));
                _controlChannel->sendMessage(_buffer, GimbalMessage::RequiredSize);
                break;
            }
        }
    }
    else if (e->timerId() == _inputSelectorTimerId) {
        /***************************************
         * This timer querys SDL at regular intervals to look for a
         * suitable controller
         */
        SDL_GameControllerUpdate();
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                SDL_GameController *controller = SDL_GameControllerOpen(i);
                if (controller && SDL_GameControllerMapping(controller)) {
                    //this gamepad will do
                    _gameController = controller;
                    emit gamepadChanged(controller);
                    KILL_TIMER(_inputSelectorTimerId);
                    return;
                }
                SDL_GameControllerClose(controller);
                delete controller;
            }
        }
    }
    else if (e->timerId() == _broadcastSharedChannelInfoTimerId) {
        /***************************************
         * This timer broadcasts our information to the entire subnet so
         * the master mission control can connect to us
         */
        LOG_I("Broadcasting shared channel information on address "
              + SocketAddress(_broadcastSocket->localAddress(), _broadcastSocket->localPort()).toString());
        _broadcastSocket->writeDatagram(CHANNEL_NAME_SUBNET_SHARED, strlen(CHANNEL_NAME_SUBNET_SHARED) + 1,
                                        QHostAddress::Broadcast, _soroIniConfig.McBroadcastPort);
    }
}

/* Initializes SDL for gamepad input and loads
 * the gamepad map file.
 */
void MissionControlProcess::initSDL() {
    LOG_D("initSDL()");
    if (!_sdlInitialized) {
        LOG_I("Input mode set to use SDL");
        if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
            emit error("SDL failed to initialize: " + QString(SDL_GetError()));
            return;
        }
        _sdlInitialized = true;
        _gameController = NULL;
        if (SDL_GameControllerAddMappingsFromFile((SDL_MAP_FILE_PATH).toLocal8Bit().constData()) == -1) {
            emit error("Failed to load SDL gamepad map: " + QString(SDL_GetError()));
            return;
        }
        START_TIMER(_controlSendTimerId, CONTROL_SEND_INTERVAL);
        START_TIMER(_inputSelectorTimerId, 1000);
        emit initializedSDL();
        emit gamepadChanged(NULL);
    }
}

/* Facade for SDL_Quit that cleans up some things here as well
 */
void MissionControlProcess::quitSDL() {
    LOG_D("quitSDL()");
    if (_sdlInitialized) {
        SDL_Quit();
        _gameController = NULL;
        _sdlInitialized = false;
    }
}

void MissionControlProcess::arm_masterArmMessageReceived(const char *message, int size) {
    LOG_D("arm_masterArmMessageReceived()");
    memcpy(_buffer, message, size);
    //translate message from master pot values to slave servo values
    ArmMessage::translateMasterArmValues(_buffer, _masterArmRanges);
    qDebug() << "yaw=" << ArmMessage::getMasterYaw(_buffer)
             << ", shldr=" << ArmMessage::getMasterShoulder(_buffer)
             << ", elbow=" << ArmMessage::getMasterElbow(_buffer)
             << ", wrist=" << ArmMessage::getMasterWrist(_buffer); /**/
    if (_controlChannel != NULL) {
        _controlChannel->sendMessage(_buffer, size);
    }
    else {
        LOG_E("Got message from master arm with null control channel");
    }
}


void MissionControlProcess::arm_loadMasterArmConfig() {
    LOG_D("loadMasterArmConfig()");
    if (_role == ArmOperator) {
        QFile masterArmFile(MASTER_ARM_INI_PATH);
        if (_masterArmRanges.load(masterArmFile)) {
            LOG_I("Loaded master arm configuration");
        }
        else {
            emit error("The master arm configuration file " + MASTER_ARM_INI_PATH + " is either missing or invalid");
        }
    }
}

SDL_GameController *MissionControlProcess::getGamepad() {
    return _gameController;
}

const SoroIniLoader *MissionControlProcess::getSoroIniConfig() const {
    return &_soroIniConfig;
}

const Channel *MissionControlProcess::getControlChannel() const {
    return _controlChannel;
}

const Channel *MissionControlProcess::getSharedChannel() const {
    return _sharedChannel;
}

const MbedChannel* MissionControlProcess::arm_getMasterArmChannel() const {
    return _masterArmChannel;
}

void MissionControlProcess::drive_setMiddleSkidSteerFactor(float factor) {
    _driveMiddleSkidSteerFactor = factor;
}

void MissionControlProcess::drive_setGamepadMode(DriveGamepadMode mode) {
    _driveGamepadMode = mode;
}

float MissionControlProcess::drive_getMiddleSkidSteerFactor() const {
    return _driveMiddleSkidSteerFactor;
}

MissionControlProcess::DriveGamepadMode MissionControlProcess::drive_getGamepadMode() const {
    return _driveGamepadMode;
}

MissionControlProcess::Role MissionControlProcess::getRole() const {
    return _role;
}

bool MissionControlProcess::isMasterSubnetNode() const {
    return _masterNode;
}

MissionControlProcess::~MissionControlProcess() {
    foreach (Channel *c, _sharedChannelNodes) {
        delete c;
    }
    if (_controlChannel != NULL) delete _controlChannel;
    if (_sharedChannel != NULL) delete _sharedChannel;
    if (_masterArmChannel != NULL) delete _masterArmChannel;
    if (_log != NULL) delete _log;
    quitSDL();
}

}
}
