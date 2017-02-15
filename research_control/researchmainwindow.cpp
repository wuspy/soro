/*
 * Copyright 2017 The University of Oklahoma.
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

#include "researchmainwindow.h"
#include "ui_researchmainwindow.h"
#include "libsoro/gamepadutil.h"
#include "libsoro/sensordatarecorder.h"

namespace Soro {
namespace MissionControl {

ResearchMainWindow::ResearchMainWindow(const GamepadManager *gamepad, const DriveControlSystem *driveSystem, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ResearchMainWindow)
{
    ui->setupUi(this);
    // These are always half-width because they are only shown in stereo mode
    ui->hudPower2->rootObject()->setProperty("halfWidth", true);
    ui->hudLatency2->rootObject()->setProperty("halfWidth", true);
    ui->hudOrientation2->rootObject()->setProperty("halfWidth", true);

    // Make the embedded quick widgets transparent
    ui->hudPower->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudPower->setClearColor(Qt::transparent);
    ui->hudPower2->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudPower2->setClearColor(Qt::transparent);
    ui->hudLatency->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudLatency->setClearColor(Qt::transparent);
    ui->hudLatency2->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudLatency2->setClearColor(Qt::transparent);
    ui->hudOrientation->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudOrientation->setClearColor(Qt::transparent);
    ui->hudOrientation2->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudOrientation2->setClearColor(Qt::transparent);

    _gamepad = gamepad;
    _driveSystem = driveSystem;
    connect(ui->cameraWidget, &StereoCameraWidget::videoChanged, this, &ResearchMainWindow::adjustHud);
    connect(_gamepad, &GamepadManager::poll, this, &ResearchMainWindow::gamepadPoll);

    START_TIMER(_updateLatencyTimerId, 500);
    START_TIMER(_resizeTimerId, 1000);

    // Setup initial HUD config
    adjustHud();
}

ResearchMainWindow::~ResearchMainWindow() {
    delete ui;
}

StereoCameraWidget* ResearchMainWindow::getCameraWidget() {
    return ui->cameraWidget;
}

void ResearchMainWindow::closeEvent(QCloseEvent *event) {
    event->setAccepted(false);
    emit closed();
}

void ResearchMainWindow::gamepadPoll() {
    float x = 0, y = 0;
    switch (_driveSystem->getMode()) {
    case DriveGamepadMode::SingleStickDrive:
        x = GamepadUtil::axisShortToAxisFloat(_gamepad->axisLeftX);
        y = GamepadUtil::axisShortToAxisFloat(_gamepad->axisLeftY);
        break;
    case DriveGamepadMode::DualStickDrive:
        // TODO Unsupported currently, this is placeholder code
        x = GamepadUtil::axisShortToAxisFloat(_gamepad->axisLeftX);
        y = GamepadUtil::axisShortToAxisFloat(_gamepad->axisLeftY);
        break;
    default: break;
    }

    ui->hudLatency->rootObject()->setProperty("xValue", x);
    ui->hudLatency->rootObject()->setProperty("yValue", y);
    ui->hudLatency2->rootObject()->setProperty("xValue", x);
    ui->hudLatency2->rootObject()->setProperty("yValue", y);
}

void ResearchMainWindow::sensorUpdate(char tag, int value) {
    switch (tag) {
    case SensorDataRecorder::DATATAG_WHEELDATA_A:
        ui->hudPower->rootObject()->setProperty("wheelMLPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelMLPower", value);
        break;
    case SensorDataRecorder::DATATAG_WHEELDATA_B:
        ui->hudPower->rootObject()->setProperty("wheelFLPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelFLPower", value);
        break;
    case SensorDataRecorder::DATATAG_WHEELDATA_C:
        ui->hudPower->rootObject()->setProperty("wheelFRPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelFRPower", value);
        break;
    case SensorDataRecorder::DATATAG_WHEELDATA_D:
        ui->hudPower->rootObject()->setProperty("wheelMRPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelMRPower", value);
        break;
    case SensorDataRecorder::DATATAG_WHEELDATA_E:
        ui->hudPower->rootObject()->setProperty("wheelBRPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelBRPower", value);
        break;
    case SensorDataRecorder::DATATAG_WHEELDATA_F:
        ui->hudPower->rootObject()->setProperty("wheelBLPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelBLPower", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_1_X:
        ui->hudOrientation->rootObject()->setProperty("imu1X", value);
        ui->hudOrientation2->rootObject()->setProperty("imu1X", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_1_Y:
        ui->hudOrientation->rootObject()->setProperty("imu1Y", value);
        ui->hudOrientation2->rootObject()->setProperty("imu1Y", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_1_Z:
        ui->hudOrientation->rootObject()->setProperty("imu1Z", value);
        ui->hudOrientation2->rootObject()->setProperty("imu1Z", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_2_X:
        ui->hudOrientation->rootObject()->setProperty("imu2X", value);
        ui->hudOrientation2->rootObject()->setProperty("imu2X", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_2_Y:
        ui->hudOrientation->rootObject()->setProperty("imu2Y", value);
        ui->hudOrientation2->rootObject()->setProperty("imu2Y", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_2_Z:
        ui->hudOrientation->rootObject()->setProperty("imu2Z", value);
        ui->hudOrientation2->rootObject()->setProperty("imu2Z", value);
        break;
    }
}

void ResearchMainWindow::adjustHud() {
    if (_hudVisible) {
        ui->hudPower->show();
        ui->hudLatency->show();
        ui->hudOrientation->show();
        if (ui->cameraWidget->isStereoOn()) {
            ui->hudPower2->show();
            ui->hudLatency2->show();
            ui->hudOrientation2->show();
            ui->hudPower->rootObject()->setProperty("halfWidth", true);
            ui->hudLatency->rootObject()->setProperty("halfWidth", true);
            ui->hudOrientation->rootObject()->setProperty("halfWidth", true);
        }
        else {
            ui->hudPower2->hide();
            ui->hudLatency2->hide();
            ui->hudOrientation2->hide();
            ui->hudPower->rootObject()->setProperty("halfWidth", false);
            ui->hudLatency->rootObject()->setProperty("halfWidth", false);
            ui->hudOrientation->rootObject()->setProperty("halfWidth", false);
        }
    }
    else {
        ui->hudPower->hide();
        ui->hudLatency->hide();
        ui->hudOrientation->hide();
        ui->hudPower2->hide();
        ui->hudLatency2->hide();
        ui->hudOrientation2->hide();
    }
    adjustSizeAndPosition();
}

void ResearchMainWindow::adjustSizeAndPosition() {
    ui->cameraWidget->move(0, 0);
    ui->cameraWidget->resize(width(), height());
    ui->hudPower->rootObject()->setProperty("height", height() / 4);
    ui->hudPower2->rootObject()->setProperty("height", height() / 4);
    ui->hudOrientation->rootObject()->setProperty("height", height() / 4);
    ui->hudOrientation2->rootObject()->setProperty("height", height() / 4);
    ui->hudLatency->rootObject()->setProperty("height", height() / 2);
    ui->hudLatency2->rootObject()->setProperty("height", height() / 2);
    if (ui->cameraWidget->isStereoOn()) {
        // STEREO
        ui->hudPower->move(_hudParallax, 0);
        ui->hudPower2->move(width() / 2, 0);
        ui->hudLatency->move((width() / 2) - (ui->hudLatency->width() / 2), (height() / 2) - (ui->hudLatency->height() / 2));
        ui->hudLatency2->move(width() - (ui->hudLatency->width() / 2) - _hudParallax, (height() / 2) - (ui->hudLatency->height() / 2));
        ui->hudOrientation->move(_hudParallax, height() - ui->hudOrientation->height());
        ui->hudOrientation2->move(width() / 2, height() - ui->hudOrientation->height());
    }
    else {
        // MONO
        ui->hudPower->move(0, 0);
        ui->hudLatency->move(width() - ui->hudLatency->width(), (height() / 2) - (ui->hudLatency->height() / 2));
        ui->hudOrientation->move(0, height() - ui->hudOrientation->height());
    }
}

void ResearchMainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    adjustSizeAndPosition();
}

void ResearchMainWindow::setHudParallax(int parallax) {
    _hudParallax = parallax;
    adjustSizeAndPosition();
}

void ResearchMainWindow::timerEvent(QTimerEvent *event) {
    if (event->timerId() == _updateLatencyTimerId) {
        int latency = _driveSystem->getChannel()->getLastRtt();
        ui->hudLatency->rootObject()->setProperty("latency", latency);
        ui->hudLatency2->rootObject()->setProperty("latency", latency);
    }
    else if (event->timerId() == _resizeTimerId) {
        // This timer is necessary because maximizing the window sometimes doesn't cause a resize event
        adjustSizeAndPosition();
    }
}

void ResearchMainWindow::setHudVisible(bool visible) {
    _hudVisible = visible;
    adjustHud();
}

bool ResearchMainWindow::isHudVisible() const {
    return _hudVisible;
}

int ResearchMainWindow::getHudParallax() const {
    return _hudParallax;
}

} // namespace MissionControl
} // namespace Soro
