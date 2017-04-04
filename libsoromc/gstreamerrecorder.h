#ifndef GSTREAMERRECORDER_H
#define GSTREAMERRECORDER_H

#include <QObject>

#include "libsoro/videoformat.h"
#include "libsoro/socketaddress.h"

#include <Qt5GStreamer/QGst/Pipeline>
#include <Qt5GStreamer/QGst/Bin>

namespace Soro {
namespace MissionControl {

class GStreamerRecorder : public QObject
{
    Q_OBJECT
public:
    explicit GStreamerRecorder(QObject *parent = 0);

    void begin(VideoFormat format, SocketAddress addresss);
    void stop();

signals:

public slots:

private:
    QGst::PipelinePtr _pipeline;
    QGst::BinPtr _bin;

};

} // namespace MissionControl
} // namespace Soro

#endif // GSTREAMERRECORDER_H
