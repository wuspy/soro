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

#include "videoclient.h"

namespace Soro {

void VideoClient::onServerStreamingMessageInternal(QDataStream& stream) {
    QString formatSerial;
    stream >> formatSerial;
    _format.deserialize(formatSerial);
}

void VideoClient::onServerStartMessageInternal() {
    _format.setEncoding(VideoFormat::Encoding_Null);
}

void VideoClient::onServerEosMessageInternal() {
    _format.setEncoding(VideoFormat::Encoding_Null);
}

void VideoClient::onServerErrorMessageInternal() {
    _format.setEncoding(VideoFormat::Encoding_Null);
}

VideoClient::VideoClient(int mediaId, SocketAddress server, QHostAddress host, QObject *parent)
    : MediaClient("VideoClient " + QString::number(mediaId), mediaId, server, host, parent) {

    _format.setEncoding(VideoFormat::Encoding_Null);
}

VideoFormat VideoClient::getVideoFormat() const {
    return _format;
}

void VideoClient::onServerConnectedInternal() { }

void VideoClient::onServerDisconnectedInternal() {
    _format.setEncoding(VideoFormat::Encoding_Null);
}

} // namespace Soro
