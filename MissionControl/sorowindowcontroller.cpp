#include "sorowindowcontroller.h"

#define LOG_TAG "Mission Control"

//config
#define MASTER_ARM_INI_PATH QCoreApplication::applicationDirPath() + "/config/master_arm.ini"
//https://github.com/gabomdq/SDL_GameControllerDB
#define SDL_MAP_FILE_PATH QCoreApplication::applicationDirPath() + "/config/gamecontrollerdb.txt"

#define CONTROL_SEND_INTERVAL 50

namespace Soro {
namespace MissionControl {

SoroWindowController::SoroWindowController(QObject *parent) : QObject(parent) {
    _log = new Logger(this);
    _log->setLogfile(QCoreApplication::applicationDirPath() + "/mission_control" + QDateTime::currentDateTime().toString("M-dd_h:mm_AP") + ".log");
    //_log->RouteToQtLogger = true;
}

void SoroWindowController::init() {
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("Starting up...");
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
    Channel::EndPoint commEndPoint =
            _soroIniConfig.ServerSide == SoroIniLoader::MissionControlEndPoint ?
                Channel::ServerEndPoint : Channel::ClientEndPoint;

    //parse mission control configuration
    if (!_mcIniConfig.load(&err)) {
        LOG_E(err);
        emit error(err);
        return;
    }
    switch (_mcIniConfig.Layout) {
    case MissionControlIniLoader::ArmLayoutMode:
        switch (_mcIniConfig.ControlInputMode) {
        case MissionControlIniLoader::Gamepad:
            initSDL();
            break;
        case MissionControlIniLoader::MasterArm:
            arm_loadMasterArmConfig();
            _masterArmChannel = new MbedChannel(SocketAddress(QHostAddress::Any, _mcIniConfig.MasterArmPort), MBED_ID_MASTER_ARM, _log);
            connect(_masterArmChannel, SIGNAL(messageReceived(const char*,int)),
                    this, SLOT(masterArmMessageReceived(const char*,int)));
            break;
        }
        _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.ArmChannelPort), CHANNEL_NAME_ARM,
                Channel::UdpProtocol, commEndPoint, _mcIniConfig.CommHostAddress, _log);
        break;
    case MissionControlIniLoader::DriveLayoutMode:
        initSDL();
        _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.DriveChannelPort), CHANNEL_NAME_DRIVE,
                Channel::UdpProtocol, commEndPoint, _mcIniConfig.CommHostAddress, _log);
        break;
    case MissionControlIniLoader::GimbalLayoutMode:
        initSDL();
        _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.GimbalChannelPort), CHANNEL_NAME_GIMBAL,
                Channel::UdpProtocol, commEndPoint, _mcIniConfig.CommHostAddress, _log);
        break;
    case MissionControlIniLoader::SpectatorLayoutMode:
        break;
    }

    if (_mcIniConfig.MasterNode) {
        LOG_I("Setting up as master node");
        //channel for the rover
        _sharedChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.SharedChannelPort), CHANNEL_NAME_SHARED,
                Channel::TcpProtocol, commEndPoint, _mcIniConfig.CommHostAddress, _log);
        connect(_sharedChannel, SIGNAL(statisticsUpdate(int,quint64,quint64,int,int)),
                this, SLOT(sharedChannelStatsUpdate(int,quint64,quint64,int,int)));
        //channel for other mission control computers to connect to
        LOG_I("Hosting " + QString::number(_mcIniConfig.NodePorts[1] - _mcIniConfig.NodePorts[0]) + " mission control nodes");
        for (int i = 0; i < _mcIniConfig.NodePorts[1] - _mcIniConfig.NodePorts[0]; i++) {
            Channel *sc = new Channel(this, SocketAddress(QHostAddress::Any, _mcIniConfig.NodePorts[i]),
                                      "MC_NODE_" + QString::number(_mcIniConfig.NodePorts[i]),
                                      Channel::TcpProtocol, Channel::ServerEndPoint, _mcIniConfig.NodeHostAddress, _log);
            _sharedChannelNodes.append(sc);

            connect(sc, SIGNAL(messageReceived(const char*, Channel::MessageSize)),
                    this, SLOT(sharedChannelNodeMessageReceived(const char*, Channel::MessageSize)));
        }
    }
    else {
        LOG_I("Setting up as normal node");
        //channel to connect to the master mission control computer
        _sharedChannel = new Channel(this, SocketAddress(_mcIniConfig.MasterNodeAddress.host, _mcIniConfig.MasterNodeAddress.port),
                "MC_NODE_" + QString::number(_mcIniConfig.MasterNodeAddress.port),
                Channel::TcpProtocol, Channel::ClientEndPoint, _mcIniConfig.NodeHostAddress, _log);
    }

    connect(_sharedChannel, SIGNAL(messageReceived(const char*, Channel::MessageSize)),
            this, SLOT(sharedChannelMessageReceived(const char*, Channel::MessageSize)));

    if ((_controlChannel != NULL) && (_controlChannel->getState() == Channel::ErrorState)) {
        emit error("The control channel experienced a fatal error. This is most likely due to a configuration problem.");
        return;
    }
    if (_sharedChannel->getState() == Channel::ErrorState) {
        emit error("The shared data channel experienced a fatal error. This is most likely due to a configuration problem.");
        return;
    }
    LOG_I("Configuration has been loaded successfully");

    if (_controlChannel != NULL) _controlChannel->open();
    _sharedChannel->open();
}

void SoroWindowController::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _controlSendTimerId) {
        /***************************************
         * This code sends gamepad data to the rover at a regular interval
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
            switch (_mcIniConfig.Layout) {
            case MissionControlIniLoader::ArmLayoutMode:
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
            case MissionControlIniLoader::DriveLayoutMode:
                switch (_driveGamepadMode) {
                case SingleStick:
                    DriveMessage::setGamepadData_DualStick(_buffer,
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTX),
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTY),
                                                 _driveMiddleSkidSteerFactor);
                    break;
                case DualStick:
                    DriveMessage::setGamepadData_DualStick(_buffer,
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTX),
                                                 SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_RIGHTY),
                                                 _driveMiddleSkidSteerFactor);
                    break;
                }
                _controlChannel->sendMessage(_buffer, DriveMessage::RequiredSize);
                break;
            case MissionControlIniLoader::GimbalLayoutMode:
                GimbalMessage::setGamepadData(_buffer,
                                              SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTX),
                                              SDL_GameControllerGetAxis(_gameController, SDL_CONTROLLER_AXIS_LEFTY));
                _controlChannel->sendMessage(_buffer, GimbalMessage::RequiredSize);
                break;
            }
        }
    }
    else if (e->timerId() == _inputSelectorTimerId) {
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
}

/* Initializes SDL for gamepad input and loads
 * the gamepad map file.
 */
void SoroWindowController::initSDL() {
    if (!_sdlInitialized) {
        LOG_I("Input mode set to use SDL");
        if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
            emit error("SDL failed to initialize: " + QString(SDL_GetError()));
            return;
        }
        _sdlInitialized = true;
        _gameController = NULL;
        if (SDL_GameControllerAddMappingsFromFile((SDL_MAP_FILE_PATH).toLocal8Bit().data()) == -1) {
            emit error("Failed to load SDL gamepad map: " + QString(SDL_GetError()));
            return;
        }
        START_TIMER(_controlSendTimerId, CONTROL_SEND_INTERVAL);
        emit initializedSDL();
        emit gamepadChanged(NULL);
    }
}

/* Facade for SDL_Quit that cleans up some things here as well
 */
void SoroWindowController::quitSDL() {
    if (_sdlInitialized) {
        SDL_Quit();
        _gameController = NULL;
        _sdlInitialized = false;
    }
}

void SoroWindowController::arm_masterArmMessageReceived(const char *message, int size) {
    memcpy(_buffer, message, size);
    //translate message from master pot values to slave servo values
    ArmMessage::translateMasterArmValues(_buffer, _masterArmRanges);
    qDebug() << "yaw=" << ArmMessage::getMasterYaw(_buffer)
             << ", shldr=" << ArmMessage::getMasterShoulder(_buffer)
             << ", elbow=" << ArmMessage::getMasterElbow(_buffer)
             << ", wrist=" << ArmMessage::getMasterWrist(_buffer); /**/
    _controlChannel->sendMessage(_buffer, size);
}

/* Receives TCP messages from the rover only (as master node) or from the rover
 * and all other mission controls
 */
void SoroWindowController::sharedChannelMessageReceived(const char *message, Channel::MessageSize size) {
    if (_mcIniConfig.MasterNode) {
        //rebroadcast to othr mission controls
        foreach (Channel *c, _sharedChannelNodes) {
            c->sendMessage(message, size);
        }
    }
    else {
        //TODO
    }
}

/* Receives TCP messages from other mission control computers (master node only)
 */
void SoroWindowController::sharedChannelNodeMessageReceived(const char *message, Channel::MessageSize size) {
    //forward to rover
    _sharedChannel->sendMessage(message, size);
    //rebroadcast to other mission control computers (including the sender)
    foreach (Channel *c, _sharedChannelNodes) {
        c->sendMessage(message, size);
    }
}

/* Receives status updates from the shared channel to the rover (master node only)
 */
void SoroWindowController::sharedChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                        int rateUp, int rateDown) {
    if (_controlChannel != NULL) {
        emit connectionQualityUpdate(rtt, rtt - _controlChannel->getLastRtt());
    }
    else {
        emit connectionQualityUpdate(rtt, -1);
    }
}

void SoroWindowController::arm_loadMasterArmConfig() {
    if (_mcIniConfig.Layout == MissionControlIniLoader::ArmLayoutMode) {
        QFile masterArmFile(MASTER_ARM_INI_PATH);
        if (_masterArmRanges.load(masterArmFile)) {
            LOG_I("Loaded master arm configuration");
        }
        else {
            emit error("The master arm configuration file " + MASTER_ARM_INI_PATH + " is either missing or invalid");
        }
    }
}

SDL_GameController *SoroWindowController::getGamepad() {
    return _gameController;
}

const SoroIniLoader *SoroWindowController::getSoroIniConfig() const {
    return &_soroIniConfig;
}

const MissionControlIniLoader *SoroWindowController::getMissionControlIniConfig() const {
    return &_mcIniConfig;
}

const Channel *SoroWindowController::getControlChannel() const {
    return _controlChannel;
}

const Channel *SoroWindowController::getSharedChannel() const {
    return _sharedChannel;
}

const MbedChannel* SoroWindowController::arm_getMasterArmChannel() const {
    return _masterArmChannel;
}

void SoroWindowController::drive_setMiddleSkidSteerFactor(float factor) {
    _driveMiddleSkidSteerFactor = factor;
}

void SoroWindowController::drive_setGamepadMode(DriveGamepadMode mode) {
    _driveGamepadMode = mode;
}

float SoroWindowController::drive_getMiddleSkidSteerFactor() const {
    return _driveMiddleSkidSteerFactor;
}

SoroWindowController::DriveGamepadMode SoroWindowController::drive_getGamepadMode() const {
    return _driveGamepadMode;
}

SoroWindowController::~SoroWindowController() {
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
