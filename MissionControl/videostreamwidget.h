#ifndef VIDEOSTREAMWIDGET_H
#define VIDEOSTREAMWIDGET_H


#include <QWidget>
#include <QLabel>
#include <QTimerEvent>
#include <QResizeEvent>

#include <Qt5GStreamer/QGst/Ui/VideoWidget>
#include <Qt5GStreamer/QGst/Pipeline>
#include <Qt5GStreamer/QGst/Element>
#include <Qt5GStreamer/QGst/ElementFactory>
#include <Qt5GStreamer/QGst/Bin>
#include <Qt5GStreamer/QGst/Bus>
#include <Qt5GStreamer/QGlib/RefPointer>
#include <Qt5GStreamer/QGlib/Error>
#include <Qt5GStreamer/QGlib/Connect>
#include <Qt5GStreamer/QGst/Message>

#include "socketaddress.h"
#include "soro_global.h"
#include "videoencoding.h"

namespace Soro {
namespace MissionControl {

class VideoStreamWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoStreamWidget(QWidget *parent = 0);
    ~VideoStreamWidget();

    /* Configure the widget to receive a video stream. If succesful,
     * the widget should start playing the stream immediately.
     */
    void configure(SocketAddress address, VideoEncoding encoding);

    /* Stops video playback, and displays they reason why
     * if one is provided.
     */
    void endStream(QString reason = "");

private:
    QGst::PipelinePtr _pipeline;
    QGst::Ui::VideoWidget *_videoWidget;
    QLabel *_messageLabel;
    int _hideMessageTimerId = TIMER_INACTIVE;
    int _reconfigureTimerId = TIMER_INACTIVE;
    SocketAddress _address;
    VideoEncoding _encoding;

    void resetPipeline();

private slots:
    /* Recieves messages from the gstreamer pipeline bus
     */
    void onBusMessage(const QGst::MessagePtr & message);

protected:
    void timerEvent(QTimerEvent *e);
    void resizeEvent(QResizeEvent *e);

signals:
    /* Emitted when the widget receives an end-of-stream message
     */
    void eosMessage();

    /* Emitted when the widget encounters a playback error
     */
    void error();
};

} // namespace MissionControl
} // namespace Soro

#endif // VIDEOSTREAMWIDGET_H
