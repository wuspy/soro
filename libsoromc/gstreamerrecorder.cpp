#include "gstreamerrecorder.h"
#include "libsoro/logger.h"

#include <Qt5GStreamer/QGst/Bus>
#include <Qt5GStreamer/QGlib/Connect>

#define LOG_TAG "GStreamerRecorder" + _name

namespace Soro {
namespace MissionControl {

GStreamerRecorder::GStreamerRecorder(SocketAddress mediaAddress, QString name, QObject *parent) : QObject(parent)
{
    _name = name;
    _mediaAddress = mediaAddress;
}

void GStreamerRecorder::begin(const MediaFormat* format, qint64 timestamp) {
    stop();
    QString binStr = QString("udpsrc address=%1 port=%2 reuse=true ! %3 ! %4").arg(
                _mediaAddress.host.toString(),
                QString::number(_mediaAddress.port),
                format->createGstDecodingArgs(VideoFormat::DecodingType_RtpDecodeOnly),
                format->createGstFileRecordingArgs(
                    QString("\"%1/../research_media/%2_%3.%4\"").arg(
                        QCoreApplication::applicationDirPath(),
                        QString::number(timestamp),
                        _name,
                        format->getFileExtension())
                    )
                );
    LOG_I(LOG_TAG, "Starting recording with bin string " + binStr);
    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &GStreamerRecorder::onBusMessage);

    _bin = QGst::Bin::fromDescription(binStr);
    _pipeline->add(_bin);
    _pipeline->setState(QGst::StatePlaying);
}

void GStreamerRecorder::stop() {
    if (!_pipeline.isNull()) {
        LOG_I(LOG_TAG, "Stopping recording");
        _pipeline->bus()->removeSignalWatch();
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
        _bin.clear();
    }
}

void GStreamerRecorder::onBusMessage(const QGst::MessagePtr & message) {
    switch (message->type()) {
    case QGst::MessageEos:
        LOG_E(LOG_TAG, "onBusMessage(): Received end-of-stream message.");
        stop();
        break;
    case QGst::MessageError:
    {
        QString errorMessage = message.staticCast<QGst::ErrorMessage>()->error().message().toLatin1();
        LOG_E(LOG_TAG, "onBusMessage(): Received error message from gstreamer '" + errorMessage + "'");
        stop();
    }
    default:
        break;
    }
}

} // namespace MissionControl
} // namespace Soro
