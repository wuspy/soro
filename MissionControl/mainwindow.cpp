#include "mainwindow.h"
#include "ui_soromainwindow.h"

static QString formatDataRate(quint64 rate, QString units) {
    if (rate > 1000000000) {
        return QString::number(rate / 1000000000) + "G" + units;
    }
    if (rate > 1000000) {
        return QString::number(rate/ 1000000) + "M" + units;
    }
    if (rate > 1000) {
        return QString::number(rate / 1000) + "K" + units;
    }
    return QString::number(rate) + units;
}

namespace Soro {
namespace MissionControl {

const QString MainWindow::_logLevelFormattersHTML[4] = {
    "<div style=\"color:#b71c1c\">%1&emsp;E/<i>%2</i>:&emsp;%3</div>",
    "<div style=\"color:#0d47a1\">%1&emsp;W/<i>%2</i>:&emsp;%3</div>",
    "<div>%1&emsp;I/<i>%2</i>:&emsp;%3</div>",
    "<div style=\"color:#dddddd\">%1&emsp;D/<i>%2</i>:&emsp;%3</div>"
};

MainWindow::MainWindow(const Configuration *config, GamepadManager *gamepad,
                               MissionControlNetwork *mcNetwork, ControlSystem *controlSystem, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SoroMainWindow) {

    ui->setupUi(this);
    _config = config;
    _mcNetwork = mcNetwork;
    _gamepad = gamepad;
    _controlSystem = controlSystem;

    _videoWindow = new CameraWindow(this);
    _videoWindow->show();

    _preloaderMovie = new QMovie(this);
    _preloaderMovie->setFileName(":/icons/preloader_white_yellow_36px.gif");

    ui->media_camera1ControlWidget->setName("Camera 1");
    ui->media_camera2ControlWidget->setName("Camera 2");
    ui->media_camera3ControlWidget->setName("Camera 3");
    ui->media_camera4ControlWidget->setName("Camera 4");
    ui->media_camera5ControlWidget->setName("Camera 5");

    connect(ui->media_audioControlWidget, SIGNAL(optionSelected(AudioFormat)),
            this, SIGNAL(audioStreamFormatChanged(AudioFormat)));
    connect(ui->media_audioControlWidget, SIGNAL(muteToggled(bool)),
            this, SIGNAL(audioStreamMuteChanged(bool)));
    connect(ui->media_camera1ControlWidget, SIGNAL(optionSelected(VideoFormat)),
            this, SLOT(camera1ControlOptionChanged(VideoFormat)));
    connect(ui->media_camera2ControlWidget, SIGNAL(optionSelected(VideoFormat)),
            this, SLOT(camera2ControlOptionChanged(VideoFormat)));
    connect(ui->media_camera3ControlWidget, SIGNAL(optionSelected(VideoFormat)),
            this, SLOT(camera3ControlOptionChanged(VideoFormat)));
    connect(ui->media_camera4ControlWidget, SIGNAL(optionSelected(VideoFormat)),
            this, SLOT(camera4ControlOptionChanged(VideoFormat)));
    connect(ui->media_camera5ControlWidget, SIGNAL(optionSelected(VideoFormat)),
            this, SLOT(camera5ControlOptionChanged(VideoFormat)));

    connect(ui->media_camera1ControlWidget, SIGNAL(userEditedName(QString)),
            this, SLOT(camera1NameEdited(QString)));
    connect(ui->media_camera2ControlWidget, SIGNAL(userEditedName(QString)),
            this, SLOT(camera2NameEdited(QString)));
    connect(ui->media_camera3ControlWidget, SIGNAL(userEditedName(QString)),
            this, SLOT(camera3NameEdited(QString)));
    connect(ui->media_camera4ControlWidget, SIGNAL(userEditedName(QString)),
            this, SLOT(camera4NameEdited(QString)));
    connect(ui->media_camera5ControlWidget, SIGNAL(userEditedName(QString)),
            this, SLOT(camera5NameEdited(QString)));

    connect(ui->masterarm_reloadFileButton, SIGNAL(clicked(bool)),
            this, SIGNAL(reloadMasterArmClicked()));

    addWidgetShadow(ui->statusBarWidget, 10, 0);
    addWidgetShadow(ui->infoContainer, 10, 0);
    addWidgetShadow(ui->videoContainer, 10, 0);

    switch (_mcNetwork->getRole()) {
    case ArmOperatorRole: {
        ArmControlSystem *armControlSystem = reinterpret_cast<ArmControlSystem*>(_controlSystem);
        ui->statusLabel->setText("<html>Arm Operator" + QString(_mcNetwork->isBroker() ? " <span style=\"color:#b71c1c\"><b>[MASTER]</b></span>" : "") + "</html>");
        connect(armControlSystem, SIGNAL(masterArmStateChanged(bool)),
                this, SLOT(onMasterArmStateChanged(bool)));
        connect(armControlSystem, SIGNAL(masterArmUpdate(const char*)),
                this, SLOT(onMasterArmUpdate(const char*)));
        onMasterArmStateChanged(armControlSystem->isMasterArmConnected());
    }
        break;
    case DriverRole:
        ui->statusLabel->setText("<html>Driver" + QString(_mcNetwork->isBroker() ? " <span style=\"color:#b71c1c\"><b>[MASTER]</b></span>" : "") + "</html>");
        onGamepadChanged(_gamepad->getGamepad(), _gamepad->getGamepadName());
        break;
    case CameraOperatorRole:
        ui->statusLabel->setText("<html>Camera Operator" + QString(_mcNetwork->isBroker() ? " <span style=\"color:#b71c1c\"><b>[MASTER]</b></span>" : "") + "</html>");
        onGamepadChanged(_gamepad->getGamepad(), _gamepad->getGamepadName());
        break;
    case SpectatorRole:
        ui->statusLabel->setText("<html>Spectator" + QString(_mcNetwork->isBroker() ? " <span style=\"color:#b71c1c\"><b>[MASTER]</b></span>" : "") + "</html>");
        ui->hid_inputDeviceLabel->setStyleSheet("color: black;");
        ui->hid_inputDeviceLabel->setText("Not available for spectators");
        ui->hid_inputDeviceGraphicLabel->setStyleSheet("");
        ui->comm_controlStateLabel->setStyleSheet("color: black;");
        ui->comm_controlStateLabel->setText("Not available for spectators");
        ui->comm_controlStateGraphicLabel->setStyleSheet("");
        break;
    }

    // With the new network architecture, this window will not be shown if the mcc connection goes out
    ui->comm_mccStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
    ui->comm_mccStateLabel->setText("Connected to MCC network");
    ui->comm_mccStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/check_circle_green_18px.png);");

    connect(_gamepad, SIGNAL(gamepadChanged(SDL_GameController*,QString)),
            this, SLOT(onGamepadChanged(SDL_GameController*,QString)));
    if (_controlSystem) {
        connect(_controlSystem->getChannel(), SIGNAL(stateChanged(Channel*,Channel::State)),
                this, SLOT(onControlChannelStateChanged(Channel*,Channel::State)));
    }

    onArmSubsystemStateChanged(UnknownSubsystemState);
    onDriveCameraSubsystemStateChanged(UnknownSubsystemState);
    onSecondaryComputerStateChanged(UnknownSubsystemState);
}

void MainWindow::updateConnectionStateInformation() {
    // update control channel state UI
    if (_mcNetwork->getRole() != SpectatorRole) {
        switch (_lastControlChannelState) {
        case Channel::ConnectedState:
            ui->comm_controlStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
            ui->comm_controlStateLabel->setText("Connected to Rover");
            ui->comm_controlStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/check_circle_green_18px.png);");
            break;
        case Channel::ErrorState:
            QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                        "The rover control channel experienced a fatal error. This is most likely due to a configuration problem.",
                        QMessageBox::Ok, this).exec();
            exit(1);
            return;
        default:
            ui->comm_controlStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
            ui->comm_controlStateLabel->setText("Connecting to Rover...");
            ui->comm_controlStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/minus_circle_yellow_18px.png);");
            break;
        }
    }

    // update shared network state UI
    switch (_lastRoverChannelState) {
    case Channel::ConnectedState:
        ui->comm_sharedStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_sharedStateLabel->setText("Connected to Shared Link");
        ui->comm_sharedStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/check_circle_green_18px.png);");
        // also update media controls
        ui->media_audioControlWidget->setAvailable(true);
        ui->media_camera1ControlWidget->setAvailable(true);
        ui->media_camera2ControlWidget->setAvailable(true);
        ui->media_camera3ControlWidget->setAvailable(true);
        ui->media_camera4ControlWidget->setAvailable(true);
        ui->media_camera5ControlWidget->setAvailable(true);
        break;
    case Channel::ErrorState:
        QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                    "The control channel experienced a fatal error. This is most likely due to a configuration problem.",
                    QMessageBox::Ok, this).exec();
        exit(1);
        return;
    default:
        ui->comm_sharedStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_sharedStateLabel->setText("Connecting to Shared Link...");
        ui->comm_sharedStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/minus_circle_yellow_18px.png);");
        // also update media controls
        ui->media_audioControlWidget->setAvailable(false);
        ui->media_camera1ControlWidget->setAvailable(false);
        ui->media_camera2ControlWidget->setAvailable(false);
        ui->media_camera3ControlWidget->setAvailable(false);
        ui->media_camera4ControlWidget->setAvailable(false);
        ui->media_camera5ControlWidget->setAvailable(false);
        break;
    }

    // update main status label
    if (((_lastControlChannelState == Channel::ConnectedState) || (_mcNetwork->getRole() == SpectatorRole))
            && (_lastRoverChannelState == Channel::ConnectedState)) {
        ui->comm_statusContainer->setStyleSheet("background-color: #1B5E20;\
                                                color: white;\
                                                border-radius: 10px;");
        ui->comm_mainStatusGraphicLabel->setMovie(NULL);
        ui->comm_mainStatusGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/check_white_36px.png);");
        _preloaderMovie->stop();
        ui->comm_mainStatusLabel->setText("Connected");
        ui->comm_detailStatusLabel->setText(QString::number(_lastRtt) + "ms Ping - " + QString::number(100 - _lastDroppedPacketPercent) + "% Integrity");
    }
    else {
        ui->comm_statusContainer->setStyleSheet("background-color: #F57F17;\
                                                color: white;\
                                                border-radius: 10px;");
        ui->comm_mainStatusGraphicLabel->setStyleSheet("");
        ui->comm_mainStatusGraphicLabel->setMovie(_preloaderMovie);
        _preloaderMovie->start();

        int connections = 2;
        if (_lastControlChannelState == Channel::ConnectedState) connections++;
        if (_lastRoverChannelState == Channel::ConnectedState) connections++;

        ui->comm_detailStatusLabel->setText("Waiting for connection " + QString::number(connections) + " of 3");
        ui->comm_mainStatusLabel->setText("Connecting...");
    }
}

void MainWindow::reloadMasterArmClicked() {
    if (_mcNetwork->getRole() == ArmOperatorRole) {
        reinterpret_cast<ArmControlSystem*>(_controlSystem)->reloadMasterArmConfig();
    }
}

void MainWindow::onGamepadChanged(SDL_GameController *controller, QString name) {
    if (controller) {
        ui->hid_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_green_18px.png);");
        ui->hid_inputDeviceLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->hid_inputDeviceLabel->setText(name);
    }
    else {
        ui->hid_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_yellow_18px.png);");
        ui->hid_inputDeviceLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->hid_inputDeviceLabel->setText("No input devices");
    }
}

void MainWindow::onMasterArmStateChanged(bool connected) {
    if (connected) {
        ui->hid_inputDeviceLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->hid_inputDeviceLabel->setText("Master arm connected");
        ui->hid_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_green_18px.png);");
    }
    else {
        ui->masterarm_yawValueLabel->setText("N/A");
        ui->masterarm_shoulderValueLabel->setText("N/A");
        ui->masterarm_elbowValueLabel->setText("N/A");
        ui->masterarm_wristValueLabel->setText("N/A");
        ui->masterarm_bucketToggleValueLabel->setText("N/A");
        ui->masterarm_dumpMacroValueLabel->setText("N/A");
        ui->masterarm_stowMacroValueLabel->setText("N/A");

        ui->hid_inputDeviceLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->hid_inputDeviceLabel->setText("Connecting to master arm...");
        ui->hid_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_yellow_18px.png);");
    }
}

void MainWindow::onMasterArmUpdate(const char *armMessage) {
    ui->masterarm_yawValueLabel->setText(QString::number(ArmMessage::getMasterYaw(armMessage)));
    ui->masterarm_shoulderValueLabel->setText(QString::number(ArmMessage::getMasterShoulder(armMessage)));
    ui->masterarm_elbowValueLabel->setText(QString::number(ArmMessage::getMasterElbow(armMessage)));
    ui->masterarm_wristValueLabel->setText(QString::number(ArmMessage::getMasterWrist(armMessage)));
    ui->masterarm_bucketToggleValueLabel->setText(ArmMessage::getBucketClose(armMessage) ? "CLOSE" : "OPEN");
    ui->masterarm_dumpMacroValueLabel->setText(ArmMessage::getDump(armMessage) ? "ON" : "OFF");
    ui->masterarm_stowMacroValueLabel->setText(ArmMessage::getStow(armMessage) ? "ON" : "OFF");
}

void MainWindow::updateSubsystemStateInformation() {
    switch (_lastArmSubsystemState) {
    case NormalSubsystemState:
        ui->sys_armSubsystemLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->sys_armSubsystemLabel->setText("Arm subsystem normal");
        ui->sys_armSubsystemGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/embedded_board_green_18px.png);");
        break;
    case MalfunctionSubsystemState:
        ui->sys_armSubsystemLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->sys_armSubsystemLabel->setText("Arm subsystem malfunction");
        ui->sys_armSubsystemGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/embedded_board_yellow_18px.png);");
        break;
    case UnknownSubsystemState:
        ui->sys_armSubsystemLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->sys_armSubsystemLabel->setText("Waiting for connection...");
        ui->sys_armSubsystemGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/embedded_board_yellow_18px.png);");
        break;
    }

    switch (_lastDriveCameraSubsystemState) {
    case NormalSubsystemState:
        ui->sys_driveGimbalSubsystemLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->sys_driveGimbalSubsystemLabel->setText("Drive/Camera subsystem normal");
        ui->sys_driveGimbalSubsystemGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/embedded_board_green_18px.png);");
        break;
    case MalfunctionSubsystemState:
        ui->sys_driveGimbalSubsystemLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->sys_driveGimbalSubsystemLabel->setText("Drive/Camera subsystem malfunction");
        ui->sys_driveGimbalSubsystemGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/embedded_board_yellow_18px.png);");
        break;
    case UnknownSubsystemState:
        ui->sys_driveGimbalSubsystemLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->sys_driveGimbalSubsystemLabel->setText("Waiting for connection...");
        ui->sys_driveGimbalSubsystemGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/embedded_board_yellow_18px.png);");
        break;
    }

    switch (_lastSecondaryComputerState) {
    case NormalSubsystemState:
        ui->sys_secondaryComputerLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->sys_secondaryComputerLabel->setText("Secondary computer normal");
        ui->sys_secondaryComputerGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/embedded_board_green_18px.png);");
        break;
    case MalfunctionSubsystemState:
        ui->sys_secondaryComputerLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->sys_secondaryComputerLabel->setText("Secondary computer malfunction");
        ui->sys_secondaryComputerGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/embedded_board_yellow_18px.png);");
        break;
    case UnknownSubsystemState:
        ui->sys_secondaryComputerLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->sys_secondaryComputerLabel->setText("Waiting for connection...");
        ui->sys_secondaryComputerGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/embedded_board_yellow_18px.png);");
        break;
    }
}

bool MainWindow::isMuteAudioSelected() {
    return ui->media_audioControlWidget->isMuted();
}

void MainWindow::camera1ControlOptionChanged(VideoFormat option) {
    cameraControlOptionChanged(0, option);
}

void MainWindow::camera2ControlOptionChanged(VideoFormat option) {
    cameraControlOptionChanged(1, option);
}

void MainWindow::camera3ControlOptionChanged(VideoFormat option) {
    cameraControlOptionChanged(2, option);
}

void MainWindow::camera4ControlOptionChanged(VideoFormat option) {
    cameraControlOptionChanged(3, option);
}

void MainWindow::camera5ControlOptionChanged(VideoFormat option) {
    cameraControlOptionChanged(4, option);
}

void MainWindow::camera1NameEdited(QString newName) {
    emit cameraNameEdited(0, newName);
}

void MainWindow::camera2NameEdited(QString newName) {
    emit cameraNameEdited(1, newName);
}

void MainWindow::camera3NameEdited(QString newName) {
    emit cameraNameEdited(2, newName);
}

void MainWindow::camera4NameEdited(QString newName) {
    emit cameraNameEdited(3, newName);
}

void MainWindow::camera5NameEdited(QString newName) {
    emit cameraNameEdited(4, newName);
}

void MainWindow::cameraControlOptionChanged(int camera, VideoFormat option) {
    emit cameraFormatChanged(camera, option);
}

void MainWindow::setCameraName(int camera, QString name) {
    VideoControlWidget *widget;
    switch (camera) {
    case 0:
        widget = ui->media_camera1ControlWidget;
        break;
    case 1:
        widget = ui->media_camera2ControlWidget;
        break;
    case 2:
        widget = ui->media_camera3ControlWidget;
        break;
    case 3:
        widget = ui->media_camera4ControlWidget;
        break;
    case 4:
        widget = ui->media_camera5ControlWidget;
        break;
    default:
        return;
    }

    widget->setName(name);
}

void MainWindow::onFatalError(QString description) {
    QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",description,
        QMessageBox::Ok, this).exec();
    exit(1);
}

void MainWindow::onWarning(QString description) {
    QMessageBox(QMessageBox::Warning, "Mission Control",description,
        QMessageBox::Ok, this).show(); //do not block
}

void MainWindow::onBitrateUpdate(quint64 bpsRoverDown, quint64 bpsRoverUp) {
    ui->comm_bitrateLabel->setText("Rover ▲ " + formatDataRate(bpsRoverUp, "b/s") + " ▼ " + formatDataRate(bpsRoverDown, "b/s"));
}

void MainWindow::onLocationUpdate(const NmeaMessage &location) {
    ui->googleMapView->updateLocation(location);
    ui->gpsStatusLabel->setText("<html><b>GPS:</b> "
                                + QString::number(location.Satellites) + " Satellites, "
                                + QString::number(location.GroundSpeed) + "kph, "
                                + QString::number(location.Altitude) + "m Elevation");

    START_TIMER(_clearGpsStatusTimerId, 15000);
}

void MainWindow::timerEvent(QTimerEvent *e) {
    QMainWindow::timerEvent(e);
    if (e->timerId() == _clearGpsStatusTimerId) {
        ui->gpsStatusLabel->setText("Waiting for GPS...");
        KILL_TIMER(_clearGpsStatusTimerId);
    }
}

void MainWindow::onControlChannelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    _lastControlChannelState = state;
    updateConnectionStateInformation();
}

void MainWindow::onRoverChannelStateChanged(Channel::State state) {
    _lastRoverChannelState = state;
    updateConnectionStateInformation();
}

void MainWindow::onArmSubsystemStateChanged(RoverSubsystemState state) {
    _lastArmSubsystemState = state;
    updateSubsystemStateInformation();
}

void MainWindow::onDriveCameraSubsystemStateChanged(RoverSubsystemState state) {
    _lastDriveCameraSubsystemState = state;
    updateSubsystemStateInformation();
}

void MainWindow::onSecondaryComputerStateChanged(RoverSubsystemState state) {
    _lastSecondaryComputerState = state;
    updateSubsystemStateInformation();
}

void MainWindow::onRttUpdate(int rtt) {
    _lastRtt = rtt;
    updateConnectionStateInformation();
}

void MainWindow::onDroppedPacketRateUpdate(int droppedRatePercent) {
    _lastDroppedPacketPercent = droppedRatePercent;
    updateConnectionStateInformation();
}

void MainWindow::onCameraFormatChanged(int camera, VideoFormat format) {
    VideoControlWidget *widget;
    switch (camera) {
    case 0:
        widget = ui->media_camera1ControlWidget;
        break;
    case 1:
        widget = ui->media_camera2ControlWidget;
        break;
    case 2:
        widget = ui->media_camera3ControlWidget;
        break;
    case 3:
        widget = ui->media_camera4ControlWidget;
        break;
    case 4:
        widget = ui->media_camera5ControlWidget;
        break;
    default:
        return;
    }

    widget->selectOption(format);
}

void MainWindow::onAudioFormatChanged(AudioFormat format) {
    ui->media_audioControlWidget->selectOption(format);
}


CameraWidget* MainWindow::getTopCameraWidget() {
    return ui->topVideoWidget;
}

CameraWidget* MainWindow::getBottomCameraWidget() {
    return ui->bottomVideoWidget;
}

CameraWidget* MainWindow::getFullscreenCameraWidget() {
    return _videoWindow->getCameraWidget();
}

void MainWindow::resizeEvent(QResizeEvent* event) {
   QMainWindow::resizeEvent(event);
   /// Video on right
   /*ui->infoContainer->resize(width() / 2, ui->infoContainer->height());
   ui->infoContainer->move(0, 0);
   ui->googleMapView->move(0, ui->infoContainer->height());
   ui->statusBarWidget->resize(width() / 2, 30);
   ui->statusBarWidget->move(0, height() - 30);
   ui->googleMapView->resize(width() / 2 + 2,
                             height() - ui->statusBarWidget->height() - ui->infoContainer->height() + 1);
   ui->videoContainer->move(width() / 2, 0);
   ui->videoContainer->resize(width() / 2, height());*/

   /// Video on left
   ui->infoContainer->resize(width() / 2, ui->infoContainer->height());
   ui->infoContainer->move(width() / 2, 0);
   ui->googleMapView->move(width() / 2, ui->infoContainer->height());
   ui->statusBarWidget->resize(width() / 2, 30);
   ui->statusBarWidget->move(width() / 2, height() - 30);
   ui->googleMapView->resize(width() / 2 + 2,
                             height() - ui->statusBarWidget->height() - ui->infoContainer->height() + 1);
   ui->videoContainer->move(0, 0);
   ui->videoContainer->resize(width() / 2, height());
}

void MainWindow::closeEvent(QCloseEvent *e) {
    if (_videoWindow) {
        _videoWindow->close();
    }
    e->accept();
    emit closed();
}

void MainWindow::keyPressEvent(QKeyEvent *e) {
    QMainWindow::keyPressEvent(e);
    if (e->key() == Qt::Key_F11) {
        if (_fullscreen) showNormal(); else showFullScreen();
        _fullscreen = !_fullscreen;
    }
    else if ((e->key() == Qt::Key_D) | (e->key() == Qt::Key_Right)) {
        emit cycleVideosClockwise();
    }
    else if ((e->key() == Qt::Key_A) | (e->key() == Qt::Key_Left)) {
        emit cycleVideosCounterclockwise();
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

}
}
