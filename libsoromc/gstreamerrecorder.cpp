#include "gstreamerrecorder.h"

// TODO very rough, finish implementing

namespace Soro {
namespace MissionControl {

GStreamerRecorder::GStreamerRecorder(SocketAddress videoAddress, QString name, QObject *parent) : QObject(parent)
{
    _name = name;
    _videoAddress = videoAddress;
}

void GStreamerRecorder::begin(VideoFormat format, qint64 timestamp) {
    stop();
    QString binStr = QString("udpsrc host=%1 port=%2 ! %3 ! filesink locaiton=%4../research_videos/%5_%6.raw").arg(
                _videoAddress.host.toString(),
                QString::number(_videoAddress.port),
                format.createGstDecodingArgs(VideoFormat::DecodingType_RtpDecodeOnly),
                QCoreApplication::applicationDirPath(),
                QString::number(timestamp),
                _name);

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
