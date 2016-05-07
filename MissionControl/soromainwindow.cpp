#include "soromainwindow.h"
#include "ui_soromainwindow.h"

#define ARM_CAMERA_DISPLAY_NAME "Arm Camera"
#define DRIVE_CAMERA_DISPLAY_NAME "Drive Camera"
#define GIMBAL_CAMERA_DISPLAY_NAME "Gimbal Camera"

namespace Soro {
namespace MissionControl {

SoroMainWindow::SoroMainWindow(QString name, bool masterSubnetNode, MissionControlProcess::Role role, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SoroMainWindow) {

    ui->setupUi(this);
    _videoWindow = new CameraWindow(this);
    _videoWindow->show();

    addWidgetShadow(ui->statusBarWidget, 10, 0);
    addWidgetShadow(ui->infoContainer, 10, 0);
    addWidgetShadow(ui->videoContainer, 10, 0);

    _controller = new MissionControlProcess(name, ui->topVideoWidget, ui->bottomVideoWidget, _videoWindow->getCameraWidget(), masterSubnetNode, role, this);
    connect(_controller, SIGNAL(fatalError(QString)),
            this, SLOT(onFatalError(QString)), Qt::DirectConnection);
    connect(_controller, SIGNAL(warning(QString)),
            this, SLOT(onWarning(QString)), Qt::DirectConnection);
    connect(_controller, SIGNAL(gamepadChanged(SDL_GameController*)),
            this, SLOT(onGamepadChanged(SDL_GameController*)));
    connect(_controller, SIGNAL(arm_masterArmStateChanged(MbedChannel*,MbedChannel::State)),
            this, SLOT(arm_onMasterArmStateChanged(MbedChannel*, MbedChannel::State)));
    connect(_controller, SIGNAL(connectionStateChanged(Channel::State,Channel::State,Channel::State)),
            this, SLOT(onConnectionStateChanged(Channel::State,Channel::State,Channel::State)));
    connect(_controller, SIGNAL(roverCameraUpdate(RoverCameraState,RoverCameraState,RoverCameraState,RoverCameraState,RoverCameraState)),
            this, SLOT(onRoverCameraUpdate(RoverCameraState,RoverCameraState,RoverCameraState,RoverCameraState,RoverCameraState)));
    connect(_controller, SIGNAL(rttUpdate(int)),
            this, SLOT(onRttUpdate(int)));
    connect(_controller, SIGNAL(droppedPacketRateUpdate(int)),
            this, SLOT(onDroppedPacketRateUpdate(int)));
    connect(_controller, SIGNAL(notification(MissionControlProcess::NotificationType,QString,QString)),
            this, SLOT(onNotification(MissionControlProcess::NotificationType,QString,QString)));
    connect(_controller, SIGNAL(roverSystemStateUpdate(RoverSubsystemState,RoverSubsystemState,RoverSubsystemState)),
            this, SLOT(onRoverSystemStateUpdate(RoverSubsystemState,RoverSubsystemState,RoverSubsystemState)));
    //initialize in the event loop
    START_TIMER(_initTimerId, 1);
}

void SoroMainWindow::onGamepadChanged(SDL_GameController *controller) {
    if (controller && SDL_GameControllerGetAttached(controller)) {
        ui->hid_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_green_18px.png);");
        ui->hid_inputDeviceLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->hid_inputDeviceLabel->setText(SDL_GameControllerName(controller));
    }
    else {
        ui->hid_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_yellow_18px.png);");
        ui->hid_inputDeviceLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->hid_inputDeviceLabel->setText("No input devices");
    }
}

void SoroMainWindow::onConnectionStateChanged(Channel::State controlChannelState, Channel::State mccNetworkState, Channel::State sharedChannelState) {
    // update control channel state UI
    switch (controlChannelState) {
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

    // update MCC network state UI
    switch (mccNetworkState) {
    case Channel::ConnectedState:
        ui->comm_mccStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_mccStateLabel->setText("Connected to MCC network");
        ui->comm_mccStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/check_circle_green_18px.png);");
        break;
    case Channel::ErrorState:
        QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",
                    "The mission control center network channel experienced a fatal error. This is most likely due to a configuration problem.",
                    QMessageBox::Ok, this).exec();
        exit(1);
        return;
    default:
        ui->comm_mccStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_mccStateLabel->setText("Connecting to MCC network...");
        ui->comm_mccStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/minus_circle_yellow_18px.png);");
        break;
    }

    // update shared network state UI
    switch (sharedChannelState) {
    case Channel::ConnectedState:
        ui->comm_sharedStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_sharedStateLabel->setText("Connected to Shared Link");
        ui->comm_sharedStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/check_circle_green_18px.png);");
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
        break;
    }

    // update main status label
    if (((controlChannelState == Channel::ConnectedState) || (_controller->getRole() == MissionControlProcess::SpectatorRole))
            && (mccNetworkState == Channel::ConnectedState)
            && (sharedChannelState == Channel::ConnectedState)) {
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_mainStateLabel->setText("Connected");
    }
    else {
        ui->comm_mainStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_mainStateLabel->setText("Connecting...");
    }
}

void SoroMainWindow::arm_onMasterArmStateChanged(MbedChannel *channel, MbedChannel::State state) {
    Q_UNUSED(channel);
    switch (state) {
    case MbedChannel::ConnectedState:
        ui->hid_inputDeviceLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->hid_inputDeviceLabel->setText("Master arm connected");
        ui->hid_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_green_18px.png);");
        break;
    case MbedChannel::ConnectingState:
        ui->hid_inputDeviceLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->hid_inputDeviceLabel->setText("Connecting to master arm...");
        ui->hid_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_yellow_18px.png);");
        break;
    }
}

void SoroMainWindow::onRttUpdate(int rtt) {
    if (rtt == -1) {
        ui->comm_mainPingLabel->setText("N/A");
    }
    else {
        ui->comm_mainPingLabel->setText(QString::number(rtt));
    }
}

void SoroMainWindow::onDroppedPacketRateUpdate(int droppedRatePercent) {
    ui->comm_droppedPacketsLabel->setText(QString::number(droppedRatePercent) + "%");
}

void SoroMainWindow::onRoverSystemStateUpdate(RoverSubsystemState armSystemState,
                                              RoverSubsystemState driveCameraSystemState,
                                              RoverSubsystemState secondaryComputerState) {
    switch (armSystemState) {
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

    switch (driveCameraSystemState) {
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

    switch (secondaryComputerState) {
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

void SoroMainWindow::onRoverCameraUpdate(RoverCameraState camera1State, RoverCameraState camera2State, RoverCameraState camera3State,
                                         RoverCameraState camera4State, RoverCameraState camera5State) {
    switch (camera1State) {
    case StreamingCameraState:
        ui->video_camera1Label->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->video_camera1Label->setText("Camera 1 is streaming");
        ui->video_camera1GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_green_18px.png);");
        break;
    case DisabledCameraState:
        ui->video_camera1Label->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->video_camera1Label->setText("Camera 1 is disabled");
        ui->video_camera1GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_green_18px.png);");
        break;
    case UnavailableCameraState:
        ui->video_camera1Label->setStyleSheet("QLabel { color : black; }");
        ui->video_camera1Label->setText("Camera 1 is disconnected");
        ui->video_camera1GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_black_18px.png);");
        break;
    }

    switch (camera2State) {
    case StreamingCameraState:
        ui->video_camera2Label->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->video_camera2Label->setText("Camera 2 is streaming");
        ui->video_camera2GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_green_18px.png);");
        break;
    case DisabledCameraState:
        ui->video_camera2Label->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->video_camera2Label->setText("Camera 2 is disabled");
        ui->video_camera2GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_green_18px.png);");
        break;
    case UnavailableCameraState:
        ui->video_camera2Label->setStyleSheet("QLabel { color : black; }");
        ui->video_camera2Label->setText("Camera 2 is disconnected");
        ui->video_camera2GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_black_18px.png);");
        break;
    }

    switch (camera3State) {
    case StreamingCameraState:
        ui->video_camera3Label->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->video_camera3Label->setText("Camera 3 is streaming");
        ui->video_camera3GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_green_18px.png);");
        break;
    case DisabledCameraState:
        ui->video_camera3Label->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->video_camera3Label->setText("Camera 3 is disabled");
        ui->video_camera3GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_green_18px.png);");
        break;
    case UnavailableCameraState:
        ui->video_camera3Label->setStyleSheet("QLabel { color : black; }");
        ui->video_camera3Label->setText("Camera 3 is disconnected");
        ui->video_camera3GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_black_18px.png);");
        break;
    }

    switch (camera4State) {
    case StreamingCameraState:
        ui->video_camera4Label->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->video_camera4Label->setText("Camera 4 is streaming");
        ui->video_camera4GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_green_18px.png);");
        break;
    case DisabledCameraState:
        ui->video_camera4Label->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->video_camera4Label->setText("Camera 4 is disabled");
        ui->video_camera4GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_green_18px.png);");
        break;
    case UnavailableCameraState:
        ui->video_camera4Label->setStyleSheet("QLabel { color : black; }");
        ui->video_camera4Label->setText("Camera 4 is disconnected");
        ui->video_camera4GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_black_18px.png);");
        break;
    }

    switch (camera5State) {
    case StreamingCameraState:
        ui->video_camera5Label->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->video_camera5Label->setText("Camera 5 is streaming");
        ui->video_camera5GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_green_18px.png);");
        break;
    case DisabledCameraState:
        ui->video_camera5Label->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->video_camera5Label->setText("Camera 5 is disabled");
        ui->video_camera5GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_green_18px.png);");
        break;
    case UnavailableCameraState:
        ui->video_camera5Label->setStyleSheet("QLabel { color : black; }");
        ui->video_camera5Label->setText("Camera 5 is disconnected");
        ui->video_camera5GraphicsLabel->setStyleSheet("qproperty-pixmap: url(:/icons/camera_black_18px.png);");
        break;
    }
}



void SoroMainWindow::onNotification(MissionControlProcess::NotificationType type, QString sender, QString message) {

}

void SoroMainWindow::onFatalError(QString description) {
    QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",description,
        QMessageBox::Ok, this).exec();
    exit(1);
}

void SoroMainWindow::onWarning(QString description) {
    QMessageBox(QMessageBox::Warning, "Mission Control",description,
        QMessageBox::Ok, this).show(); //do not block
}

void SoroMainWindow::timerEvent(QTimerEvent *e) {
    QMainWindow::timerEvent(e);
    if (e->timerId() == _initTimerId) {
        KILL_TIMER(_initTimerId);
        _controller->init();
        setWindowTitle("Mission Control");
        switch (_controller->getRole()) {
        case MissionControlProcess::ArmOperatorRole:
            ui->statusLabel->setText(_controller->getName() + " (Arm Operator)" + (_controller->isMasterSubnetNode() ? " [MASTER MCC]" : ""));
            arm_onMasterArmStateChanged(NULL, MbedChannel::ConnectingState);
            onRttUpdate(-1);
            break;
        case MissionControlProcess::DriverRole:
            ui->statusLabel->setText(_controller->getName() + " (Driver)" + (_controller->isMasterSubnetNode() ? " [MASTER MCC]" : ""));
            onGamepadChanged(NULL);
            onRttUpdate(-1);
            break;
        case MissionControlProcess::CameraOperatorRole:
            ui->statusLabel->setText(_controller->getName() + " (Camera Operator)" + (_controller->isMasterSubnetNode() ? " [MASTER MCC]" : ""));
            onGamepadChanged(NULL);
            onRttUpdate(-1);
            break;
        case MissionControlProcess::SpectatorRole:
            ui->statusLabel->setText(_controller->getName() + " (Spectator)" + (_controller->isMasterSubnetNode() ? " [MASTER MCC]" : ""));
            ui->comm_controlStateLabel->setText("Not Available");
            ui->comm_controlStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
            ui->comm_controlStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/minus_circle_yellow_18px.png);");
            break;
        }

        // initialize the UI by invoking listeners on initial values
        onConnectionStateChanged(Channel::ConnectingState,
                                 _controller->isMasterSubnetNode() ? Channel::ConnectedState : Channel::ConnectingState,
                                 Channel::ConnectingState);
        onRoverSystemStateUpdate(UnknownSubsystemState, UnknownSubsystemState, UnknownSubsystemState);
        onRoverCameraUpdate(UnavailableCameraState, UnavailableCameraState, UnavailableCameraState, UnavailableCameraState, UnavailableCameraState);
    }
    //JUST TESTING SHIT
    /*float lat = 29.564844 + qrand() % 1000 * 0.000001;
    float lng = -95.081317 + qrand() % 1000 * 0.000001;
    ui->googleMapView->updateLocation(LatLng(lat, lng));
    ui->googleMapView->updateHeading(rand() % 360);*/
}

void SoroMainWindow::resizeEvent(QResizeEvent* event) {
   QMainWindow::resizeEvent(event);
   ui->infoContainer->resize(width() / 2, ui->infoContainer->height());
   ui->googleMapView->move(0, ui->infoContainer->height());
   ui->statusBarWidget->resize(width() / 2, 30);
   ui->statusBarWidget->move(0, height() - 30);
   ui->googleMapView->resize(width() / 2 + 2,
                             height() - ui->statusBarWidget->height() - ui->infoContainer->height() + 1);
   ui->videoContainer->move(width() / 2, 0);
   ui->videoContainer->resize(width() / 2, height());
}

void SoroMainWindow::keyPressEvent(QKeyEvent *e) {
    QMainWindow::keyPressEvent(e);
    if (e->key() == Qt::Key_F11) {
        if (_fullscreen) showNormal(); else showFullScreen();
        _fullscreen = !_fullscreen;
    }
    else if (e->key() == Qt::Key_D) {
        _controller->cycleVideosClockwise();
    }
    else if (e->key() == Qt::Key_A) {
        _controller->cycleVideosCounterClockwise();
    }
}

SoroMainWindow::~SoroMainWindow() {
    delete ui;
    delete _controller;
}

}
}
