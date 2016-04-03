#include "soromainwindow.h"
#include "ui_soromainwindow.h"

#define ARM_CAMERA_DISPLAY_NAME "Arm Camera"
#define DRIVE_CAMERA_DISPLAY_NAME "Drive Camera"
#define GIMBAL_CAMERA_DISPLAY_NAME "Gimbal Camera"

using namespace Soro;
using namespace Soro::MissionControl;

SoroMainWindow::SoroMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SoroMainWindow) {

    ui->setupUi(this);
    addWidgetShadow(ui->statusBarWidget, 10, 0);
    addWidgetShadow(ui->infoContainer, 10, 0);

    _videoWindow = new VideoWindow(this);
    _videoWindow->show();

    _controller = new SoroWindowController(this);
    connect(ui->settingsButton, SIGNAL(clicked(bool)),
            _controller, SLOT(settingsClicked()));
    connect(_controller, SIGNAL(gamepadChanged(QString)),
            this, SLOT(controllerGamepadChanged(QString)), Qt::DirectConnection);
    connect(_controller, SIGNAL(initialized(SoroIniConfig,MissionControlIniConfig)),
            this, SLOT(controllerInitialized(SoroIniConfig,MissionControlIniConfig)), Qt::DirectConnection);
    connect(_controller, SIGNAL(error(QString)),
            this, SLOT(controllerError(QString)), Qt::DirectConnection);
    connect(_controller, SIGNAL(warning(QString)),
            this, SLOT(controllerWarning(QString)), Qt::DirectConnection);
    connect(_controller, SIGNAL(connectionQualityUpdate(int,int)),
            this, SLOT(controllerConnectionQualityUpdate(int,int)));
}

void SoroMainWindow::controllerInitialized(const SoroIniConfig& soroConfig,
                                           const MissionControlIniConfig& mcConfig) {
    switch (mcConfig.Layout) {
    case MissionControlIniConfig::ArmLayoutMode:
        setWindowTitle("Mission Control - Arm");
        ui->videoPane1->setCameraName(DRIVE_CAMERA_DISPLAY_NAME);
        ui->videoPane2->setCameraName(GIMBAL_CAMERA_DISPLAY_NAME);
        _videoWindow->getVideoPane()->setCameraName(ARM_CAMERA_DISPLAY_NAME);
        if (mcConfig.ControlInputMode == MissionControlIniConfig::MasterArm) {
            masterArmSerialStateChanged(SerialChannel3::ConnectingState);
            connect(_controller->getMasterArmSerial(), SIGNAL(stateChanged(SerialChannel3::State)),
                    this, SLOT(masterArmSerialStateChanged(SerialChannel3::State)));
        }
        break;
    case MissionControlIniConfig::DriveLayoutMode:
        setWindowTitle("Mission Control - Driver");
        ui->videoPane1->setCameraName(GIMBAL_CAMERA_DISPLAY_NAME);
        ui->videoPane2->setCameraName(ARM_CAMERA_DISPLAY_NAME);
        _videoWindow->getVideoPane()->setCameraName(DRIVE_CAMERA_DISPLAY_NAME);
        break;
    case MissionControlIniConfig::GimbalLayoutMode:
        setWindowTitle("Mission Control - Gimbal");
        ui->videoPane1->setCameraName(DRIVE_CAMERA_DISPLAY_NAME);
        ui->videoPane2->setCameraName(ARM_CAMERA_DISPLAY_NAME);
        _videoWindow->getVideoPane()->setCameraName(GIMBAL_CAMERA_DISPLAY_NAME);
        break;
    case MissionControlIniConfig::SpectatorLayoutMode:
        setWindowTitle("Mission Control - Spectating");
        ui->videoPane1->setCameraName(DRIVE_CAMERA_DISPLAY_NAME);
        ui->videoPane2->setCameraName(ARM_CAMERA_DISPLAY_NAME);
        _videoWindow->getVideoPane()->setCameraName(GIMBAL_CAMERA_DISPLAY_NAME);
        break;
    }
    sharedChannelStateChanged(Channel::ConnectingState);
    if (_controller->getControlChannel() != NULL) {
        controlChannelStateChanged(Channel::ConnectingState);
    }
    connect(_controller->getSharedChannel(), SIGNAL(stateChanged(Channel::State)),
            this, SLOT(sharedChannelStateChanged(Channel::State)));
    connect (_controller->getSharedChannel(), SIGNAL(statisticsUpdate(int,quint64,quint64,int,int)),
             this, SLOT(sharedChannelStatsUpdate(int,quint64,quint64,int,int)));
    connect (_controller->getControlChannel(), SIGNAL(stateChanged(Channel::State)),
             this, SLOT(controlChannelStateChanged(Channel::State)));
    connect(_controller->getControlChannel(), SIGNAL(statisticsUpdate(int,quint64,quint64,int,int)),
            this, SLOT(controlChannelStatsUpdate(int,quint64,quint64,int,int)));
}

void SoroMainWindow::controllerGamepadChanged(QString name) {
    if (name.isNull()) {
        ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_yellow_18px.png);");
        ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_inputDeviceLabel->setText("No input devices");
    }
    else {
        ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_green_18px.png);");
        ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_inputDeviceLabel->setText(name);
    }
}

void SoroMainWindow::sharedChannelStateChanged(Channel::State state) {
    const Channel *controlChannel = _controller->getControlChannel();
    switch (state) {
    case Channel::ConnectedState:
        ui->comm_sharedStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_sharedStateLabel->setText("Shared link is connected");
        ui->comm_sharedStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/check_circle_green_18px.png);");
        if ((controlChannel == NULL) || (controlChannel->getState() == Channel::ConnectedState)) {
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

void SoroMainWindow::controlChannelStateChanged(Channel::State state) {
    const Channel *sharedChannel = _controller->getSharedChannel();
    switch (state) {
    case Channel::ConnectedState:
        ui->comm_controlStateLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_controlStateLabel->setText("Control link is connected");
        ui->comm_controlStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/check_circle_green_18px.png);");
        if (sharedChannel->getState() == Channel::ConnectedState) {
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

void SoroMainWindow::masterArmSerialStateChanged(SerialChannel3::State state) {
    switch (state) {
    case SerialChannel3::ConnectedState:
        ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_inputDeviceLabel->setText("Master arm connected");
        ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_green_18px.png);");
        break;
    case SerialChannel3::ConnectingState:
        ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_inputDeviceLabel->setText("Connecting to master arm...");
        ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_yellow_18px.png);");
        break;
    }
}

void SoroMainWindow::controlChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                                       int rateUp, int rateDown) {
    if (rtt == -1) {
        ui->comm_mainPingLabel->setText("---");
    }
    else {
        ui->comm_mainPingLabel->setText(QString::number(rtt));
    }
}

void SoroMainWindow::controllerConnectionQualityUpdate(int sharedRtt, int tcpLag) {
    ui->comm_qualityLabel->setText(QString::number(tcpLag));
}

void SoroMainWindow::controllerError(QString description) {
    QMessageBox(QMessageBox::Critical, "WOW VERY ERROR",description,
        QMessageBox::Ok, this).exec();
    exit(1);
}

void SoroMainWindow::controllerWarning(QString description) {
    QMessageBox(QMessageBox::Warning, "Mission Control",description,
        QMessageBox::Ok, this).exec();
}

void SoroMainWindow::timerEvent(QTimerEvent *e) {
    QMainWindow::timerEvent(e);
    //JUST TESTING SHIT
    /*float lat = 29.564844 + qrand() % 1000 * 0.000001;
    float lng = -95.081317 + qrand() % 1000 * 0.000001;
    ui->googleMapView->updateLocation(LatLng(lat, lng));
    ui->googleMapView->updateHeading(rand() % 360);*/
}

void SoroMainWindow::resizeEvent(QResizeEvent* event) {
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

void SoroMainWindow::keyPressEvent(QKeyEvent *e) {
    QMainWindow::keyPressEvent(e);
    if (e->key() == Qt::Key_F11) {
        if (_fullscreen) showNormal(); else showFullScreen();
        _fullscreen = !_fullscreen;
    }
    else {
        _currentKey = QChar(e->key()).toLower().toLatin1();
        _controller->currentKeyChanged(_currentKey);
    }
}

void SoroMainWindow::keyReleaseEvent(QKeyEvent *e) {
    QMainWindow::keyReleaseEvent(e);
    if (QChar(e->key()).toLower().toLatin1() == _currentKey) {
        _currentKey = '\0';
        _controller->currentKeyChanged(_currentKey);
    }
}

SoroMainWindow::~SoroMainWindow() {
    delete ui;
    if (_videoWindow != NULL) delete _videoWindow;
    if (_controller != NULL) delete _controller;
}
