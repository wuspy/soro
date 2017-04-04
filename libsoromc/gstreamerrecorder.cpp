#include "gstreamerrecorder.h"

// TODO very rough, finish implementing

namespace Soro {
namespace MissionControl {

GStreamerRecorder::GStreamerRecorder(QObject *parent) : QObject(parent)
{

}

void GStreamerRecorder::begin(VideoFormat format, SocketAddress address) {
    stop();
    QString binStr = QString("udpsrc host=%1 port=%2 ! %3 ! filesink locaiton=%4../research_videos/%5.raw").arg(
                address.host.toString(),
                QString::number(address.port),
                format.createGstDecodingArgs(VideoFormat::DecodingType_RtpDecodeOnly),
                QCoreApplication::applicationDirPath(),
                QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()));

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
