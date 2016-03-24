#include "mcmainwindow.h"
#include "ui_mcmainwindow.h"

#define LOG_TAG "Mission Control"

//config
#define MARINI_PATH "master_arm_ranges.ini"
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

using namespace Soro::MissionControl;

McMainWindow::McMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::McMainWindow) {

    ui->setupUi(this);
    addShadow(ui->statusBarWidget);
    addShadow(ui->infoContainer);
    connect(ui->settingsButton, SIGNAL(clicked(bool)),
            this, SLOT(settingsClicked()));

    QString appPath = QCoreApplication::applicationDirPath();

    _log = new Logger(this);
    _log->setLogfile(appPath + "/mission_control.log");
    _log->RouteToQtLogger = true;
    _log->i(LOG_TAG, "-------------------------------------------------------");
    _log->i(LOG_TAG, "-------------------------------------------------------");
    _log->i(LOG_TAG, "-------------------------------------------------------");
    _log->i(LOG_TAG, "Starting up...");

    //must initialize from the event loop
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
        if (glfwJoystickPresent(_controllerId)) {
            int axisCount, buttonCount;
            const float *axes = glfwGetJoystickAxes(_controllerId, &axisCount);
            const unsigned char *buttons = glfwGetJoystickButtons(_controllerId, &buttonCount);
            switch (_mode) {
            case ArmLayoutMode:
                ArmMessage::setGlfwData(_buffer, axes, buttons, axisCount, buttonCount, _controlMap);
                _controlChannel->sendMessage(QByteArray::fromRawData(_buffer, ArmMessage::size(_buffer)));
                break;
            case DriveLayoutMode:
                //TODO
                break;
            case GimbalLayoutMode:
                //TODO
                break;
            }
        }
        else if (_controllerId != NO_CONTROLLER) {
            _log->w(LOG_TAG, "Controller ID " + QString::number(_controllerId) + " has been disconnected");
            QMessageBox(QMessageBox::Critical, "DAFUQ YOU DO?",
                        "The joystick or gamepad you were using to control the rover has been disconnected. Connect a controller, then load its appropriate button map file.",
                        QMessageBox::Ok, this).exec();
            _controllerId = NO_CONTROLLER;
        }
    }
    else if (e->timerId() == _initTimerId) {
        KILL_TIMER(_initTimerId); //single shot
        //parse soro.ini configuration
        SoroIniConfig config;
        QString err = QString::null;
        if (!config.parse(&err)) {
            _log->e(LOG_TAG, err);
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR", err, QMessageBox::Ok, this).exec();
            exit(1); return;
        }
        _armVideoPort = config.armVideoPort;
        _driveVideoPort = config.driveVideoPort;
        _gimbalVideoPort = config.gimbalVideoPort;  //TODO everything video

        //parse mission_control.ini configuration
        QString appPath = QApplication::applicationDirPath();
        IniParser configParser;
        QFile configFile(appPath + "/mission_control.ini");
        if (!configParser.load(configFile)) {
            _log->e(LOG_TAG, "The configuration file " + appPath + "/mission_control.ini is missing or invalid");
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                        "The configuration file " + appPath + "/mission_control.ini is missing or invalid",
                        QMessageBox::Ok, this).exec();
            exit(1); return;
        }
        QHostAddress mainHost, videoHost;
        if (!configParser.valueAsIP(MCINI_TAG_COMM_HOST_ADDRESS, &mainHost, true)) {
            _log->i(LOG_TAG, "Using default host for main communication");
            mainHost = QHostAddress::Any;
        }
        if (!configParser.valueAsIP(MCINI_TAG_VIDEO_HOST_ADDRESS, &videoHost, true)) {
            _log->i(LOG_TAG, "Using default host for video receiving");
            videoHost = QHostAddress::Any;
        }
        QString modeStr =  configParser.value(MCINI_TAG_LAYOUT);
        if (modeStr == MCINI_VALUE_LAYOUT_ARM) {
            _mode = ArmLayoutMode;
            QString inputMode = configParser.value(MCINI_TAG_INPUT_MODE);
            if (inputMode == MCINI_VALUE_USE_GLFW) {
                _log->i(LOG_TAG, "Input mode set to use GLFW (joystick or gamepad)");
                _inputMode = GLFW;
                glfwInit();
                _glfwInitialized = true;
            }
            else if (inputMode == MCINI_VALUE_USE_MASTER) {
                _log->i(LOG_TAG, "Input mode set to Master Arm");
                _inputMode = MasterArm;
                _masterArmSerial = new SerialChannel("ARM", this, _log);
                connect(_masterArmSerial, SIGNAL(messageReceived(const char*,int)),
                        this, SLOT(masterArmSerialMessageReceived(const char*,int)));
                connect(_masterArmSerial, SIGNAL(stateChanged(SerialChannel::State)),
                        this, SLOT(masterArmSerialStateChanged(SerialChannel::State)));
                loadMasterArmConfig();
            }
            else {
                QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                            "The configuration file " + appPath + "/mission_control.ini is invalid",
                            QMessageBox::Ok, this).exec();
                exit(1); return;
            }
            _controlChannel = new Channel(this, SocketAddress(config.serverAddress, config.armChannelPort), CHANNEL_NAME_ARM,
                                      Channel::UdpProtocol, Channel::ServerEndPoint, mainHost, _log);
        }
        else if (modeStr == MCINI_VALUE_LAYOUT_DRIVE) {
            _mode = DriveLayoutMode;
            glfwInit();
            _glfwInitialized = true;
            _controlChannel = new Channel(this, SocketAddress(config.serverAddress, config.driveChannelPort), CHANNEL_NAME_DRIVE,
                                      Channel::UdpProtocol, Channel::ServerEndPoint, mainHost, _log);
        }
        else if (modeStr == MCINI_VALUE_LAYOUT_GIMBAL) {
            _mode = GimbalLayoutMode;
            glfwInit();
            _glfwInitialized = true;
            _controlChannel = new Channel(this, SocketAddress(config.serverAddress, config.gimbalChannelPort), CHANNEL_NAME_GIMBAL,
                                      Channel::UdpProtocol, Channel::ServerEndPoint, mainHost, _log);
        }
        else if (modeStr == MCINI_VALUE_LAYOUT_SPECTATOR) {
            _mode = SpectatorLayoutMode;
        }
        else {
            _log->e(LOG_TAG, "Unknown configuration value for MCINI_TAG_LAYOUT");
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                        "The configuration file " + appPath + "/mission_control.ini is invalid",
                        QMessageBox::Ok, this).exec();
            exit(1); return;
        }
        _sharedChannel = new Channel(this, SocketAddress(config.serverAddress, config.sharedChannelPort), CHANNEL_NAME_SHARED,
                                  Channel::TcpProtocol, Channel::ClientEndPoint, QHostAddress::Any, _log);

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
        _log->i(LOG_TAG, "Configuration has been loaded successfully");
    }
}

void McMainWindow::masterArmSerialStateChanged(SerialChannel::State) {

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
        _buffer[ARM_MSG_BUCKET_FAST_OPEN_INDEX] = 1;
        _buffer[ARM_MSG_BUCKET_FAST_CLOSE_INDEX] = 0;
    }
    else if (_currentKey == 'c') {
        _buffer[ARM_MSG_BUCKET_FAST_CLOSE_INDEX] = 1;
        _buffer[ARM_MSG_BUCKET_FAST_OPEN_INDEX] = 0;
    }
    else {
        _buffer[ARM_MSG_BUCKET_FAST_CLOSE_INDEX] = 0;
        _buffer[ARM_MSG_BUCKET_FAST_OPEN_INDEX] = 0;
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
        if ((_controlChannel == NULL) || (_controlChannel->getState() == Channel::ConnectedState)) {
            ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
            ui->comm_mainStateLabel->setText("Connected");
        }
        break;
    case Channel::ConnectingState:
        ui->comm_sharedStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_sharedStateLabel->setText("Shared link is connecting...");
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
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_mainStateLabel->setText("Connecting...");
        break;
    case Channel::ReadyState:
        ui->comm_sharedStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_sharedStateLabel->setText("Shared link ready");
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
        if (_sharedChannel->getState() == Channel::ConnectedState) {
            ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
            ui->comm_mainStateLabel->setText("Connected");
        }
        break;
    case Channel::ConnectingState:
        ui->comm_controlStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_controlStateLabel->setText("Control link is connecting...");
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
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_mainStateLabel->setText("Connecting...");
        break;
    case Channel::ReadyState:
        ui->comm_controlStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_controlStateLabel->setText("Control link ready");
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_mainStateLabel->setText("Ready");
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_mainStateLabel->setText("Connecting...");
        break;
    }
}

void McMainWindow::controlChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                                       int rateUp, int rateDown) {
    if (rtt = -1) {
        ui->comm_mainPingLabel->setText("---");
    }
    else {
        ui->comm_mainPingLabel->setText(QString::number(rtt));
    }
}

void McMainWindow::sharedChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                                       int rateUp, int rateDown) {

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
    loadMasterArmConfig();  //TODO make this a menu
}

void McMainWindow::loadMasterArmConfig() {
    if (_mode == ArmLayoutMode) {
        QString appPath = QApplication::applicationDirPath();
        QFile masterArmFile(appPath + "/" + MARINI_PATH);
        if (_masterArmRanges.parse(masterArmFile)) {
            _log->i(LOG_TAG, "Loaded master arm configuration");
        }
        else {
            _log->e(LOG_TAG, "Could not load master arm configuration");
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                        "The master arm configuration file " + appPath + "/" + MARINI_PATH + " is either missing or invalid",
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
    if (_controlChannel != NULL) delete _controlChannel;
    if (_sharedChannel != NULL) delete _sharedChannel;
    if (_masterArmSerial != NULL) delete _masterArmSerial;
    if (_log != NULL) delete _log;
    delete ui;
    delete _videoWindow;
    if (_glfwInitialized) glfwTerminate();
}
