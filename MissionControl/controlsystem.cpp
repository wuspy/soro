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

#include "controlsystem.h"

namespace Soro {
namespace MissionControl {

ControlSystem::ControlSystem(const QHostAddress& roverAddress, QObject *parent) : QObject(parent) {
    _roverAddress = roverAddress;
}

ControlSystem::~ControlSystem() {
    if (_channel) {
        disconnect(_channel, 0, this, 0);
        delete _channel;
    }
}

bool ControlSystem::init(QString channelName, quint16 channelPort, QString *errorString) {
    if (_channel) {
        disconnect(_channel, 0, this, 0);
        delete _channel;
    }

    _channel = Channel::createClient(this, SocketAddress(_roverAddress, channelPort), channelName,
            Channel::UdpProtocol, QHostAddress::Any);
    _channel->open();

    if (_channel->getState() == Channel::ErrorState) {
        if (errorString) *errorString = QString("Could not initialize the networking for control subsystems.");
        delete _channel;
        _channel = NULL;
        return false;
    }

    connect(_channel, SIGNAL(stateChanged(Channel*,Channel::State)),
            this, SLOT(channelStateChanged(Channel*,Channel::State)));
    return true;
}

void ControlSystem::channelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    emit connectionStateChanged(state == Channel::ConnectedState);
}

}
}
