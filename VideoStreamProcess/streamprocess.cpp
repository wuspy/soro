#include "streamprocess.h"

namespace Soro {
namespace Rover {

StreamProcess::StreamProcess(QGst::ElementPtr source, StreamFormat format, SocketAddress host, SocketAddress address, QObject *parent)
        : QObject(parent) {

    qDebug() << "Creating pipeline";

    // create pipeline
    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &StreamProcess::onBusMessage);

    qDebug() << "Pipeline created";

    // create gstreamer command
    QString binStr = "videoconvert ! ";
    QString caps = "video/x-raw,format=I420";
    if ((format.Width > 0) & (format.Height > 0)) {
        binStr += "videoscale ! ";
        caps += ",width=" + QString::number(format.Width) + ",height=" + QString::number(format.Height);
    }
    if (format.Framerate > 0) {
        caps += ",framerate=" + QString::number(format.Framerate) + "/1";
    }
    binStr += caps;
    switch (format.Encoding) {
    case MjpegEncoding:
        binStr += " ! jpegenc quality=" + QString::number(format.Mjpeg_Quality) + " ! rtpjpegpay ! ";
        break;
    case Mpeg2Encoding:
        binStr += " ! avenc_mpeg4 bitrate=" + QString::number(format.Bitrate) + " ! rtpmp4vpay config-interval=3 ! ";
        break;
    case x264Encoding:
        binStr += " ! x264enc tune=zerolatency bitrate=" + QString::number(format.Bitrate / 1000) + " ! rtph264pay ! ";
        break;
    default:
        //unknown codec
        QCoreApplication::exit(STREAMPROCESS_ERR_UNKNOWN_CODEC);
        return;
    }

    binStr += "udpsink bind-address=" + host.host.toString() + " bind-port=" + QString::number(host.port) + " host=" + address.host.toString() + " port=" + QString::number(address.port);
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

StreamProcess::~StreamProcess() {
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
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

