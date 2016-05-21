#include "videostreamer.h"

namespace Soro {
namespace Rover {

VideoStreamer::VideoStreamer(QGst::ElementPtr source, VideoFormat format, SocketAddress bindAddress, SocketAddress address, quint16 ipcPort, QObject *parent)
        : MediaStreamer(parent) {
    if (!connectToParent(ipcPort)) return;

    _pipeline = createPipeline();

    // create gstreamer command
    QString binStr = makeEncodingBinString(format, bindAddress, address);
    QGst::BinPtr encoder = QGst::Bin::fromDescription(binStr);

    qDebug() << "Created gstreamer bin <source> ! " << binStr;

    // link elements
    _pipeline->add(source, encoder);
    source->link(encoder);

    qDebug() << "Elements linked";

    // play
    _pipeline->setState(QGst::StatePlaying);

    qDebug() << "Stream started";
}

VideoStreamer::VideoStreamer(QString sourceDevice, VideoFormat format, SocketAddress bindAddress, SocketAddress address, quint16 ipcPort, QObject *parent)
        : MediaStreamer(parent) {
    if (!connectToParent(ipcPort)) return;

    _pipeline = createPipeline();

    // create gstreamer command
    QString binStr = makeEncodingBinString(format, bindAddress, address);
#ifdef __linux__
    binStr = "v4l2src device=" + sourceDevice + " ! " + binStr;
#endif
    QGst::BinPtr encoder = QGst::Bin::fromDescription(binStr);

    qDebug() << "Created gstreamer bin " << binStr;

    _pipeline->add(encoder);

    // play
    _pipeline->setState(QGst::StatePlaying);

    qDebug() << "Stream started";

}

QString VideoStreamer::makeEncodingBinString(VideoFormat format, SocketAddress bindAddress, SocketAddress address) {
    QString binStr = "videoconvert ! "
                     "videoscale method=nearest-neighbour ! "
                     "video/x-raw,height=";
    switch (format) {
    case Mpeg2_144p_300Kpbs:
        binStr += "144 ! avenc_mpeg4 bitrate=300000 bitrate-tolerance=300000";
        break;
    case Mpeg2_360p_750Kpbs:
        binStr += "360 ! avenc_mpeg4 bitrate=750000 bitrate-tolerance=500000 max-threads=3";
        break;
    case Mpeg2_480p_1500Kpbs:
        binStr += "480 ! avenc_mpeg4 bitrate=1500000 bitrate-tolerance=1000000 max-threads=3";
        break;
    case Mpeg2_720p_3000Kpbs:
        binStr += "720 ! avenc_mpeg4 bitrate=3000000 bitrate-tolerance=1000000 max-threads=3";
        break;
    case Mpeg2_720p_5000Kpbs:
        binStr += "720 ! avenc_mpeg4 bitrate=5000000 bitrate-tolerance=2000000 max-threads=3";
        break;
    default:
        //unknown codec
        QCoreApplication::exit(STREAMPROCESS_ERR_UNKNOWN_CODEC);
        return "";
    }

    binStr += " max-threads=3 ! "
              "rtpmp4vpay config-interval=3 pt=96 ! "
              "udpsink bind-address=" + bindAddress.host.toString() + " bind-port=" + QString::number(bindAddress.port)
                    + " host=" + address.host.toString() + " port=" + QString::number(address.port);

    return binStr;
}

} // namespace Rover
} // namespace Soro

