#include "soromainwindow.h"
#include "ui_soromainwindow.h"

#define ARM_CAMERA_DISPLAY_NAME "Arm Camera"
#define DRIVE_CAMERA_DISPLAY_NAME "Drive Camera"
#define GIMBAL_CAMERA_DISPLAY_NAME "Gimbal Camera"

namespace Soro {
namespace MissionControl {

SoroMainWindow::SoroMainWindow(QHostAddress mainHost, QHostAddress videoHost, QHostAddress localLanHost, QHostAddress masterArmHost,
                               bool masterSubnetNode, MissionControlProcess::Role role, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SoroMainWindow) {

    ui->setupUi(this);
    addWidgetShadow(ui->statusBarWidget, 10, 0);
    addWidgetShadow(ui->infoContainer, 10, 0);

    _controller = new MissionControlProcess(mainHost, videoHost, localLanHost, masterArmHost, masterSubnetNode, role, this);
    connect(_controller, SIGNAL(error(QString)),
            this, SLOT(controllerError(QString)), Qt::DirectConnection);
    connect(_controller, SIGNAL(warning(QString)),
            this, SLOT(controllerWarning(QString)), Qt::DirectConnection);
    connect(_controller, SIGNAL(connectionQualityUpdate(int,int)),
            this, SLOT(controllerConnectionQualityUpdate(int,int)));
    connect(_controller, SIGNAL(gamepadChanged(SDL_GameController*)),
            this, SLOT(controllerGamepadChanged(SDL_GameController*)));
    connect(_controller, SIGNAL(arm_masterArmStateChanged(MbedChannel::State)),
            this, SLOT(masterArmStateChanged(MbedChannel::State)));
    connect(_controller, SIGNAL(sharedChannelStateChanged(Channel::State)),
            this, SLOT(sharedChannelStateChanged(Channel::State)));
    connect(_controller, SIGNAL(controlChannelStateChanged(Channel::State)),
            this, SLOT(controlChannelStateChanged(Channel::State)));
    connect(_controller, SIGNAL(controlChannelStatsUpdate(int,quint64,quint64,int,int)),
            this, SLOT(controlChannelStatsUpdate(int,quint64,quint64,int,int)));

    //initialize in the event loop
    START_TIMER(_initTimerId, 1);
}

void SoroMainWindow::controllerGamepadChanged(SDL_GameController *controller) {
    if (controller && SDL_GameControllerGetAttached(controller)) {
        ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_green_18px.png);");
        ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_inputDeviceLabel->setText(SDL_GameControllerName(controller));
    }
    else {
        ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_yellow_18px.png);");
        ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_inputDeviceLabel->setText("No input devices");
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

void SoroMainWindow::masterArmStateChanged(MbedChannel::State state) {
    switch (state) {
    case MbedChannel::ConnectedState:
        ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #1B5E20; }");
        ui->comm_inputDeviceLabel->setText("Master arm connected");
        ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_green_18px.png);");
        break;
    case MbedChannel::ConnectingState:
        ui->comm_inputDeviceLabel->setStyleSheet("QLabel { color : #F57F17; }");
        ui->comm_inputDeviceLabel->setText("Connecting to master arm...");
        ui->comm_inputDeviceGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/gamepad_yellow_18px.png);");
        break;
    }
}

void SoroMainWindow::controlChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                                       int rateUp, int rateDown) {
    if (rtt == -1) {
        ui->comm_mainPingLabel->setText("N/A");
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
        QMessageBox::Ok, this).show(); //do not block
}

void SoroMainWindow::timerEvent(QTimerEvent *e) {
    QMainWindow::timerEvent(e);
    if (e->timerId() == _initTimerId) {
        KILL_TIMER(_initTimerId);
        _controller->init();
        switch (_controller->getRole()) {
        case MissionControlProcess::ArmOperator:
            setWindowTitle("Mission Control - Arm");
            masterArmStateChanged(MbedChannel::ConnectingState);
            controlChannelStateChanged(Channel::ConnectingState);
            controlChannelStatsUpdate(-1, 0, 0, 0, 0);
            break;
        case MissionControlProcess::Driver:
            setWindowTitle("Mission Control - Driver");
            controllerGamepadChanged(NULL);
            controlChannelStateChanged(Channel::ConnectingState);
            controlChannelStatsUpdate(-1, 0, 0, 0, 0);
            break;
        case MissionControlProcess::CameraOperator:
            setWindowTitle("Mission Control - Camera Operator");
            controllerGamepadChanged(NULL);
            controlChannelStateChanged(Channel::ConnectingState);
            controlChannelStatsUpdate(-1, 0, 0, 0, 0);
            break;
        case MissionControlProcess::Spectator:
            setWindowTitle("Mission Control - Spectating");
            ui->comm_controlStateLabel->setText("Not Available");
            ui->comm_controlStateLabel->setStyleSheet("QLabel { color : #F57F17; }");
            ui->comm_controlStateGraphicLabel->setStyleSheet("qproperty-pixmap: url(:/icons/minus_circle_yellow_18px.png);");
            controlChannelStatsUpdate(-1, 0, 0, 0, 0);
            break;
        }
        sharedChannelStateChanged(Channel::ConnectingState);
    }
    //JUST TESTING SHIT
    /*float lat = 29.564844 + qrand() % 1000 * 0.000001;
    float lng = -95.081317 + qrand() % 1000 * 0.000001;
    ui->googleMapView->updateLocation(LatLng(lat, lng));
    ui->googleMapView->updateHeading(rand() % 360);*/
}

void SoroMainWindow::resizeEvent(QResizeEvent* event) {
   QMainWindow::resizeEvent(event);
   ui->infoContainer->resize(width(), ui->infoContainer->height());
   ui->googleMapView->move(0, ui->infoContainer->height());
   ui->statusBarWidget->resize(width(), 30);
   ui->statusBarWidget->move(0, height() - 30);
   ui->googleMapView->resize(width() + 2,
                             height() - ui->statusBarWidget->height() - ui->infoContainer->height() + 1);
}

void SoroMainWindow::keyPressEvent(QKeyEvent *e) {
    QMainWindow::keyPressEvent(e);
    if (e->key() == Qt::Key_F11) {
        if (_fullscreen) showNormal(); else showFullScreen();
        _fullscreen = !_fullscreen;
    }
}

SoroMainWindow::~SoroMainWindow() {
    delete ui;
    delete _controller;
}

}
}
