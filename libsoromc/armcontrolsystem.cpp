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

#include "armcontrolsystem.h"

#define LOG_TAG "ArmControlSystem"

namespace Soro {
namespace MissionControl {

ArmControlSystem::ArmControlSystem(const QHostAddress& roverAddress, QObject *parent) : ControlSystem(roverAddress, parent) { }

ArmControlSystem::~ArmControlSystem() {
    if (_mbed) {
        delete _mbed;
    }
}

bool ArmControlSystem::init(QString *errorString) {
    _enabled = false;
    if (!ControlSystem::init(CHANNEL_NAME_ARM, NETWORK_ALL_ARM_CHANNEL_PORT, errorString)) return false;

    if (_mbed) {
        disconnect(_mbed, 0, this, 0);
        delete _mbed;
    }

    _mbed = new MbedChannel(SocketAddress(QHostAddress::Any, NETWORK_MC_MASTER_ARM_PORT), MBED_ID_MASTER_ARM);

    connect(_mbed, SIGNAL(stateChanged(MbedChannel::State)),
            this, SLOT(mbedStateChanged(MbedChannel::State)));
    connect(_mbed, SIGNAL(messageReceived(const char*,int)),
            this, SLOT(mbedMessageReceived(const char*,int)));

    if (!reloadMasterArmConfig()) {
        *errorString = "Failed to load master arm configuration from '../config/master_arm.conf'";
        return false;
    }
    return true;
}

void ArmControlSystem::enable() {
    _enabled = true;
}

void ArmControlSystem::disable() {
     _enabled = false;
}

bool ArmControlSystem::reloadMasterArmConfig() {
    QFile masterArmFile(QCoreApplication::applicationDirPath() + "/../config/master_arm.conf");
    if (_armConfig.load(masterArmFile)) {
        LOG_I(LOG_TAG, "Loaded new master arm configuration");
        return true;
    }
    else {
        LOG_E(LOG_TAG, "Failed to load master arm configuration from '../config/master_arm.conf'");
        return false;
    }
}

void ArmControlSystem::mbedMessageReceived(const char *message, int size) {
    if (_enabled && _channel && _armConfig.isLoaded) {
        //translate message from master pot values to slave servo values
        memcpy(_buffer, message, size);
        ArmMessage::translateMasterArmValues(_buffer, _armConfig);
        if (_channel != NULL) {
            _channel->sendMessage(_buffer, size);
        }
        else {
            LOG_E(LOG_TAG, "Got message from master arm with null control channel");
        }
    }
    emit masterArmStateChanged(message);
}

void ArmControlSystem::mbedStateChanged(MbedChannel::State state) {
    emit masterArmStateChanged(state == MbedChannel::ConnectedState);
}

bool ArmControlSystem::isMasterArmConnected() const {
    return _mbed->getState() == MbedChannel::ConnectedState;
}

}
}
