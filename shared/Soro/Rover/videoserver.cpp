#include "videoserver.h"

namespace Soro {
namespace Rover {

VideoServer::VideoServer(int mediaId, SocketAddress host, Logger *log, QObject *parent)
    : MediaServer("VideoServer " + QString::number(mediaId), mediaId, QCoreApplication::applicationDirPath() + "/VideoStreamProcess" , host, log, parent) {
}

void VideoServer::onStreamStoppedInternal() {
    _videoDevice = "";
    _format = VideoFormat_Null;
}

void VideoServer::start(QString deviceName, VideoFormat format) {
    _videoDevice = deviceName;
    _format = format;

    initStream();
}

void VideoServer::start(FlyCapture2::PGRGuid camera, VideoFormat format) {
    start("FlyCapture2:" + QString::number(camera.value[0]) + ":"
                        + QString::number(camera.value[1]) + ":"
                        + QString::number(camera.value[2]) + ":"
                        + QString::number(camera.value[3]),
                        format);
}

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

} // namespace Rover
} // namespace Soro
