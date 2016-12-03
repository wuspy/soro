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

#include "videoserver.h"
#include "logger.h"

namespace Soro {

VideoServer::VideoServer(int mediaId, SocketAddress host, QObject *parent)
    : MediaServer("VideoServer " + QString::number(mediaId), mediaId, QCoreApplication::applicationDirPath() + "/VideoStreamProcess" , host, parent) {
}

void VideoServer::onStreamStoppedInternal() {
    if (!_starting) {
        _videoDevice = "";
        _format = VideoFormat_Null;
    }
}

void VideoServer::start(QString deviceName, VideoFormat format) {
    _videoDevice = deviceName;
    _format = format;

    // prevent onStreamStoppedInternal from resetting the stream parameters
    // in the event a running stream must be stopped
    _starting = true;
    initStream();
    _starting = false;
}

/*void VideoServer::start(FlyCapture2::PGRGuid camera, VideoFormat format) {
    start("FlyCapture2:" + QString::number(camera.value[0]) + ":"
                        + QString::number(camera.value[1]) + ":"
                        + QString::number(camera.value[2]) + ":"
                        + QString::number(camera.value[3]),
                        format);
}*/

void VideoServer::constructChildArguments(QStringList& outArgs, SocketAddress host, SocketAddress address, quint16 ipcPort) {
    outArgs << _videoDevice;
    outArgs << QString::number(reinterpret_cast<unsigned int&>(_format));
    outArgs << QHostAddress(address.host.toIPv4Address()).toString();
    outArgs << QString::number(address.port);
    outArgs << QHostAddress(host.host.toIPv4Address()).toString();
    outArgs << QString::number(host.port);
    outArgs << QString::number(ipcPort);

    qDebug() << "Starting with args " << outArgs;
}

void VideoServer::constructStreamingMessage(QDataStream& stream) {
    stream << reinterpret_cast<quint32&>(_format);
}

VideoFormat VideoServer::getVideoFormat() const {
    return _format;
}

} // namespace Soro
