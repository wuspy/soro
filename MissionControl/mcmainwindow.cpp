#include "mcmainwindow.h"
#include "ui_mcmainwindow.h"

#define LOG_TAG "Mission Control"
#define ARM_CAMERA_DISPLAY_NAME "Arm Camera"
#define DRIVE_CAMERA_DISPLAY_NAME "Drive Camera"
#define GIMBAL_CAMERA_DISPLAY_NAME "Gimbal Camera"

#define APPPATH QCoreApplication::applicationDirPath()

//config
#define MARINI_PATH "config/master_arm.ini"
#define MCINI_PATH "config/mission_control.ini"
#define GLFWINI_PATH "config/last_glfw_config.ini"
#define MCINI_TAG_LAYOUT "Layout"
#define MCINI_VALUE_LAYOUT_ARM "Arm"
#define MCINI_VALUE_LAYOUT_DRIVE "Drive"
#define MCINI_VALUE_LAYOUT_GIMBAL "Gimbal"
#define MCINI_VALUE_LAYOUT_SPECTATOR "Spectator"
#define MCINI_TAG_COMM_HOST_ADDRESS "MainHostAddress"
#define MCINI_TAG_VIDEO_HOST_ADDRESS "VideoHostAddress"
#define MCINI_TAG_INPUT_MODE "InputMode"
#define MCINI_VALUE_USE_GLFW "GLFW"
#define MCINI_VALUE_USE_MASTER "MasterArm"

#define CONTROL_SEND_INTERVAL 50

using namespace Soro;
using namespace Soro::MissionControl;

McMainWindow::McMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::McMainWindow) {

    ui->setupUi(this);
    addShadow(ui->statusBarWidget);
    addShadow(ui->infoContainer);
    connect(ui->settingsButton, SIGNAL(clicked(bool)),
            this, SLOT(settingsClicked()));

    _log = new Logger(this);
    _log->setLogfile(APPPATH + "/mission_control.log");
    //_log->RouteToQtLogger = true;
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("-------------------------------------------------------");
    LOG_I("Starting up...");

    _videoWindow = new VideoWindow(this);
    _videoWindow->show();

    //must initialize after event loop starts
    START_TIMER(_initTimerId, 1);
}

void McMainWindow::timerEvent(QTimerEvent *e) {
    QMainWindow::timerEvent(e);

    //JUST TESTING SHIT
    /*float lat = 29.564844 + qrand() % 1000 * 0.000001;
    float lng = -95.081317 + qrand() % 1000 * 0.000001;
    ui->googleMapView->updateLocation(LatLng(lat, lng));
    ui->googleMapView->updateHeading(rand() % 360);*/


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
                ui->comm_inputDeviceLabel->setText("No input devices");
                //update ui
                ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_yellow_18px.png);");
                ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #F57F17; }");
                ui->comm_inputDeviceLabel->setText("No input devices");
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
                        QMessageBox(QMessageBox::Critical, "Unknown Controller",
                                    "The controller you have just plugged in appears to be a differnt model than the last "
                                    "one you were using. You will need to set up this controller's configuration, but know that this"
                                    " will overwrite the saved configuration you had for any previous controllers.",
                                    QMessageBox::Ok, this).exec();
                    }
                    //update ui
                    ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_green_18px.png);");
                    ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #1B5E20; }");
                    ui->comm_inputDeviceLabel->setText(controllerName);
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
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR", err, QMessageBox::Ok, this).exec();
            exit(1); return;
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
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                        "The configuration file " + APPPATH + "/" + MCINI_PATH + " missing or invalid",
                        QMessageBox::Ok, this).exec();
            exit(1); return;
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
        QString modeStr =  configParser.value(MCINI_TAG_LAYOUT);
        if (modeStr == MCINI_VALUE_LAYOUT_ARM) {
            _mode = ArmLayoutMode;
            QString inputMode = configParser.value(MCINI_TAG_INPUT_MODE);
            if (inputMode == MCINI_VALUE_USE_GLFW) {
                //use gamepad to control the arm
                _armInputMap = new ArmGlfwMap();
                initForGLFW(_armInputMap);
            }
            else if (inputMode == MCINI_VALUE_USE_MASTER) {
                //Use the master/slave arm input method
                LOG_I("Input mode F57F17set to Master Arm");
                _inputMode = MasterArm;
                loadMasterArmConfig();
                _masterArmSerial = new SerialChannel(MASTER_ARM_SERIAL_CHANNEL_NAME, this, _log);
                masterArmSerialStateChanged(SerialChannel::ConnectingState);
                connect(_masterArmSerial, SIGNAL(messageReceived(const char*,int)),
                        this, SLOT(masterArmSerialMessageReceived(const char*,int)));
                connect(_masterArmSerial, SIGNAL(stateChanged(SerialChannel::State)),
                        this, SLOT(masterArmSerialStateChanged(SerialChannel::State)));
            }
            else {
                QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                            "The configuration file " + APPPATH + "/" + MCINI_PATH + " is invalid (can't determine input mode)",
                            QMessageBox::Ok, this).exec();
                exit(1); return;
            }
            _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.ArmChannelPort), CHANNEL_NAME_ARM,
                                      Channel::UdpProtocol, commEndPoint, commHost, _log);
            ui->videoPane1->setCameraName(DRIVE_CAMERA_DISPLAY_NAME);
            ui->videoPane2->setCameraName(GIMBAL_CAMERA_DISPLAY_NAME);
            _videoWindow->getVideoPane()->setCameraName(ARM_CAMERA_DISPLAY_NAME);
        }
        else if (modeStr == MCINI_VALUE_LAYOUT_DRIVE) {
            _mode = DriveLayoutMode;
            _driveInputMap = new DriveGlfwMap();
            initForGLFW(_driveInputMap);
            _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.DriveChannelPort), CHANNEL_NAME_DRIVE,
                                      Channel::UdpProtocol, commEndPoint, commHost, _log);
            ui->videoPane1->setCameraName(GIMBAL_CAMERA_DISPLAY_NAME);
            ui->videoPane2->setCameraName(ARM_CAMERA_DISPLAY_NAME);
            _videoWindow->getVideoPane()->setCameraName(DRIVE_CAMERA_DISPLAY_NAME);
        }
        else if (modeStr == MCINI_VALUE_LAYOUT_GIMBAL) {
            _mode = GimbalLayoutMode;
            _gimbalInputMap = new GimbalGlfwMap();
            initForGLFW(_gimbalInputMap);
            _controlChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.GimbalChannelPort), CHANNEL_NAME_GIMBAL,
                                      Channel::UdpProtocol, commEndPoint, commHost, _log);
            ui->videoPane1->setCameraName(DRIVE_CAMERA_DISPLAY_NAME);
            ui->videoPane2->setCameraName(ARM_CAMERA_DISPLAY_NAME);
            _videoWindow->getVideoPane()->setCameraName(GIMBAL_CAMERA_DISPLAY_NAME);
        }
        else if (modeStr == MCINI_VALUE_LAYOUT_SPECTATOR) {
            _mode = SpectatorLayoutMode;
            ui->videoPane1->setCameraName(DRIVE_CAMERA_DISPLAY_NAME);
            ui->videoPane2->setCameraName(ARM_CAMERA_DISPLAY_NAME);
            _videoWindow->getVideoPane()->setCameraName(GIMBAL_CAMERA_DISPLAY_NAME);
        }
        else {
            LOG_E("Unknown configuration value for MCINI_TAG_LAYOUT");
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                        "The configuration file " + APPPATH + "/" + MCINI_PATH + " is invalid (can't determine layout)",
                        QMessageBox::Ok, this).exec();
            exit(1); return;
        }
        _sharedChannel = new Channel(this, SocketAddress(_soroIniConfig.ServerAddress, _soroIniConfig.SharedChannelPort), CHANNEL_NAME_SHARED,
                            Channel::TcpProtocol, commEndPoint, commHost, _log);
        sharedChannelStateChanged(Channel::ConnectingState);
        connect(_sharedChannel, SIGNAL(messageReceived(QByteArray)),
                this, SLOT(sharedChannelMessageReceived(QByteArray)));
        connect(_sharedChannel, SIGNAL(stateChanged(Channel::State)),
                this, SLOT(sharedChannelStateChanged(Channel::State)));
        connect (_sharedChannel, SIGNAL(statisticsUpdate(int,quint64,quint64,int,int)),
                 this, SLOT(sharedChannelStatsUpdate(int,quint64,quint64,int,int)));
        if (_controlChannel != NULL) {
            controlChannelStateChanged(Channel::ConnectingState);
            connect (_controlChannel, SIGNAL(stateChanged(Channel::State)),
                     this, SLOT(controlChannelStateChanged(Channel::State)));
            connect(_controlChannel, SIGNAL(statisticsUpdate(int,quint64,quint64,int,int)),
                    this, SLOT(controlChannelStatsUpdate(int,quint64,quint64,int,int)));
        }

        if ((_controlChannel != NULL) && (_controlChannel->getState() == Channel::ErrorState)) {
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                        "The control channel experienced a fatal error. This is most likely due to a configuration problem.",
                        QMessageBox::Ok, this).exec();
            exit(1); return;
        }
        if (_sharedChannel->getState() == Channel::ErrorState) {
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                        "The shared data channel experienced a fatal error. This is most likely due to a configuration problem.",
                        QMessageBox::Ok, this).exec();
            exit(1); return;
        }
        LOG_I("Configuration has been loaded successfully");

        /***************************************
         ***************************************/

    }
}

GlfwMap* McMainWindow::getInputMap() {
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

int McMainWindow::firstGlfwControllerId() {
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
        if (glfwJoystickPresent(i)) return i;
    }
    return NO_CONTROLLER;
}

void McMainWindow::initForGLFW(GlfwMap *map) {
    LOG_I("Input mode set to use GLFW (joystick or gamepad)");
    glfwInit();
    _inputMode = GLFW;
    _glfwInitialized = true;
    QFile mapFile(APPPATH + (QString)"/" + GLFWINI_PATH);
    map->loadMapping(mapFile);
    START_TIMER(_inputSelectorTimerId, 1000);
    //update ui
    ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_yellow_18px.png);");
    ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #F57F17; }");
    ui->comm_inputDeviceLabel->setText("No input devices");
}

void McMainWindow::masterArmSerialStateChanged(SerialChannel::State state) {
    switch (state) {
    case SerialChannel::ConnectedState:
        ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_inputDeviceLabel->setText("Master arm connected");
        ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_green_18px.png);");
        break;
    case SerialChannel::ConnectingState:
        ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_inputDeviceLabel->setText("Connecting to master arm...");
        ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_yellow_18px.png);");
        break;
    }
}

void McMainWindow::masterArmSerialMessageReceived(const char *message, int size) {
    qDebug() << "yaw=" << ArmMessage::masterYaw(message)
             << ", shldr=" << ArmMessage::masterShoulder(message)
             << ", elbow=" << ArmMessage::masterElbow(message)
             << ", wrist=" << ArmMessage::masterWrist(message)
             << ", bucket=" << ArmMessage::masterBucket(message);

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

void McMainWindow::sharedChannelMessageReceived(const QByteArray& message) {
    //TODO
}

void McMainWindow::sharedChannelStateChanged(Channel::State state) {
    switch (state) {
    case Channel::ConnectedState:
        ui->comm_sharedStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_sharedStateLabel->setText("Shared link is connected");
        ui->comm_sharedStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/check_circle_green_18px.png);");
        if ((_controlChannel == NULL) || (_controlChannel->getState() == Channel::ConnectedState)) {
            ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
            ui->comm_mainStateLabel->setText("Connected");
        }
        break;
    case Channel::ConnectingState:
        ui->comm_sharedStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_sharedStateLabel->setText("Shared link is connecting...");
        ui->comm_sharedStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/minus_circle_yellow_18px.png);");
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_mainStateLabel->setText("Connecting...");
        break;
    case Channel::ErrorState:
        QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                    "The shared data channel experienced a fatal error. This is most likely due to a configuration problem.",
                    QMessageBox::Ok, this).exec();
        exit(1);
        return;
    case Channel::UnconfiguredState:
        ui->comm_sharedStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_sharedStateLabel->setText("Shared link is configuring...");
        ui->comm_sharedStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/minus_circle_yellow_18px.png);");
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_mainStateLabel->setText("Connecting...");
        break;
    case Channel::ReadyState:
        ui->comm_sharedStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_sharedStateLabel->setText("Shared link ready");
        ui->comm_sharedStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/minus_circle_black_18px.png);");
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_mainStateLabel->setText("Ready");
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_mainStateLabel->setText("Connecting...");
        break;
    }
}

void McMainWindow::controlChannelStateChanged(Channel::State state) {
    switch (state) {
    case Channel::ConnectedState:
        ui->comm_controlStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_controlStateLabel->setText("Control link is connected");
        ui->comm_controlStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/check_circle_green_18px.png);");
        if (_sharedChannel->getState() == Channel::ConnectedState) {
            ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
            ui->comm_mainStateLabel->setText("Connected");
        }
        break;
    case Channel::ConnectingState:
        ui->comm_controlStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_controlStateLabel->setText("Control link is connecting...");
        ui->comm_controlStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/minus_circle_yellow_18px.png);");
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_mainStateLabel->setText("Connecting...");
        break;
    case Channel::ErrorState:
        QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                    "The control channel experienced a fatal error. This is most likely due to a configuration problem.",
                    QMessageBox::Ok, this).exec();
        exit(1);
        return;
    case Channel::UnconfiguredState:
        ui->comm_controlStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_controlStateLabel->setText("Control link is configuring...");
        ui->comm_controlStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/minus_circle_yellow_18px.png);");
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_mainStateLabel->setText("Connecting...");
        break;
    case Channel::ReadyState:
        ui->comm_controlStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_controlStateLabel->setText("Control link ready");
        ui->comm_controlStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/minus_circle_black_18px.png);");
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_mainStateLabel->setText("Ready");
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_mainStateLabel->setText("Connecting...");
        break;
    }
}

void McMainWindow::controlChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                                       int rateUp, int rateDown) {
    if (rtt == -1) {
        ui->comm_mainPingLabel->setText("---");
    }
    else {
        ui->comm_mainPingLabel->setText(QString::number(rtt));
    }
}

void McMainWindow::sharedChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                                       int rateUp, int rateDown) {
    //TODO
}

void McMainWindow::resizeEvent(QResizeEvent* event) {
   QMainWindow::resizeEvent(event);
   ui->videoContainerWidget->resize(height() / 2 * 1.77777, height());
   ui->infoContainer->resize(width() - ui->videoContainerWidget->width(), ui->infoContainer->height());
   ui->videoContainerWidget->move(width() - ui->videoContainerWidget->width(), 0);
   ui->googleMapView->move(0, ui->infoContainer->height());
   ui->statusBarWidget->resize(width() - ui->videoContainerWidget->width(), 30);
   ui->statusBarWidget->move(0, height() - 30);
   ui->googleMapView->resize(width() - ui->videoContainerWidget->width() + 2,
                             height() - ui->statusBarWidget->height() - ui->infoContainer->height() + 1);
}

void McMainWindow::settingsClicked() {
    GlfwMap *map = getInputMap();
    if (map != NULL) {
        GlfwMapDialog d(NULL, _controllerId, map);
        d.exec();
    }
    else if (_inputMode == MasterArm) {
        loadMasterArmConfig();
    }
}

void McMainWindow::loadMasterArmConfig() {
    if (_mode == ArmLayoutMode) {
        QFile masterArmFile(APPPATH + "/" + MARINI_PATH);
        if (_masterArmRanges.load(masterArmFile)) {
            LOG_I("Loaded master arm configuration");
        }
        else {
            LOG_E("Could not load master arm configuration");
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                        "The master arm configuration file " + APPPATH + "/" + MARINI_PATH + " is either missing or invalid",
                        QMessageBox::Ok, this).exec();
            exit(1);
            return;
        }
    }
}

void McMainWindow::keyPressEvent(QKeyEvent *e) {
    QMainWindow::keyPressEvent(e);
    if (e->key() == Qt::Key_F11) {
        if (_fullscreen) showNormal(); else showFullScreen();
        _fullscreen = !_fullscreen;
    }
    else _currentKey = QChar(e->key()).toLower().toLatin1();
}

void McMainWindow::keyReleaseEvent(QKeyEvent *e) {
    QMainWindow::keyReleaseEvent(e);
    if (QChar(e->key()).toLower().toLatin1() == _currentKey) _currentKey = '\0';
}

McMainWindow::~McMainWindow() {
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
    delete ui;
    if (_videoWindow != NULL) delete _videoWindow;
    if (_glfwInitialized) glfwTerminate();
}
