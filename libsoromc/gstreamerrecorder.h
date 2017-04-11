#ifndef GSTREAMERRECORDER_H
#define GSTREAMERRECORDER_H

#include <QObject>

#include "libsoro/videoformat.h"
#include "libsoro/socketaddress.h"
#include "libsoro/socketaddress.h"

#include <Qt5GStreamer/QGst/Pipeline>
#include <Qt5GStreamer/QGst/Bin>

namespace Soro {
namespace MissionControl {

class GStreamerRecorder : public QObject
{
    Q_OBJECT
public:
    explicit GStreamerRecorder(SocketAddress mediaAddress, QString name, QObject *parent=0);

    void begin(const MediaFormat* format, qint64 timestamp);
    void stop();

private:
    QGst::PipelinePtr _pipeline;
    QGst::BinPtr _bin;
    QString _name;
    SocketAddress _mediaAddress;

};

} // namespace MissionControl
} // namespace Soro

#endif // GSTREAMERRECORDER_H
