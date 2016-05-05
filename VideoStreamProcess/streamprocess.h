#ifndef STREAMPROCESS_H
#define STREAMPROCESS_H

#include <QObject>
#include <QCoreApplication>

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

#include "logger.h"

namespace Soro {
namespace Rover {

class StreamProcess : public QObject {
    Q_OBJECT
public:

    explicit StreamProcess(QGst::ElementPtr source, StreamFormat format, SocketAddress host, SocketAddress address, QObject *parent = 0);
    ~StreamProcess();

private:
    QGst::PipelinePtr _pipeline;

private slots:
    void onBusMessage(const QGst::MessagePtr & message);

signals:
    void eos();
    void error(QString message);
};

} // namespace Rover
} // namespace Soro

#endif // STREAMPROCESS_H
