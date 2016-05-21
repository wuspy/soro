#ifndef MEDIASTREAMER_H
#define MEDIASTREAMER_H

#include <QObject>
#include <QCoreApplication>
#include <QTcpSocket>

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

#include "logger.h"

namespace Soro {
namespace Rover {

class MediaStreamer : public QObject {
    Q_OBJECT
public:
    ~MediaStreamer();

protected:
    MediaStreamer(QObject *parent = 0);

    QGst::PipelinePtr _pipeline;
    QGst::PipelinePtr createPipeline();
    QTcpSocket *_ipcSocket = NULL;

    bool connectToParent(quint16 port);
    void stop();

private slots:
    void onBusMessage(const QGst::MessagePtr & message);
    void ipcSocketReadyRead();
    void ipcSocketError(QAbstractSocket::SocketError error);
    void ipcSocketDisconnected();

signals:
    void eos();
    void error(QString message);
};

} // namespace Rover
} // namespace Soro

#endif // MEDIASTREAMER_H
