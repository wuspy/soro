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

#include "drivecontrolsystem.h"

namespace Soro {
namespace MissionControl {

DriveControlSystem::DriveControlSystem(const Configuration *config, GamepadManager *input, QObject *parent) : ControlSystem(config, parent) {
    _input = input;
}

bool DriveControlSystem::init(QString *errorString) {
    KILL_TIMER(_controlSendTimerId);
    if (!_input) {
        if (errorString) *errorString = QString("The gamepad input handler did not initialize successfully.");
        return false;
    }
    return ControlSystem::init(CHANNEL_NAME_DRIVE, _config->DriveChannelPort, errorString);
}

void DriveControlSystem::enable() {
    START_TIMER(_controlSendTimerId, 50);
}

void DriveControlSystem::disable() {
    KILL_TIMER(_controlSendTimerId);
}

void DriveControlSystem::setMiddleSkidFactor(float factor) {
    _midSkidFactor = factor;
}

void DriveControlSystem::setMode(DriveGamepadMode mode) {
    _mode = mode;
}

void DriveControlSystem::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if ((e->timerId() == _controlSendTimerId) && _channel && _input->isGamepadConnected()) {
        //send the rover a drive gamepad packet
        switch (_mode) {
        case SingleStickDrive:
            DriveMessage::setGamepadData_DualStick(_buffer, _input->axisLeftX, _input->axisLeftY, _midSkidFactor);
            break;
        case DualStickDrive:
            DriveMessage::setGamepadData_DualStick(_buffer, _input->axisLeftY, _input->axisRightY, _midSkidFactor);
            break;
        }
        _channel->sendMessage(_buffer, DriveMessage::RequiredSize);
    }
}

}
}
