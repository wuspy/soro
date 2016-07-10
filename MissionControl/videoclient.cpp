#include "videoclient.h"

namespace Soro {
namespace MissionControl {

void VideoClient::onServerStreamingMessageInternal(QDataStream& stream) {
    stream >> reinterpret_cast<quint32&>(_format);
}

void VideoClient::onServerStartMessageInternal() {
    _format = VideoFormat_Null;
}

void VideoClient::onServerEosMessageInternal() {
    _format = VideoFormat_Null;
}

void VideoClient::onServerErrorMessageInternal() {
    _format = VideoFormat_Null;
}

VideoClient::VideoClient(int mediaId, SocketAddress server, QHostAddress host, QObject *parent)
    : MediaClient("VideoClient " + QString::number(mediaId), mediaId, server, host, parent) {

    _format = VideoFormat_Null;
}

VideoFormat VideoClient::getVideoFormat() const {
    return _format;
}

void VideoClient::onServerConnectedInternal() { }

void VideoClient::onServerDisconnectedInternal() {
    _format = VideoFormat_Null;
}

} // namespace MissionControl
} // namespace Soro
