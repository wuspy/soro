#include "flycapsource.h"

namespace Soro {
namespace Rover {

FlycapSource::FlycapSource(int width, int height, int framerate) {
    setLatency(-1, -1);
    setLive(true);
    setStreamType(QGst::AppStreamTypeStream);
    setSize(-1);

    // create caps string
    QString caps = "video/x-raw,format=RGB,"
                    "width=(int)" + QString::number(width) + ","
                    "height=(int)" + QString::number(height) + ","
                    "framerate=(fraction)" + QString::number(framerate) + "/1";

    // configure application source
    setCaps(QGst::Caps::fromString(caps));

    NeedsData = false;
}

FlycapSource::~FlycapSource() { }

void FlycapSource::needData(uint length) {
    Q_UNUSED(length);
    NeedsData = true;
}

void FlycapSource::enoughData() {
    NeedsData = false;
}

} // namespace Rover
} // namespace Soro
