#include "videostreamwidget.h"

namespace Soro {
namespace MissionControl {


VideoStreamWidget::VideoStreamWidget(QWidget *parent) : QWidget(parent) {
    _videoWidget = new QGst::Ui::VideoWidget(this);
    _messageLabel = new QLabel(this);

    _messageLabel->setStyleSheet("QLabel { background-color: #A0000000; color: #FFFFFF; }");
    _messageLabel->setVisible(false);
    _messageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    _isPlaying = true;
    stop();
}

VideoStreamWidget::~VideoStreamWidget() {
    resetPipeline();
}

void VideoStreamWidget::play(QGst::ElementPtr source, VideoEncoding encoding) {
    resetPipeline();
    _messageLabel->setVisible(false);
    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &VideoStreamWidget::onBusMessage);

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

void VideoStreamWidget::play(SocketAddress address, VideoEncoding encoding) {
    resetPipeline();
    _messageLabel->setVisible(false);

    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &VideoStreamWidget::onBusMessage);

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

QGst::ElementPtr VideoStreamWidget::createSink() {
    QGst::ElementPtr sink = QGst::ElementFactory::make("qt5videosink");
    sink->setProperty("force-aspect-ratio", true);
    _videoWidget->setVideoSink(sink);
    return sink;
}

QGst::BinPtr VideoStreamWidget::createDecoder(VideoEncoding encoding) {
    switch (encoding) {
    case MjpegEncoding:
        return QGst::Bin::fromDescription("identity ! application/x-rtp,encoding=JPEG,payload=26 ! "
                                       "rtpjpegdepay ! "
                                       "jpegdec ! "
                                       "videoconvert ! "
                                       "video/x-raw,format=RGB ! "
                                       "identity");
        break;
    case Mpeg2Encoding:
        return QGst::Bin::fromDescription("identity ! application/x-rtp,media=video,clock-rate=90000,encoding-name=MP4V-ES,profile-level-id=1,payload=96,ssrc=2873740600,timestamp-offset=391825150,seqnum-offset=2980 ! "
                                       "rtpmp4vdepay ! "
                                       "avdec_mpeg4 ! "
                                       "videoconvert ! "
                                       "video/x-raw,format=RGB ! "
                                       "identity");
        break;
    default:
        return QGst::BinPtr::wrap(NULL, false);
    }
}

void VideoStreamWidget::stop(QString reason) {
    if (reason.isEmpty()) {
        _messageLabel->setText("<html><h2>No video :(</h2></html>");
    }
    else {
        _messageLabel->setText("<html><h2>No video :(</h2><br><br>" + reason + "</html>");
    }
    _messageLabel->setVisible(true);

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
        _videoWidget->setVideoSink(sink);

        _isPlaying = false;
        _pipeline->setState(QGst::StatePlaying);
    }
}

void VideoStreamWidget::resetPipeline() {
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
}

void VideoStreamWidget::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    _messageLabel->move(0, 0);
    _messageLabel->resize(width(), height());
    _videoWidget->move(0, 0);
    _videoWidget->resize(width(), height());
}

bool VideoStreamWidget::isPlaying() {
    return _isPlaying;
}

void VideoStreamWidget::onBusMessage(const QGst::MessagePtr & message) {
    switch (message->type()) {
    case QGst::MessageEos:
        stop("Received end-of-stream message.");
        emit eosMessage();
        break;
    case QGst::MessageError:
        stop("Decoding Error: " + message.staticCast<QGst::ErrorMessage>()->error().message() + "<br><br>Resetting...");
        emit error();
        break;
    default:
        break;
    }
}

} // namespace MissionControl
} // namespace Soro
