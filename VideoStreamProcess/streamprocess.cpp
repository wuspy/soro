#include "streamprocess.h"

namespace Soro {
namespace Rover {

StreamProcess::StreamProcess(QGst::ElementPtr source, StreamFormat format, SocketAddress bindAddress, SocketAddress address, quint16 ipcPort, QObject *parent)
        : QObject(parent) {
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

StreamProcess::StreamProcess(QString sourceDevice, StreamFormat format, SocketAddress bindAddress, SocketAddress address, quint16 ipcPort, QObject *parent)
        : QObject(parent) {
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

StreamProcess::~StreamProcess() {
    stop();
}

void StreamProcess::stop() {
    qDebug() << "Stopping";
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
    if (_ipcSocket) {
        _ipcSocket->abort();
        delete _ipcSocket;
        _ipcSocket = NULL;
    }
}

bool StreamProcess::connectToParent(quint16 port) {
    _ipcSocket = new QTcpSocket(this);

    connect(_ipcSocket, SIGNAL(readyRead()),
            this, SLOT(ipcSocketReadyRead()));

    _ipcSocket->connectToHost(QHostAddress::LocalHost, port);
    if (!_ipcSocket->waitForConnected(1000)) {
        qCritical() << "Unable to connect to parent on port " << port;
        QCoreApplication::exit(0);
        return false;
    }
    return true;
}

void StreamProcess::ipcSocketReadyRead() {
    char buffer[512];
    while (_ipcSocket->bytesAvailable() > 0) {
        _ipcSocket->readLine(buffer, 512);
        if (QString(buffer).compare("stop") == 0) {
            qDebug() << "Got exit request from parent";
            stop();
            QCoreApplication::exit(0);
        }
    }
}

QGst::PipelinePtr StreamProcess::createPipeline() {
    qDebug() << "Creating pipeline";

    QGst::PipelinePtr pipeline = QGst::Pipeline::create();
    pipeline->bus()->addSignalWatch();
    QGlib::connect(pipeline->bus(), "message", this, &StreamProcess::onBusMessage);

    qDebug() << "Pipeline created";
    return pipeline;
}

QString StreamProcess::makeEncodingBinString(StreamFormat format, SocketAddress bindAddress, SocketAddress address, bool convertI420) {
    QString binStr = "videoconvert ! ";
    QString caps = "video/x-raw";
    if (convertI420) {
        caps += ",format=I420";
    }
    if (format.Height > 0) {
        binStr += "videoscale method=nearest-neighbour ! ";
        caps += ",height=" + QString::number(format.Height);
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
        // mpeg2 takes bitrate in bps
        binStr += " ! avenc_mpeg4 bitrate=" + QString::number(format.Bitrate) + " bitrate-tolerance=1000000 ! rtpmp4vpay config-interval=3 pt=96 ! ";
        break;
    case x264Encoding:
        // x264 takes bitrate in kbps
        binStr += " ! x264enc tune=zerolatency bitrate=" + QString::number(format.Bitrate / 1000) + " ! rtph264pay config-interval=3 pt=96 ! ";
        break;
    default:
        //unknown codec
        QCoreApplication::exit(STREAMPROCESS_ERR_UNKNOWN_CODEC);
        return "";
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

