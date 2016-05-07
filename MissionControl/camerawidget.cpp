#include "camerawidget.h"
#include "ui_camerawidget.h"

namespace Soro {
namespace MissionControl {


CameraWidget::CameraWidget(QWidget *parent) : QWidget(parent), ui(new Ui::CameraWidget) {
    ui->setupUi(this);

    ui->messageLabel->setVisible(false);
    addWidgetShadow(ui->controlsWidget, 10, 0);

    _isPlaying = true;
    stop();
}

CameraWidget::~CameraWidget() {
    resetPipeline();
}

void CameraWidget::play(QGst::ElementPtr source, VideoEncoding encoding) {
    resetPipeline();
    ui->messageLabel->setVisible(false);
    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &CameraWidget::onBusMessage);

    QGst::BinPtr decoder = createDecoder(encoding);
    if (!decoder) {
        stop("The video player doesn't recognize the encoding");
        return;
    }
    QGst::ElementPtr sink = createSink();

    _pipeline->add(source, decoder, sink);
    source->link(decoder);
    decoder->link(sink);

    _isPlaying = true;
    _pipeline->setState(QGst::StatePlaying);
}

void CameraWidget::play(SocketAddress address, VideoEncoding encoding) {
    resetPipeline();
    ui->messageLabel->setVisible(false);

    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &CameraWidget::onBusMessage);

    QGst::BinPtr source = QGst::Bin::fromDescription("udpsrc address=" + address.host.toString() + " port=" + QString::number(address.port));
    QGst::BinPtr decoder = createDecoder(encoding);
    if (!decoder) {
        stop("The video player doesn't recognize the encoding");
        return;
    }
    QGst::ElementPtr sink = createSink();

    _pipeline->add(source, decoder, sink);
    source->link(decoder);
    decoder->link(sink);

    _isPlaying = true;
    _pipeline->setState(QGst::StatePlaying);
}

QGst::ElementPtr CameraWidget::createSink() {
    QGst::ElementPtr sink = QGst::ElementFactory::make("qt5videosink");
    sink->setProperty("force-aspect-ratio", true);
    ui->videoWidget->setVideoSink(sink);
    return sink;
}

QGst::BinPtr CameraWidget::createDecoder(VideoEncoding encoding) {
    switch (encoding) {
    case MjpegEncoding:
        return QGst::Bin::fromDescription("identity ! application/x-rtp,encoding=JPEG,payload=26 ! "
                                       "rtpjpegdepay ! "
                                       "jpegdec ! "
                                       "videoconvert");
        break;
    case Mpeg2Encoding:
        return QGst::Bin::fromDescription("identity ! application/x-rtp,media=video,clock-rate=90000,encoding-name=MP4V-ES,profile-level-id=1,payload=96,ssrc=2873740600,timestamp-offset=391825150,seqnum-offset=2980 ! "
                                       "rtpmp4vdepay ! "
                                       "avdec_mpeg4 ! "
                                       "videoconvert");
        break;
    case x264Encoding:
        return QGst::Bin::fromDescription("identity ! application/x-rtp,media=video,clock-rate=90000,encoding-name=H264 ! "
                                       "rtph264depay ! "
                                       "avdec_h264 ! "
                                       "videoconvert");
        break;
    default:
        return QGst::BinPtr::wrap(NULL, false);
    }
}

void CameraWidget::stop(QString reason) {
    if (reason.isEmpty()) {
        ui->messageLabel->setText("<html><h2>No video :(</h2></html>");
    }
    else {
        ui->messageLabel->setText("<html><h2>No video :(</h2><br><br>" + reason + "</html>");
    }
    ui->messageLabel->setVisible(true);

    if (_isPlaying) {
        resetPipeline();
        //create videotestsrc pipeline for coolness
        _pipeline = QGst::Pipeline::create();
        QGst::ElementPtr sink = QGst::ElementFactory::make("qt5videosink");
        QGst::ElementPtr source = QGst::ElementFactory::make("videotestsrc");
        source->setProperty("pattern", 1); //snow pattern
        sink->setProperty("force-aspect-ratio", false);

        _pipeline->add(source, sink);
        source->link(sink);
        ui->videoWidget->setVideoSink(sink);

        _isPlaying = false;
        _pipeline->setState(QGst::StatePlaying);
    }
}

void CameraWidget::resetPipeline() {
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
}

void CameraWidget::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    ui->messageLabel->move(0, 0);
    ui->messageLabel->resize(width(), height());
    ui->videoWidget->move(0, 0);
    ui->videoWidget->resize(width(), height());

    ui->controlsWidget->move(width() - ui->controlsWidget->width() - 20, height() - ui->controlsWidget->height() - 20);
}

QString CameraWidget::getCameraName() {
    return ui->cameraNameLabel->text();
}

void CameraWidget::setCameraName(QString name) {
    ui->cameraNameLabel->setText(name);
}

bool CameraWidget::isPlaying() {
    return _isPlaying;
}

void CameraWidget::onBusMessage(const QGst::MessagePtr & message) {
    switch (message->type()) {
    case QGst::MessageEos:
        stop("Received end-of-stream message.");
        emit eosMessage();
        break;
    case QGst::MessageError:
        //stop("Decoding Error: " + message.staticCast<QGst::ErrorMessage>()->error().message() + "<br><br>Resetting...");
        //emit error();
        //break;
    default:
        break;
    }
}

} // namespace MissionControl
} // namespace Soro
