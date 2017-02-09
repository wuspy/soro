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

#include "initwindow.h"
#include "ui_initwindow.h"

#define LOG_TAG "InitWindow"

namespace Soro {
namespace MissionControl {

InitWindow::InitWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InitWindow) {
    ui->setupUi(this);

    connect(ui->armOperatorButton, &QPushButton::clicked, this, &InitWindow::armOperatorSelected);
    connect(ui->driverButton, &QPushButton::clicked, this, &InitWindow::driverSelected);
    connect(ui->spectatorButton, &QPushButton::clicked, this, &InitWindow::spectatorSelected);
    connect(ui->cameraOperatorButton, &QPushButton::clicked, this, &InitWindow::cameraOperatorSelected);
    connect(ui->roverAddressTextBox, &QLineEdit::textChanged, this, &InitWindow::roverAddressTextChanged);

    QTimer::singleShot(1, this, SLOT(init_start()));
}

void InitWindow::closeEvent(QCloseEvent *e) {
    // Exit the application when this window is closed
    if (_mcNetwork && _mcNetwork->isBroker()) {
        QMessageBox::StandardButton resBtn = QMessageBox::warning(this, "Mission Control",
                    "Warning: This mission control is acting as broker, and closing it will reset the entire mission control network.",
                    QMessageBox::Cancel | QMessageBox::Ok,
                    QMessageBox::Ok);
        if (resBtn != QMessageBox::Ok) {
            e->ignore();
        } else {
            QCoreApplication::exit(0);
        }
    }
    else {
        QCoreApplication::exit(0);
    }
}

void InitWindow::roverAddressTextChanged(QString text) {
    if (QRegExp(IPV4_REGEX).exactMatch(text) || QRegExp(IPV6_REGEX).exactMatch(text)) {
        ui->roverAddressStatusLabel->setText("✓");
        _addressValid = true;
    }
    else {
        ui->roverAddressStatusLabel->setText("x");
        _addressValid = false;
    }
}

void InitWindow::init_start() {
    init_address();
}

void InitWindow::init_address() {
    showStatus();
    ui->retryButton->hide();

    setStatusText("Checking for internet connection");
    QTcpSocket socket;
    socket.connectToHost("8.8.8.8", 53);
    if (socket.waitForConnected(3000)) {
        ui->addressLabel->setText(socket.localAddress().toString());
    }
    else {
        ui->addressLabel->setText("No Internet Connection");
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
        LOG_E(LOG_TAG, "Cannot init gamepad: " + err);
        setErrorText(err);
        delete _mcNetwork;
        disconnect(ui->retryButton, 0, this, 0);
        connect(ui->retryButton, &QPushButton::clicked, this, &InitWindow::init_gamepadManager);
        ui->retryButton->show();
        return;
    }

    init_mcNetwork();
}

void InitWindow::init_mcNetwork() {
    ui->retryButton->hide();

    setStatusText("Initializing mission control network");

    if (!_mcNetwork) {
        _mcNetwork = new MissionControlNetwork(this);
        connect(_mcNetwork, &MissionControlNetwork::connected, this, &InitWindow::mcNetworkConnected);
        connect(_mcNetwork, &MissionControlNetwork::disconnected, this, &InitWindow::mcNetworkDisconnected);
        connect(_mcNetwork, &MissionControlNetwork::roleGranted, this, &InitWindow::mcNetworkRoleGranted);
        connect(_mcNetwork, &MissionControlNetwork::roleDenied, this, &InitWindow::mcNetworkRoleDenied);
    }
    QString err;
    if (!_mcNetwork->init(&err)) {
        setErrorText(err);
        delete _mcNetwork;
        disconnect(ui->retryButton, 0, this, 0);
        connect(ui->retryButton, &QPushButton::clicked, this, &InitWindow::init_mcNetwork);
        ui->retryButton->show();
        return;
    }

    setStatusText("Joining mission control network");
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
    showStatus();
    setErrorText("That role is already filled on the mission control network. Select a different role for this mission control.");
    disconnect(ui->retryButton, 0, this, 0);
    connect(ui->retryButton, &QPushButton::clicked, this, &InitWindow::showRoleButtons);
    ui->retryButton->show();
}

void InitWindow::showInvalidAddressError() {
    showStatus();
    setErrorText("Please enter a valid IP address");
    disconnect(ui->retryButton, 0, this, 0);
    connect(ui->retryButton, &QPushButton::clicked, this, &InitWindow::showRoleButtons);
    ui->retryButton->show();
}

void InitWindow::armOperatorSelected() {
    if (!_addressValid) {
        showInvalidAddressError();
        return;
    }
    showStatus();
    setStatusText("Requesting Arm Operator role");
    _mcNetwork->requestRole(ArmOperatorRole);
}

void InitWindow::driverSelected() {
    if (!_addressValid) {
        showInvalidAddressError();
        return;
    }
    showStatus();
    setStatusText("Requesting Driver role");
    _mcNetwork->requestRole(DriverRole);
}

void InitWindow::cameraOperatorSelected() {
    if (!_addressValid) {
        showInvalidAddressError();
        return;
    }
    showStatus();
    setStatusText("Requesting Camera Operator role");
    _mcNetwork->requestRole(CameraOperatorRole);
}

void InitWindow::spectatorSelected() {
    if (!_addressValid) {
        showInvalidAddressError();
        return;
    }
    showStatus();
    setStatusText("Requesting Spectator role");
    _mcNetwork->requestRole(SpectatorRole);
}

void InitWindow::mcNetworkDisconnected() {
    reset();
    showStatus();
    setErrorText("The connection to the mission control network has unexpectedly closed.");
    disconnect(ui->retryButton, 0, this, 0);
    connect(ui->retryButton, &QPushButton::clicked, this, &InitWindow::init_start);
}

void InitWindow::init_controlSystem() {
    ui->retryButton->hide();
    setStatusText("Initializing control system");

    QHostAddress roverAddress = QHostAddress(ui->roverAddressTextBox->text());

    switch (_mcNetwork->getRole()) {
    case ArmOperatorRole:
        _controlSystem = new ArmControlSystem(roverAddress, this);
        break;
    case DriverRole:
        _controlSystem = new DriveControlSystem(roverAddress, _gamepad, this);
        break;
    case CameraOperatorRole:
        _controlSystem = new CameraControlSystem(roverAddress, _gamepad, this);
        break;
    default:
        break;
    }

    if (_controlSystem) {
        QString err;
        if (!_controlSystem->init(&err)) {
            setErrorText(err);
            delete _controlSystem;
            _controlSystem = nullptr;
            disconnect(ui->retryButton, 0, this, 0);
            connect(ui->retryButton, &QPushButton::clicked, this, &InitWindow::showRoleButtons);
            ui->retryButton->show();
            return;
        }
    }

    // Start mission control
    if (_mc) {
        LOG_W(LOG_TAG, "Preparing to start mission control, but it is not null");
        delete _mc;
    }
    _mc = new MissionControlProcess(roverAddress, _gamepad, _mcNetwork, _controlSystem, this);
    connect(_mc, &MissionControlProcess::windowClosed, this, &InitWindow::mcWindowClosed);
    setCompletedText("Mission control is running");
}

void InitWindow::mcWindowClosed() {
    LOG_I(LOG_TAG, "Main window has closed");
    reset();
    init_start();
}

void InitWindow::reset() {
    LOG_I(LOG_TAG, "Resetting mission control");
    if (_mc) {
        delete _mc;
        _mc = nullptr;
    }
    if (_controlSystem) {
        delete _controlSystem;
        _controlSystem = nullptr;
    }
    if (_gamepad) {
        delete _gamepad;
        _gamepad = nullptr;
    }
    if (_mcNetwork) {
        delete _mcNetwork;
        _mcNetwork = nullptr;
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
