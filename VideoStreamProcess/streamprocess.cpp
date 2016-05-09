#include "streamprocess.h"

namespace Soro {
namespace Rover {

StreamProcess::StreamProcess(QGst::ElementPtr source, StreamFormat format, SocketAddress bindAddress, SocketAddress address, QObject *parent)
        : QObject(parent) {
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

StreamProcess::StreamProcess(QString sourceDevice, StreamFormat format, SocketAddress bindAddress, SocketAddress address, QObject *parent)
        : QObject(parent) {
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

StreamProcess::~StreamProcess() {
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
}

QGst::PipelinePtr StreamProcess::createPipeline() {
    qDebug() << "Creating pipeline";

    QGst::PipelinePtr pipeline = QGst::Pipeline::create();
    pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &StreamProcess::onBusMessage);

    qDebug() << "Pipeline created";
    return pipeline;
}

QString StreamProcess::makeEncodingBinString(StreamFormat format, SocketAddress bindAddress, SocketAddress address) {
    QString binStr = "videoconvert ! ";
    QString caps = "video/x-raw,format=I420";
    if ((format.Width > 0) & (format.Height > 0)) {
        binStr += "videoscale method=nearest-neighbour ! ";
        caps += ",width=" + QString::number(format.Width) + ",height=" + QString::number(format.Height);
    }
    if (format.Framerate > 0) {
        caps += ",framerate=" + QString::number(format.Framerate) + "/1";
    }
    binStr += caps;
    switch (format.Encoding) {
    case MjpegEncoding:
        binStr += "jpegenc quality=" + QString::number(format.Mjpeg_Quality) + " ! rtpjpegpay ! ";
        break;
    case Mpeg2Encoding:
        binStr += "avenc_mpeg4 bitrate=" + QString::number(format.Bitrate) + " ! rtpmp4vpay config-interval=3 pt=96 ! ";
        break;
    case x264Encoding:
        binStr += "x264enc tune=zerolatency bitrate=" + QString::number(format.Bitrate) + " ! rtph264pay config-interval=3 pt=96 ! ";
        break;
    default:
        //unknown codec
        QCoreApplication::exit(STREAMPROCESS_ERR_UNKNOWN_CODEC);
        return;
    }

    binStr += "udpsink bind-address=" + bindAddress.host.toString() + " bind-port=" + QString::number(bindAddress.port)
            + " host=" + address.host.toString() + " port=" + QString::number(address.port);

    return binStr;
}

void StreamProcess::onBusMessage(const QGst::MessagePtr & message) {
    qDebug() << "Getting bus message";
    switch (message->type()) {
    case QGst::MessageEos:
        qWarning() << "End-of-Stream";
        QCoreApplication::exit(STREAMPROCESS_ERR_GSTREAMER_EOS);
        break;
    case QGst::MessageError:
        qCritical() << "Bus error: " << message.staticCast<QGst::ErrorMessage>()->error().message();
        QCoreApplication::exit(STREAMPROCESS_ERR_GSTREAMER_ERROR);
        break;
    default:
        break;
    }
}

} // namespace Rover
} // namespace Soro

