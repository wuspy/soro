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

AudioClient::AudioClient(int mediaId, SocketAddress server, QHostAddress host, Logger *log, QObject *parent)
    : MediaClient("AudioClient " + QString::number(mediaId), mediaId, server, host, log, parent) {
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
