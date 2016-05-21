#include "audioserver.h"

namespace Soro {
namespace Rover {

AudioServer::AudioServer(int mediaId, SocketAddress host, Logger *log, QObject *parent)
    : MediaServer("AudioServer " + QString::number(mediaId), mediaId, QCoreApplication::applicationDirPath() + "/AudioStreamProcess" , host, log, parent) {
}

void AudioServer::onStreamStoppedInternal() {
    _audioDevice = "";
    _format = AudioFormat_Null;
}

void AudioServer::start(QString deviceName, AudioFormat format) {
    _audioDevice = deviceName;
    _format = format;

    initStream();
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

} // namespace Rover
} // namespace Soro
