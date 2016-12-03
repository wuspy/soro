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

#include "audioserver.h"
#include "logger.h"

namespace Soro {

AudioServer::AudioServer(int mediaId, SocketAddress host, QObject *parent)
    : MediaServer("AudioServer " + QString::number(mediaId), mediaId, QCoreApplication::applicationDirPath() + "/AudioStreamProcess" , host, parent) {
}

void AudioServer::onStreamStoppedInternal() {
    if (!_starting) {
        _audioDevice = "";
        _format = AudioFormat_Null;
    }
}

void AudioServer::start(QString deviceName, AudioFormat format) {
    _audioDevice = deviceName;

    _format = format;

    // prevent onStreamStoppedInternal from resetting the stream parameters
    // in the event a running stream must be stopped
    _starting = true;
    initStream();
    _starting = false;
}

void AudioServer::constructChildArguments(QStringList& outArgs, SocketAddress host, SocketAddress address, quint16 ipcPort) {
    outArgs << _audioDevice;
    outArgs << QString::number(reinterpret_cast<unsigned int&>(_format));
    outArgs << QHostAddress(address.host.toIPv4Address()).toString();
    outArgs << QString::number(address.port);
    outArgs << QHostAddress(host.host.toIPv4Address()).toString();
    outArgs << QString::number(host.port);
    outArgs << QString::number(ipcPort);

    qDebug() << "Starting with args " << outArgs;
}

void AudioServer::constructStreamingMessage(QDataStream& stream) {
    stream << reinterpret_cast<quint32&>(_format);
}

AudioFormat AudioServer::getAudioFormat() const {
    return _format;
}

} // namespace Soro
