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

#include "audioclient.h"

namespace Soro {
namespace MissionControl {

void AudioClient::onServerStreamingMessageInternal(QDataStream& stream) {
    stream >> reinterpret_cast<quint32&>(_format);
}

void AudioClient::onServerStartMessageInternal() {
    _format = AudioFormat_Null;
}

void AudioClient::onServerEosMessageInternal() {
    _format = AudioFormat_Null;
}

void AudioClient::onServerErrorMessageInternal() {
    _format = AudioFormat_Null;
}

AudioClient::AudioClient(int mediaId, SocketAddress server, QHostAddress host, QObject *parent)
    : MediaClient("AudioClient " + QString::number(mediaId), mediaId, server, host, parent) {
}

AudioFormat AudioClient::getAudioFormat() const {
    return _format;
}

void AudioClient::onServerConnectedInternal() { }

void AudioClient::onServerDisconnectedInternal() {
    _format = AudioFormat_Null;
}

} // namespace MissionControl
} // namespace Soro
