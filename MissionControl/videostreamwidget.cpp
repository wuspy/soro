#include "videostreamwidget.h"

namespace Soro {
namespace MissionControl {


VideoStreamWidget::VideoStreamWidget(QWidget *parent) : QWidget(parent) {
    _videoWidget = new QGst::Ui::VideoWidget(this);
    _messageLabel = new QLabel(this);

    _messageLabel->setStyleSheet("QLabel { background-color: #A0000000; color: #FFFFFF; }");
    _messageLabel->setVisible(false);
    _messageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    endStream();
}

VideoStreamWidget::~VideoStreamWidget() {
    resetPipeline();
}

void VideoStreamWidget::configure(SocketAddress address, VideoEncoding encoding) {
    resetPipeline();
    KILL_TIMER(_reconfigureTimerId);
    _messageLabel->setVisible(false);
    _address = address;
    _encoding = encoding;

    _pipeline = QGst::Pipeline::create();
    _pipeline->bus()->addSignalWatch();
    QGlib::connect(_pipeline->bus(), "message", this, &VideoStreamWidget::onBusMessage);

    QGst::ElementPtr sink = QGst::ElementFactory::make("qt5videosink");
    sink->setProperty("force-aspect-ratio", true);

    QGst::BinPtr receiver;
    switch (encoding) {
    case MJPEG:
        receiver = QGst::Bin::fromDescription("udpsrc address=" + address.host.toString() + " port=" + QString::number(address.port) + " ! "
                                               "application/x-rtp,encoding=JPEG,payload=26 ! "
                                               "rtpjpegdepay ! "
                                               "jpegdec ! "
                                               "videoconvert ! "
                                               "video/x-raw,format=RGB ! "
                                               "identity");
        break;
    case MPEG2:
        receiver = QGst::Bin::fromDescription("udpsrc address=" + address.host.toString() + " port=" + QString::number(address.port) + " ! "
                                               "application/x-rtp,media=video,clock-rate=90000,encoding-name=MP4V-ES,profile-level-id=1,payload=96,ssrc=2873740600,timestamp-offset=391825150,seqnum-offset=2980 ! "
                                               "rtpmp4vdepay ! "
                                               "avdec_mpeg4 ! "
                                               "videoconvert ! "
                                               "video/x-raw,format=RGB ! "
                                               "identity");
        break;
    }

    _pipeline->add(receiver, sink);
    receiver->link(sink);
    _videoWidget->setVideoSink(sink);

    _pipeline->setState(QGst::StatePlaying);
}

void VideoStreamWidget::endStream(QString reason) {
    resetPipeline();
    KILL_TIMER(_reconfigureTimerId);
    KILL_TIMER(_hideMessageTimerId);
    if (reason.isEmpty()) {
        _messageLabel->setText("<html><h2>No video :(</h2></html>");
    }
    else {
        _messageLabel->setText("<html><h2>No video :(</h2><br><br>" + reason + "</html>");
    }
    _messageLabel->setVisible(true);

    //create videotestsrc pipeline for coolness
    _pipeline = QGst::Pipeline::create();
    QGst::ElementPtr sink = QGst::ElementFactory::make("qt5videosink");
    QGst::ElementPtr source = QGst::ElementFactory::make("videotestsrc");
    source->setProperty("pattern", 1); //snow pattern
    sink->setProperty("force-aspect-ratio", false);

    _pipeline->add(source, sink);
    source->link(sink);
    _videoWidget->setVideoSink(sink);

    _pipeline->setState(QGst::StatePlaying);
}

void VideoStreamWidget::resetPipeline() {
    if (_pipeline) {
        _pipeline->setState(QGst::StateNull);
        _pipeline.clear();
    }
}

void VideoStreamWidget::timerEvent(QTimerEvent *e) {
    QWidget::timerEvent(e);
    if (e->timerId() == _hideMessageTimerId) {
        _messageLabel->setVisible(false);
        KILL_TIMER(_hideMessageTimerId); //single shot
    }
    else if (e->timerId() == _reconfigureTimerId) {
        configure(_address, _encoding);
        KILL_TIMER(_reconfigureTimerId);
    }
}

void VideoStreamWidget::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    _messageLabel->move(0, 0);
    _messageLabel->resize(width(), height());
    _videoWidget->move(0, 0);
    _videoWidget->resize(width(), height());
}

void VideoStreamWidget::onBusMessage(const QGst::MessagePtr & message) {
    switch (message->type()) {
    case QGst::MessageEos:
        endStream("Received end-of-stream message.");
        emit eosMessage();
        break;
    case QGst::MessageError:
        endStream("GStreamer Error: " + message.staticCast<QGst::ErrorMessage>()->error().message() + "<br><br>Resetting...");
        START_TIMER(_reconfigureTimerId, 3000);
        emit error();
        break;
    default:
        break;
    }
}

} // namespace MissionControl
} // namespace Soro
