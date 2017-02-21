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
    ui->hudOrientationBack2->rootObject()->setProperty("halfWidth", true);
    ui->hudOrientationSide2->rootObject()->setProperty("halfWidth", true);

    // Make the embedded quick widgets transparent
    ui->hudPower->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudPower->setClearColor(Qt::transparent);
    ui->hudPower2->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudPower2->setClearColor(Qt::transparent);
    ui->hudLatency->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudLatency->setClearColor(Qt::transparent);
    ui->hudLatency2->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudLatency2->setClearColor(Qt::transparent);
    ui->hudOrientationBack->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudOrientationBack->setClearColor(Qt::transparent);
    ui->hudOrientationBack2->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudOrientationBack2->setClearColor(Qt::transparent);
    ui->hudOrientationSide->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudOrientationSide->setClearColor(Qt::transparent);
    ui->hudOrientationSide2->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->hudOrientationSide2->setClearColor(Qt::transparent);

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
        x = 0;
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
    case SensorDataRecorder::DATATAG_WHEELPOWER_A:
        ui->hudPower->rootObject()->setProperty("wheelMLPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelMLPower", value);
        break;
    case SensorDataRecorder::DATATAG_WHEELPOWER_B:
        ui->hudPower->rootObject()->setProperty("wheelFLPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelFLPower", value);
        break;
    case SensorDataRecorder::DATATAG_WHEELPOWER_C:
        ui->hudPower->rootObject()->setProperty("wheelFRPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelFRPower", value);
        break;
    case SensorDataRecorder::DATATAG_WHEELPOWER_D:
        ui->hudPower->rootObject()->setProperty("wheelMRPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelMRPower", value);
        break;
    case SensorDataRecorder::DATATAG_WHEELPOWER_E:
        ui->hudPower->rootObject()->setProperty("wheelBRPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelBRPower", value);
        break;
    case SensorDataRecorder::DATATAG_WHEELPOWER_F:
        ui->hudPower->rootObject()->setProperty("wheelBLPower", value);
        ui->hudPower2->rootObject()->setProperty("wheelBLPower", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_REAR_YAW:
        ui->hudOrientationBack->rootObject()->setProperty("rearYaw", value);
        ui->hudOrientationBack2->rootObject()->setProperty("rearYaw", value);
        ui->hudOrientationSide->rootObject()->setProperty("rearYaw", value);
        ui->hudOrientationSide2->rootObject()->setProperty("rearYaw", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_REAR_PITCH:
        ui->hudOrientationBack->rootObject()->setProperty("rearPitch", value);
        ui->hudOrientationBack2->rootObject()->setProperty("rearPitch", value);
        ui->hudOrientationSide->rootObject()->setProperty("rearPitch", value);
        ui->hudOrientationSide2->rootObject()->setProperty("rearPitch", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_REAR_ROLL:
        ui->hudOrientationBack->rootObject()->setProperty("rearRoll", value);
        ui->hudOrientationBack2->rootObject()->setProperty("rearRoll", value);
        ui->hudOrientationSide->rootObject()->setProperty("rearRoll", value);
        ui->hudOrientationSide2->rootObject()->setProperty("rearRoll", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_FRONT_YAW:
        ui->hudOrientationBack->rootObject()->setProperty("frontYaw", value);
        ui->hudOrientationBack2->rootObject()->setProperty("frontYaw", value);
        ui->hudOrientationSide->rootObject()->setProperty("frontYaw", value);
        ui->hudOrientationSide2->rootObject()->setProperty("frontYaw", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_FRONT_PITCH:
        ui->hudOrientationBack->rootObject()->setProperty("frontPitch", value);
        ui->hudOrientationBack2->rootObject()->setProperty("frontPitch", value);
        ui->hudOrientationSide->rootObject()->setProperty("frontPitch", value);
        ui->hudOrientationSide2->rootObject()->setProperty("frontPitch", value);
        break;
    case SensorDataRecorder::DATATAG_IMUDATA_FRONT_ROLL:
        ui->hudOrientationBack->rootObject()->setProperty("frontRoll", value);
        ui->hudOrientationBack2->rootObject()->setProperty("frontRoll", value);
        ui->hudOrientationSide->rootObject()->setProperty("frontRoll", value);
        ui->hudOrientationSide2->rootObject()->setProperty("frontRoll", value);
        break;
    }
}

void ResearchMainWindow::adjustHud() {
    if (_hudVisible) {
        ui->hudPower->show();
        ui->hudLatency->show();
        ui->hudOrientationSide->show();
        ui->hudOrientationBack->show();
        if (ui->cameraWidget->isStereoOn()) {
            ui->hudPower2->show();
            ui->hudLatency2->show();
            ui->hudOrientationBack2->show();
            ui->hudOrientationSide2->show();
            ui->hudPower->rootObject()->setProperty("halfWidth", true);
            ui->hudLatency->rootObject()->setProperty("halfWidth", true);
            ui->hudOrientationBack->rootObject()->setProperty("halfWidth", true);
            ui->hudOrientationSide->rootObject()->setProperty("halfWidth", true);
        }
        else {
            ui->hudPower2->hide();
            ui->hudLatency2->hide();
            ui->hudOrientationBack2->hide();
            ui->hudOrientationSide2->hide();
            ui->hudPower->rootObject()->setProperty("halfWidth", false);
            ui->hudLatency->rootObject()->setProperty("halfWidth", false);
            ui->hudOrientationBack->rootObject()->setProperty("halfWidth", false);
            ui->hudOrientationSide->rootObject()->setProperty("halfWidth", false);
        }
    }
    else {
        ui->hudPower->hide();
        ui->hudLatency->hide();
        ui->hudOrientationBack->hide();
        ui->hudOrientationSide->hide();
        ui->hudPower2->hide();
        ui->hudLatency2->hide();
        ui->hudOrientationBack2->hide();
        ui->hudOrientationSide2->hide();
    }
    adjustSizeAndPosition();
}

void ResearchMainWindow::adjustSizeAndPosition() {
    ui->cameraWidget->move(0, 0);
    ui->cameraWidget->resize(width(), height());
    ui->hudPower->rootObject()->setProperty("height", height() / 3);
    ui->hudPower2->rootObject()->setProperty("height", height() / 3);
    ui->hudOrientationBack->rootObject()->setProperty("height", height() / 3);
    ui->hudOrientationBack2->rootObject()->setProperty("height", height() / 3);
    ui->hudOrientationSide->rootObject()->setProperty("height", height() / 3);
    ui->hudOrientationSide2->rootObject()->setProperty("height", height() / 3);
    ui->hudLatency->rootObject()->setProperty("height", height() / 1.5);
    ui->hudLatency2->rootObject()->setProperty("height", height() / 1.5);
    if (ui->cameraWidget->isStereoOn()) {
        // STEREO
        ui->hudPower->move(
                    _hudParallax,
                    0
                );
        ui->hudPower2->move(
                    width() / 2,
                    0
                );
        ui->hudLatency->move(
                    _hudParallax,
                    height() - ui->hudLatency->height()
                );
        ui->hudLatency2->move(
                    width() / 2,
                    height() - ui->hudLatency2->height()
                );
        ui->hudOrientationBack->move(
                    width() / 2 - ui->hudOrientationBack->width() / 2 - _hudParallax,
                    height() - ui->hudOrientationBack->height()
                );
        ui->hudOrientationBack2->move(
                    width() - ui->hudOrientationBack2->width() / 2,
                    height() - ui->hudOrientationBack2->height()
                );
        ui->hudOrientationSide->move(
                    width() / 2 - ui->hudOrientationSide->width() / 2 - _hudParallax,
                    height() - ui->hudOrientationSide->height() - ui->hudOrientationBack->height()
                );
        ui->hudOrientationSide2->move(
                    width() - ui->hudOrientationSide2->width() / 2,
                    height() - ui->hudOrientationBack2->height() - ui->hudOrientationBack2->height()
                );
    }
    else {
        // MONO
        ui->hudPower->move(
                    0,
                    0
                );
        ui->hudLatency->move(
                    0,
                    height() - ui->hudLatency->height()
                );
        ui->hudOrientationBack->move(
                    width() - ui->hudOrientationBack->width(),
                    height() - ui->hudOrientationBack->height()
                );
        ui->hudOrientationSide->move(
                    width() - ui->hudOrientationSide->width(),
                    height() - ui->hudOrientationSide->height() - ui->hudOrientationBack->height()
                );
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
        if (latency >= 0) { // Don't add to latency if there's no connection
            latency += _hudLatency;
        }
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

void ResearchMainWindow::setHudLatency(int latency) {
    _hudLatency = latency;
}

int ResearchMainWindow::getHudLatency() const {
    return _hudLatency;
}

} // namespace MissionControl
} // namespace Soro
