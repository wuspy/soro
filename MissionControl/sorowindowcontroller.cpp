#include "sorowindowcontroller.h"

#define LOG_TAG "Mission Control"

#define APPPATH QCoreApplication::applicationDirPath()

//config
#define MARINI_PATH "config/master_arm.ini"
#define MCINI_PATH "config/mission_control.ini"
#define GLFWINI_PATH "config/last_glfw_config.ini"
#define MCINI_TAG_LAYOUT "Layout"
#define MCINI_VALUE_LAYOUT_ARM "arm"
#define MCINI_VALUE_LAYOUT_DRIVE "drive"
#define MCINI_VALUE_LAYOUT_GIMBAL "gimbal"
#define MCINI_VALUE_LAYOUT_SPECTATOR "spectator"
#define MCINI_TAG_COMM_HOST_ADDRESS "MainHostAddress"
#define MCINI_TAG_VIDEO_HOST_ADDRESS "VideoHostAddress"
#define MCINI_TAG_INPUT_MODE "InputMode"
#define MCINI_VALUE_USE_GLFW "glfw"
#define MCINI_VALUE_USE_MASTER "masterarm"

#define CONTROL_SEND_INTERVAL 50

using namespace Soro;
using namespace Soro::MissionControl;

SoroWindowController::SoroWindowController(QObject *parent) : QObject(parent) {
    _log = new Logger(this);
    _log->setLogfile(APPPATH + "/mission_control" + QDateTime::currentDateTime().toString("M-dd_h:mm:AP") + ".log");
    //_log->RouteToQtLogger = true;
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("Starting up...");

    //must initialize after event loop starts
    START_TIMER(_initTimerId, 1);
}

void SoroWindowController::settingsClicked() {
    GlfwMap *map = getInputMap();
    if (map != NULL) {
        GlfwMapDialog d(NULL, _controllerId, map);
        d.exec();
    }
    else if (_inputMode == MasterArm) {
        loadMasterArmConfig();
    }
}

void SoroWindowController::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _controlSendTimerId) {

        /***************************************
         * This code sends gamepad data to the rover at a regular interval
         */

        if (glfwJoystickPresent(_controllerId)) {
            int axisCount, buttonCount;
            const float *axes = glfwGetJoystickAxes(_controllerId, &axisCount);
            const unsigned char *buttons = glfwGetJoystickButtons(_controllerId, &buttonCount);
            switch (_mode) {
            case ArmLayoutMode:
                ArmMessage::setGlfwData(_buffer, axes, buttons, axisCount, buttonCount, *_armInputMap);
                _controlChannel->sendMessage(QByteArray::fromRawData(_buffer, ArmMessage::size(_buffer)));
                break;
            case DriveLayoutMode:
                DriveMessage::setGlfwData(_buffer, axes, buttons, axisCount, buttonCount, *_driveInputMap);
                _controlChannel->sendMessage(QByteArray::fromRawData(_buffer, DriveMessage::size(_buffer)));
                break;
            case GimbalLayoutMode:
                GimbalMessage::setGlfwData(_buffer, axes, buttons, axisCount, buttonCount, *_gimbalInputMap);
                _controlChannel->sendMessage(QByteArray::fromRawData(_buffer, GimbalMessage::size(_buffer)));
                break;
            }
        }

        /***************************************
         ***************************************/
    }
    else if (e->timerId() == _inputSelectorTimerId) {

        /***************************************
         * This code monitors the state of the connected joysticks and selects an available one when connected
         */

        int newJoy = firstGlfwControllerId();
        if (newJoy != _controllerId) {
            _controllerId = newJoy;
            if (_controllerId == NO_CONTROLLER) {
                //the connected controller has been disconnected
                KILL_TIMER(_controlSendTimerId);
                emit gamepadChanged(QString::null);
            }
            else {
                //a new controller is connected
                GlfwMap *map = getInputMap();
                if (map != NULL) {
                    QString controllerName = glfwGetJoystickName(_controllerId);
                    START_TIMER(_controlSendTimerId, CONTROL_SEND_INTERVAL);
                    if (map->ControllerName != controllerName) {
                        //the controller selected does not match the one in the mapping file
                        map->reset();
                        map->ControllerName = controllerName;
                        emit warning("The controller you have just plugged in appears to be a different model than the last "
                                     "one you were using. You will need to set up this controller, but know that this"
                                     " will overwrite the saved button mapping you had for any previous controller.");
                    }
                    emit gamepadChanged(controllerName);
                }
            }
        }

        /***************************************
         ***************************************/

    }
    else if (e->timerId() == _initTimerId) {

        /***************************************
         * This code handles the initialization and reading the configuration file
         * This has to be run after the event loop has been started, hence why it's
         * in a timer event instead of in the main method
         */

        KILL_TIMER(_initTimerId); //single shot
        //parse soro.ini configuration
        QString err = QString::null;
        if (!_soroIniConfig.load(&err)) {
            LOG_E(err);
            emit error(err);
            return;
        }
        _soroIniConfig.applyLogLevel(_log);
        Channel::EndPoint commEndPoint =
                _soroIniConfig.ServerSide == SoroIniConfig::MissionControlEndPoint ?
                    Channel::ServerEndPoint : Channel::ClientEndPoint;

        //parse configuration
        IniParser configParser;
        QFile configFile(APPPATH + "/" + MCINI_PATH);
        if (!configParser.load(configFile)) {
            LOG_E("The configuration file " + APPPATH + "/" + MCINI_PATH + " is missing or invalid");
            emit error("The configuration file " + APPPATH + "/" + MCINI_PATH + " missing or invalid");
            return;
        }
        QHostAddress commHost, videoHost;
        if (!configParser.valueAsIP(MCINI_TAG_COMM_HOST_ADDRESS, &commHost, true)) {
            LOG_I("Using default host for main communication");
            commHost = QHostAddress::Any;
        }
        if (!configParser.valueAsIP(MCINI_TAG_VIDEO_HOST_ADDRESS, &videoHost, true)) {
            LOG_I("Using default host for video receiving");
            videoHost = QHostAddress::Any;
        }
        QString modeStr = configParser.value(MCINI_TAG_LAYOUT);
        if (QString::compare(modeStr, MCINI_VALUE_LAYOUT_ARM, Qt::CaseInsensitive) == 0) {
            _mode = ArmLayoutMode;
            QString inputMode = configParser.value(MCINI_TAG_INPUT_MODE);
            if (QString::compare(inputMode, MCINI_VALUE_USE_GLFW, Qt::CaseInsensitive) == 0) {
                //use gamepad to control the arm
                _armInputMap = new ArmGlfwMap();
                initForGLFW(_armInputMap);
            }
            else if (QString::compare(inputMode, MCINI_VALUE_USE_MASTER, Qt::CaseInsensitive) == 0) {
                //Use the master/slave arm input method
                LOG_I("Input mode F57F17set to Master Arm");
                _inputMode = MasterArm;
                loadMasterArmConfig();
                _masterArmSerial = new SerialChannel3(MASTER_ARM_SERIAL_CHANNEL_NAME, this, _log);
            }
            else {
                emit error("The configuration file " + APPPATH + "/" + MCINI_PATH + " is invalid (can't determine input mode)");
                return;
            }
            _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.ArmChannelPort), CHANNEL_NAME_ARM,
                                      Channel::UdpProtocol, commEndPoint, commHost, _log);
        }
        else if ((QString::compare(modeStr, MCINI_VALUE_LAYOUT_DRIVE, Qt::CaseInsensitive) == 0)) {
            _mode = DriveLayoutMode;
            _driveInputMap = new DriveGlfwMap();
            initForGLFW(_driveInputMap);
            _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.DriveChannelPort), CHANNEL_NAME_DRIVE,
                                      Channel::UdpProtocol, commEndPoint, commHost, _log);
        }
        else if (QString::compare(modeStr, MCINI_VALUE_LAYOUT_GIMBAL, Qt::CaseInsensitive) == 0) {
            _mode = GimbalLayoutMode;
            _gimbalInputMap = new GimbalGlfwMap();
            initForGLFW(_gimbalInputMap);
            _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.GimbalChannelPort), CHANNEL_NAME_GIMBAL,
                                      Channel::UdpProtocol, commEndPoint, commHost, _log);
        }
        else if (QString::compare(modeStr, MCINI_VALUE_LAYOUT_SPECTATOR, Qt::CaseInsensitive) == 0) {
            _mode = SpectatorLayoutMode;
        }
        else {
            LOG_E("Unknown configuration value for MCINI_TAG_LAYOUT");
            emit error("The configuration file " + APPPATH + "/" + MCINI_PATH + " is invalid (can't determine layout)");
            return;
        }
        _sharedChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.SharedChannelPort), CHANNEL_NAME_SHARED,
                            Channel::TcpProtocol, commEndPoint, commHost, _log);
        connect(_sharedChannel, SIGNAL(messageReceived(QByteArray)),
                this, SLOT(sharedChannelMessageReceived(QByteArray)));

        if ((_controlChannel != NULL) && (_controlChannel->getState() == Channel::ErrorState)) {
            emit error("The control channel experienced a fatal error. This is most likely due to a configuration problem.");
            return;
        }
        if (_sharedChannel->getState() == Channel::ErrorState) {
            emit error("The shared data channel experienced a fatal error. This is most likely due to a configuration problem.");
            return;
        }
        LOG_I("Configuration has been loaded successfully");
        emit initialized(_soroIniConfig, _mode, _inputMode);

        if (_controlChannel != NULL) _controlChannel->open();
        _sharedChannel->open();

        /***************************************
         ***************************************/

    }
}

GlfwMap* SoroWindowController::getInputMap() {
    if (_inputMode == GLFW) {
        switch(_mode) {
        case ArmLayoutMode:
            return _armInputMap;
        case DriveLayoutMode:
            return _driveInputMap;
        case GimbalLayoutMode:
            return _gimbalInputMap;
        }
    }
    return NULL;
}

int SoroWindowController::firstGlfwControllerId() {
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
        if (glfwJoystickPresent(i)) return i;
    }
    return NO_CONTROLLER;
}

void SoroWindowController::initForGLFW(GlfwMap *map) {
    LOG_I("Input mode set to use GLFW (joystick or gamepad)");
    glfwInit();
    _inputMode = GLFW;
    _glfwInitialized = true;
    QFile mapFile(APPPATH + (QString)"/" + GLFWINI_PATH);
    map->loadMapping(mapFile);
    START_TIMER(_inputSelectorTimerId, 1000);
    //update ui
    emit gamepadChanged(QString::null);
}

void SoroWindowController::masterArmSerialMessageReceived(const char *message, int size) {
    qDebug() << "yaw=" << ArmMessage::masterYaw(message)
             << ", shldr=" << ArmMessage::masterShoulder(message)
             << ", elbow=" << ArmMessage::masterElbow(message)
             << ", wrist=" << ArmMessage::masterWrist(message)
             << ", bucket=" << ArmMessage::masterBucket(message);/**/

    memcpy(_buffer, message, size);
    //control bucket with keyboard keys
    //TODO bill build the fucking master arm
    if (_currentKey == 'x') {
        _buffer[ARM_MESSAGE_BUCKET_FULL_OPEN_INDEX] = 1;
        _buffer[ARM_MESSAGE_BUCKET_FULL_CLOSE_INDEX] = 0;
    }
    else if (_currentKey == 'c') {
        _buffer[ARM_MESSAGE_BUCKET_FULL_CLOSE_INDEX] = 1;
        _buffer[ARM_MESSAGE_BUCKET_FULL_OPEN_INDEX] = 0;
    }
    else {
        _buffer[ARM_MESSAGE_BUCKET_FULL_CLOSE_INDEX] = 0;
        _buffer[ARM_MESSAGE_BUCKET_FULL_OPEN_INDEX] = 0;
    }
    //translate message from master pot values to slave servo values
    ArmMessage::translateMasterArmValues(_buffer, _masterArmRanges);
    _controlChannel->sendMessage(QByteArray::fromRawData(_buffer, size));
}

void SoroWindowController::currentKeyChanged(char key) {
    _currentKey = key;
}

void SoroWindowController::sharedChannelMessageReceived(const QByteArray& message) {
    //TODO
}

void SoroWindowController::loadMasterArmConfig() {
    if (_mode == ArmLayoutMode) {
        QFile masterArmFile(APPPATH + "/" + MARINI_PATH);
        if (_masterArmRanges.load(masterArmFile)) {
            LOG_I("Loaded master arm configuration");
        }
        else {
            LOG_E("Could not load master arm configuration");
            emit error("The master arm configuration file " + APPPATH + "/" + MARINI_PATH + " is either missing or invalid");
            return;
        }
    }
}

const Channel *SoroWindowController::getControlChannel() const {
    return _controlChannel;
}

const Channel *SoroWindowController::getSharedChannel() const {
    return _sharedChannel;
}

const SerialChannel3* SoroWindowController::getMasterArmSerial() const {
    return _masterArmSerial;
}

SoroWindowController::~SoroWindowController() {
    GlfwMap *map = getInputMap();
    if (map != NULL) {
        if (map->isMapped()) {
            QFile mapFile(APPPATH + (QString)"/" + GLFWINI_PATH);
            map->writeMapping(mapFile);
            LOG_I("Writing glfw input map to " + APPPATH + (QString)"/" + GLFWINI_PATH);
        }
        delete map;
    }
    if (_controlChannel != NULL) delete _controlChannel;
    if (_sharedChannel != NULL) delete _sharedChannel;
    if (_masterArmSerial != NULL) delete _masterArmSerial;
    if (_log != NULL) delete _log;
    if (_glfwInitialized) glfwTerminate();
}
