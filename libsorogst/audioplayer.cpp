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

#include "audioplayer.h"
#include "libsoro/logger.h"

#define LOG_TAG "AudioPlayer"

namespace Soro {
namespace Gst {

AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent) { }

AudioPlayer::~AudioPlayer() {
    resetPipeline();
}

void AudioPlayer::stop() {
    if (_isPlaying) {
        resetPipeline();
        _isPlaying = false;
    }
}

void AudioPlayer::resetPipeline() {
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
}

void AudioPlayer::play(SocketAddress address, AudioFormat encoding) {
    resetPipeline();

    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &AudioPlayer::onBusMessage);

    // create a udpsrc to receive the stream
    QString binStr = "udpsrc address=%1 port=%2 ! "
                     "%3 ! "
                     "audioconvert ! "
                     "alsasink";

    binStr = binStr.arg(address.host.toString(),
                        QString::number(address.port),
                        encoding.createGstDecodingArgs());

    // create a gstreamer bin from the description
    QGst::BinPtr bin = QGst::Bin::fromDescription(binStr);

    _pipeline->add(bin);

    _isPlaying = true;
    _pipeline->setState(QGst::StatePlaying);
}

bool AudioPlayer::isPlaying() {
    return _isPlaying;
}

void AudioPlayer::onBusMessage(const QGst::MessagePtr & message) {
    LOG_I(LOG_TAG, "onBusMessage(): Got bus message type " + message->typeName());
    switch (message->type()) {
    case QGst::MessageEos:
        stop();
        emit eosMessage();
        break;
    case QGst::MessageError: {
        QString errorMessage = message.staticCast<QGst::ErrorMessage>()->error().message().toLatin1();
        LOG_E(LOG_TAG, "onBusMessage(): Received error message from gstreamer '" + errorMessage + "'");
        emit error();
        break;
    }
    default:
        break;
    }
}

} // namespace Gst
} // namespace Soro
