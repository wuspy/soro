#include "gstreamerrecorder.h"

// TODO very rough, finish implementing

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
                    QString("%1../research_media/%2_%3_%4.%5").arg(
                        QCoreApplication::applicationDirPath(),
                        QString::number(timestamp),
                        _name,
                        format->toHumanReadableString(),
                        format->getFileExtension())
                    )
                );

    _pipeline = QGst::Pipeline::create();

    _bin = QGst::Bin::fromDescription(binStr);
    _pipeline->add(_bin);
    _pipeline->setState(QGst::StatePlaying);
}

void GStreamerRecorder::stop() {
    if (!_pipeline.isNull()) {
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
        _bin.clear();
    }
}

} // namespace MissionControl
} // namespace Soro
