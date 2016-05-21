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
