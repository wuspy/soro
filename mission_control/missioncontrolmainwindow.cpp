/*
 * Copyright 2016 The University of Oklahoma.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "missioncontrolmainwindow.h"
#include "ui_missioncontrolmainwindow.h"
#include "libsoromc/util.h"

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

const QString MissionControlMainWindow::_logLevelFormattersHTML[4] = {
    "<div style=\"color:#b71c1c\">%1&emsp;E/<i>%2</i>:&emsp;%3</div>",
    "<div style=\"color:#0d47a1\">%1&emsp;W/<i>%2</i>:&emsp;%3</div>",
    "<div>%1&emsp;I/<i>%2</i>:&emsp;%3</div>",
    "<div style=\"color:#dddddd\">%1&emsp;D/<i>%2</i>:&emsp;%3</div>"
};

MissionControlMainWindow::MissionControlMainWindow(GamepadManager *gamepad, MissionControlNetwork *mcNetwork, ControlSystem *controlSystem, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MissionControlMainWindow) {

    ui->setupUi(this);
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

    connect(ui->media_audioControlWidget, &AudioControlWidget::playSelected, this, &MissionControlMainWindow::playAudioSelected);
    connect(ui->media_audioControlWidget, &AudioControlWidget::stopSelected, this, &MissionControlMainWindow::stopAudioSelected);
    connect(ui->media_audioControlWidget, &AudioControlWidget::muteToggled, this, &MissionControlMainWindow::audioStreamMuteChanged);
    connect(ui->media_camera1ControlWidget, &VideoControlWidget::optionSelected, this, &MissionControlMainWindow::camera1ControlOptionChanged);
    connect(ui->media_camera2ControlWidget, &VideoControlWidget::optionSelected, this, &MissionControlMainWindow::camera2ControlOptionChanged);
    connect(ui->media_camera3ControlWidget, &VideoControlWidget::optionSelected, this, &MissionControlMainWindow::camera3ControlOptionChanged);
    connect(ui->media_camera4ControlWidget, &VideoControlWidget::optionSelected, this, &MissionControlMainWindow::camera4ControlOptionChanged);
    connect(ui->media_camera5ControlWidget, &VideoControlWidget::optionSelected, this, &MissionControlMainWindow::camera5ControlOptionChanged);

    connect(ui->media_camera1ControlWidget, &VideoControlWidget::userEditedName, this, &MissionControlMainWindow::camera1NameEdited);
    connect(ui->media_camera2ControlWidget, &VideoControlWidget::userEditedName, this, &MissionControlMainWindow::camera2NameEdited);
    connect(ui->media_camera3ControlWidget, &VideoControlWidget::userEditedName, this, &MissionControlMainWindow::camera3NameEdited);
    connect(ui->media_camera4ControlWidget, &VideoControlWidget::userEditedName, this, &MissionControlMainWindow::camera4NameEdited);
    connect(ui->media_camera5ControlWidget, &VideoControlWidget::userEditedName, this, &MissionControlMainWindow::camera5NameEdited);

    connect(ui->masterarm_reloadFileButton, &QPushButton::clicked, this, &MissionControlMainWindow::reloadMasterArmClicked);

    addWidgetShadow(ui->statusBarWidget, 10, 0);
    addWidgetShadow(ui->infoContainer, 10, 0);
    addWidgetShadow(ui->videoContainer, 10, 0);

    switch (_mcNetwork->getRole()) {
    case ArmOperatorRole: {
        ArmControlSystem *armControlSystem = reinterpret_cast<ArmControlSystem*>(_controlSystem);
        ui->statusLabel->setText("<html>Arm Operator" + QString(_mcNetwork->isBroker() ? " <span style=\"color:#b71c1c\"><b>[BROKER]</b></span>" : "") + "</html>");
        connect(armControlSystem, &ArmControlSystem::masterArmStateChanged, this, &MissionControlMainWindow::onMasterArmStateChanged);
        connect(armControlSystem, &ArmControlSystem::masterArmUpdate, this, &MissionControlMainWindow::onMasterArmUpdate);
        onMasterArmStateChanged(armControlSystem->isMasterArmConnected());
    }
        break;
    case DriverRole:
        ui->statusLabel->setText("<html>Driver" + QString(_mcNetwork->isBroker() ? " <span style=\"color:#b71c1c\"><b>[BROKER]</b></span>" : "") + "</html>");
        onGamepadChanged(_gamepad->getGamepad() != nullptr, _gamepad->getGamepadName());
        break;
    case CameraOperatorRole:
        ui->statusLabel->setText("<html>Camera Operator" + QString(_mcNetwork->isBroker() ? " <span style=\"color:#b71c1c\"><b>[BROKER]</b></span>" : "") + "</html>");
        onGamepadChanged(_gamepad->getGamepad() != nullptr, _gamepad->getGamepadName());
        break;
    default:
        ui->statusLabel->setText("<html>Spectator" + QString(_mcNetwork->isBroker() ? " <span style=\"color:#b71c1c\"><b>[BROKER]</b></span>" : "") + "</html>");
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

    connect(_gamepad, &GamepadManager::gamepadChanged, this, &MissionControlMainWindow::onGamepadChanged);
    if (_controlSystem) {
        connect(_controlSystem->getChannel(), &Channel::stateChanged, this, &MissionControlMainWindow::onControlChannelStateChanged);
    }

    onArmSubsystemStateChanged(UnknownSubsystemState);
    onDriveCameraSubsystemStateChanged(UnknownSubsystemState);
    onSecondaryComputerStateChanged(UnknownSubsystemState);
}

void MissionControlMainWindow::setAvailableVideoFormats(QList<VideoFormat> formats) {
    ui->media_camera1ControlWidget->setFormats(formats);
    ui->media_camera2ControlWidget->setFormats(formats);
    ui->media_camera3ControlWidget->setFormats(formats);
    ui->media_camera4ControlWidget->setFormats(formats);
    ui->media_camera5ControlWidget->setFormats(formats);
}

void MissionControlMainWindow::updateConnectionStateInformation() {
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
        ui->comm_mainStatusGraphicLabel->setMovie(nullptr);
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

void MissionControlMainWindow::reloadMasterArmClicked() {
    if (_mcNetwork->getRole() == ArmOperatorRole) {
        reinterpret_cast<ArmControlSystem*>(_controlSystem)->reloadMasterArmConfig();
    }
}

void MissionControlMainWindow::onGamepadChanged(bool connected, QString name) {
    if (connected) {
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

void MissionControlMainWindow::onMasterArmStateChanged(bool connected) {
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

void MissionControlMainWindow::onMasterArmUpdate(const char *armMessage) {
    ui->masterarm_yawValueLabel->setText(QString::number(ArmMessage::getMasterYaw(armMessage)));
    ui->masterarm_shoulderValueLabel->setText(QString::number(ArmMessage::getMasterShoulder(armMessage)));
    ui->masterarm_elbowValueLabel->setText(QString::number(ArmMessage::getMasterElbow(armMessage)));
    ui->masterarm_wristValueLabel->setText(QString::number(ArmMessage::getMasterWrist(armMessage)));
    ui->masterarm_bucketToggleValueLabel->setText(ArmMessage::getBucketClose(armMessage) ? "CLOSE" : "OPEN");
    ui->masterarm_dumpMacroValueLabel->setText(ArmMessage::getDump(armMessage) ? "ON" : "OFF");
    ui->masterarm_stowMacroValueLabel->setText(ArmMessage::getStow(armMessage) ? "ON" : "OFF");
}

void MissionControlMainWindow::updateSubsystemStateInformation() {
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

bool MissionControlMainWindow::isMuteAudioSelected() const {
    return ui->media_audioControlWidget->isMuted();
}

void MissionControlMainWindow::camera1ControlOptionChanged(int formatIndex) {
    cameraControlOptionChanged(0, formatIndex);
}

void MissionControlMainWindow::camera2ControlOptionChanged(int formatIndex) {
    cameraControlOptionChanged(1, formatIndex);
}

void MissionControlMainWindow::camera3ControlOptionChanged(int formatIndex) {
    cameraControlOptionChanged(2, formatIndex);
}

void MissionControlMainWindow::camera4ControlOptionChanged(int formatIndex) {
    cameraControlOptionChanged(3, formatIndex);
}

void MissionControlMainWindow::camera5ControlOptionChanged(int formatIndex) {
    cameraControlOptionChanged(4, formatIndex);
}

void MissionControlMainWindow::camera1NameEdited(QString newName) {
    emit cameraNameEdited(0, newName);
}

void MissionControlMainWindow::camera2NameEdited(QString newName) {
    emit cameraNameEdited(1, newName);
}

void MissionControlMainWindow::camera3NameEdited(QString newName) {
    emit cameraNameEdited(2, newName);
}

void MissionControlMainWindow::camera4NameEdited(QString newName) {
    emit cameraNameEdited(3, newName);
}

void MissionControlMainWindow::camera5NameEdited(QString newName) {
    emit cameraNameEdited(4, newName);
}

void MissionControlMainWindow::cameraControlOptionChanged(int camera, int formatIndex) {
    emit cameraFormatChanged(camera, formatIndex);
}

void MissionControlMainWindow::setCameraName(int camera, QString name) {
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

void MissionControlMainWindow::onFatalError(QString description) {
    QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",description,
        QMessageBox::Ok, this).exec();
    exit(1);
}

void MissionControlMainWindow::onWarning(QString description) {
    QMessageBox(QMessageBox::Warning, "Mission Control",description,
        QMessageBox::Ok, this).show(); //do not block
}

void MissionControlMainWindow::onBitrateUpdate(quint64 bpsRoverDown, quint64 bpsRoverUp) {
    ui->comm_bitrateLabel->setText("Rover ▲ " + formatDataRate(bpsRoverUp, "b/s") + " ▼ " + formatDataRate(bpsRoverDown, "b/s"));
}

void MissionControlMainWindow::onLocationUpdate(const NmeaMessage &location) {
    ui->googleMapView->updateLocation(location);
    ui->gpsStatusLabel->setText("<html><b>GPS:</b> "
                                + QString::number(location.Satellites) + " Satellites, "
                                + QString::number(location.GroundSpeed) + "kph, "
                                + QString::number(location.Altitude) + "m Elevation");

    START_TIMER(_clearGpsStatusTimerId, 15000);
}

void MissionControlMainWindow::timerEvent(QTimerEvent *e) {
    QMainWindow::timerEvent(e);
    if (e->timerId() == _clearGpsStatusTimerId) {
        ui->gpsStatusLabel->setText("Waiting for GPS...");
        KILL_TIMER(_clearGpsStatusTimerId);
    }
}

void MissionControlMainWindow::onControlChannelStateChanged(Channel::State state) {
    _lastControlChannelState = state;
    updateConnectionStateInformation();
}

void MissionControlMainWindow::onRoverChannelStateChanged(Channel::State state) {
    _lastRoverChannelState = state;
    updateConnectionStateInformation();
}

void MissionControlMainWindow::onArmSubsystemStateChanged(RoverSubsystemState state) {
    _lastArmSubsystemState = state;
    updateSubsystemStateInformation();
}

void MissionControlMainWindow::onDriveCameraSubsystemStateChanged(RoverSubsystemState state) {
    _lastDriveCameraSubsystemState = state;
    updateSubsystemStateInformation();
}

void MissionControlMainWindow::onSecondaryComputerStateChanged(RoverSubsystemState state) {
    _lastSecondaryComputerState = state;
    updateSubsystemStateInformation();
}

void MissionControlMainWindow::onRttUpdate(int rtt) {
    _lastRtt = rtt;
    updateConnectionStateInformation();
}

void MissionControlMainWindow::onDroppedPacketRateUpdate(int droppedRatePercent) {
    _lastDroppedPacketPercent = droppedRatePercent;
    updateConnectionStateInformation();
}

void MissionControlMainWindow::onCameraFormatChanged(int camera, int formatIndex) {
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

    widget->selectOption(formatIndex);
}

void MissionControlMainWindow::onAudioPlaying() {
    ui->media_audioControlWidget->playSelected();
}

void MissionControlMainWindow::onAudioStopped() {
    ui->media_audioControlWidget->stopSelected();
}

CameraWidget* MissionControlMainWindow::getTopCameraWidget() {
    return ui->topVideoWidget;
}

CameraWidget* MissionControlMainWindow::getBottomCameraWidget() {
    return ui->bottomVideoWidget;
}

CameraWidget* MissionControlMainWindow::getFullscreenCameraWidget() {
    return _videoWindow->getCameraWidget();
}

void MissionControlMainWindow::resizeEvent(QResizeEvent* event) {
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

void MissionControlMainWindow::closeEvent(QCloseEvent *e) {
    if (_videoWindow) {
        _videoWindow->close();
    }
    e->accept();
    emit closed();
}

void MissionControlMainWindow::keyPressEvent(QKeyEvent *e) {
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

MissionControlMainWindow::~MissionControlMainWindow() {
    delete ui;
}

}
}
