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

#include "cameracontrolsystem.h"

namespace Soro {
namespace MissionControl {

CameraControlSystem::CameraControlSystem(const QHostAddress& roverAddress, GamepadManager *input, QObject *parent) : ControlSystem(roverAddress, parent) {
    _input = input;
}

bool CameraControlSystem::init(QString *errorString) {
    if (!_input) {
        if (errorString) *errorString = QString("The gamepad input handler did not initialize successfully.");
        return false;
    }
    return ControlSystem::init(CHANNEL_NAME_GIMBAL, NETWORK_ALL_GIMBAL_CHANNEL_PORT, errorString);
}

void CameraControlSystem::enable() {
    START_TIMER(_controlSendTimerId, 50);
}

void CameraControlSystem::disable() {
    KILL_TIMER(_controlSendTimerId);
}

void CameraControlSystem::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if ((e->timerId() == _controlSendTimerId) && _input->isGamepadConnected()) {
        //send the rover a camera control packet
        GimbalMessage::setGamepadData(_buffer, _input->axisLeftX, _input->axisRightY,
                                      _input->buttonX, _input->buttonY, _input->buttonB, _input->buttonA);
        _channel->sendMessage(_buffer, GimbalMessage::RequiredSize);
        emit cameraMessageSent(_buffer, GimbalMessage::RequiredSize);
    }
}

}
}
