#include "initwindow.h"
#include "ui_initwindow.h"

//TODO possibly make these configurable
#define GAMEPAD_POLL_INTERVAL 50

#define LOG_TAG "InitWindow"

namespace Soro {
namespace MissionControl {

InitWindow::InitWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InitWindow) {
    ui->setupUi(this);

    connect(ui->armOperatorButton, SIGNAL(clicked(bool)),
            this, SLOT(armOperatorSelected()));
    connect(ui->driverButton, SIGNAL(clicked(bool)),
            this, SLOT(driverSelected()));
    connect(ui->spectatorButton, SIGNAL(clicked(bool)),
            this, SLOT(driverSelected()));
    connect(ui->cameraOperatorButton, SIGNAL(clicked(bool)),
            this, SLOT(cameraOperatorSelected()));

    QTimer::singleShot(1, this, SLOT(init_start()));
}

void InitWindow::init_start() {
    showStatus();
    ui->retryButton->hide();

    setStatusText("Loading configuration");

    if (!Soro::init(&_config, "missioncontrol")) {
        setErrorText("Failed to load/find configuration file");
        disconnect(ui->retryButton, 0, this, 0);
        connect(ui->retryButton, SIGNAL(clicked(bool)),
                this, SLOT(init_start()));
        ui->retryButton->show();
        return;
    }

    init_address();
}

void InitWindow::init_address() {
    ui->retryButton->hide();
    QTcpSocket socket;
    socket.connectToHost("8.8.8.8", 53);
    if (socket.waitForConnected()) {
        ui->addressLabel->setText(socket.localAddress().toString());
    }
    else {
        setErrorText("You are not connected to the internet");
        disconnect(ui->retryButton, 0, this, 0);
        connect(ui->retryButton, SIGNAL(clicked(bool)),
                this, SLOT(init_address()));
        ui->retryButton->show();
        return;
    }

    init_gamepadManager();
}

void InitWindow::init_gamepadManager() {
    ui->retryButton->hide();

    if (!_gamepad) {
        _gamepad = new GamepadManager(this);
    }
    QString err;
    if (!_gamepad->init(GAMEPAD_POLL_INTERVAL, &err)) {
        LOG_E(LOG_TAG, "Failed to load gamepad map file from ./");
        setErrorText(err);
        delete _mcNetwork;
        disconnect(ui->retryButton, 0, this, 0);
        connect(ui->retryButton, SIGNAL(clicked(bool)),
                this, SLOT(init_gamepadManager()));
        ui->retryButton->show();
        return;
    }

    init_mcNetwork();
}

void InitWindow::init_mcNetwork() {
    ui->retryButton->hide();

    setStatusText("Initializing mission control network");

    if (!_mcNetwork) {
        _mcNetwork = new MissionControlNetwork(&_config, this);
        connect(_mcNetwork, SIGNAL(connected(bool)),
                this, SLOT(mcNetworkConnected(bool)));
        connect(_mcNetwork, SIGNAL(disconnected()),
                this, SLOT(mcNetworkDisconnected()));
        connect(_mcNetwork, SIGNAL(roleGranted(Role)),
                this, SLOT(mcNetworkRoleGranted(Role)));
        connect(_mcNetwork, SIGNAL(roleDenied(Role)),
                this, SLOT(mcNetworkRoleDenied()));
    }
    QString err;
    if (!_mcNetwork->init(&err)) {
        setErrorText(err);
        delete _mcNetwork;
        disconnect(ui->retryButton, 0, this, 0);
        connect(ui->retryButton, SIGNAL(clicked(bool)),
                this, SLOT(init_mcNetwork()));
        ui->retryButton->show();
        return;
    }

    setStatusText("Joining mission control network");
    // Wait for singnal from mcNetwork
}

void InitWindow::mcNetworkConnected(bool broker) {
    if (broker) {
        ui->brokerLabel->setText("This mission control is broker");
    }
    else {
        ui->brokerLabel->setText(_mcNetwork->getBrokerAddress().toString());
    }

    showRoleButtons();
}

void InitWindow::mcNetworkRoleGranted(Role role) {
    Q_UNUSED(role);
    init_controlSystem();
}

void InitWindow::mcNetworkRoleDenied() {
    setErrorText("That role is already filled on the mission control network. Select a different role for this mission control.");
    disconnect(ui->retryButton, 0, this, 0);
    connect(ui->retryButton, SIGNAL(clicked(bool)),
            this, SLOT(showRoleButtons()));
    ui->retryButton->show();
}

void InitWindow::armOperatorSelected() {
    showStatus();
    setStatusText("Requesting Arm Operator role");
    _mcNetwork->requestRole(ArmOperatorRole);
}

void InitWindow::driverSelected() {
    showStatus();
    setStatusText("Requesting Driver role");
    _mcNetwork->requestRole(DriverRole);
}

void InitWindow::cameraOperatorSelected() {
    showStatus();
    setStatusText("Requesting Camera Operator role");
    _mcNetwork->requestRole(CameraOperatorRole);
}

void InitWindow::spectatorSelected() {
    showStatus();
    setStatusText("Requesting Spectator role");
    _mcNetwork->requestRole(SpectatorRole);
}

void InitWindow::mcNetworkDisconnected() {
    reset();
    showStatus();
    setErrorText("The connection to the mission control network has unexpectedly closed");
    disconnect(ui->retryButton, 0, this, 0);
    connect(ui->retryButton, SIGNAL(clicked(bool)),
            this, SLOT(init_start()));
    ui->retryButton->show();
}

void InitWindow::init_controlSystem() {
    ui->retryButton->hide();
    setStatusText("Initializing control system");
    switch (_mcNetwork->getRole()) {
    case ArmOperatorRole:
        _controlSystem = new ArmControlSystem(&_config, this);
        break;
    case DriverRole:
        _controlSystem = new DriveControlSystem(&_config, _gamepad, this);
        break;
    case CameraOperatorRole:
        _controlSystem = new CameraControlSystem(&_config, _gamepad, this);
        break;
    default:
        break;
    }

    if (_controlSystem) {
        QString err;
        if (!_controlSystem->init(&err)) {
            setErrorText(err);
            delete _controlSystem;
            disconnect(ui->retryButton, 0, this, 0);
            connect(ui->retryButton, SIGNAL(clicked(bool)),
                    this, SLOT(init_controlSystem()));
            ui->retryButton->show();
            return;
        }
    }

    // Start mission control
    if (_mc) {
        LOG_W(LOG_TAG, "Preparing to start mission control, but it is not null");
        delete _mc;
    }
    _mc = new MissionControlProcess(&_config, _gamepad, _mcNetwork, _controlSystem, this);

    setCompletedText("Mission control is running");
}

void InitWindow::reset() {
    if (_mc) {
        delete _mc;
        _mc = NULL;
    }
    if (_controlSystem) {
        delete _controlSystem;
        _controlSystem = NULL;
    }
    if (_gamepad) {
        delete _gamepad;
        _gamepad = NULL;
    }
    if (_mcNetwork) {
        delete _mcNetwork;
        _mcNetwork = NULL;
    }
}

InitWindow::~InitWindow() {
    delete ui;
    if (_mcNetwork) {
        disconnect(_mcNetwork, 0, 0, 0);
        delete _mcNetwork;
    }
    if (_gamepad) {
        disconnect(_gamepad, 0, 0, 0);
    }
}

void InitWindow::showStatus() {
    ui->statusLabel->show();
    ui->retryButton->hide();
    ui->armOperatorButton->hide();
    ui->driverButton->hide();
    ui->cameraOperatorButton->hide();
    ui->spectatorButton->hide();
}

void InitWindow::showRoleButtons() {
    ui->statusLabel->hide();
    ui->retryButton->hide();
    ui->armOperatorButton->show();
    ui->driverButton->show();
    ui->cameraOperatorButton->show();
    ui->spectatorButton->show();
}

void InitWindow::setStatusText(QString text) {
    ui->statusLabel->setText("<html><body style=\'color:#F57F17;\'><p><b>Starting Up</b></p><p>" + text + "</p></body></html>");
}

void InitWindow::setCompletedText(QString text) {
    ui->statusLabel->setText("<html><body style=\'color:#1B5E20;\'><p><b>All Done</b></p><p>" + text + "</p></body></html>");
}

void InitWindow::setErrorText(QString text) {
    ui->statusLabel->setText("<html><body style=\'color:#b71c1c;\'><p><b>Error</b></p><p>" + text + "</p></body></html>");
}


}
}
