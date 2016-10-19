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

namespace Soro {
namespace MissionControl {

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
    QString binStr = "udpsrc address=" + address.host.toString() + " port=" + QString::number(address.port);

    // append encoding-specific elements
    switch (encoding) {
    case AC3:
        binStr += " ! application/x-rtp,media=audio,clock-rate=44100,encoding-name=AC3 ! "
                               "rtpac3depay ! "
                               "a52dec ! "
                               "audioconvert ! ";
        break;
    default:
        stop();
        emit error();
        return;
    }
#ifdef __linux__
    binStr += "alsasink";
#else
    binStr += "autoaudiosink";
#endif

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
    qDebug() << "Got bus message type " << message->typeName();
    switch (message->type()) {
    case QGst::MessageEos:
        stop();
        emit eosMessage();
        break;
    case QGst::MessageError:
        emit error();
        break;
    default:
        break;
    }
}

}
}
