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

#include "libsoro/socketaddress.h"
#include "soro_gst_global.h"

namespace Soro {
namespace Gst {

/**
 * Uses a gstreamer backend to stream media to a remote address. This class does not run in the main process,
 * instead it runs in a child process is controlled by a corresponding MediaServer in the main process.
 */
class LIBSOROGST_EXPORT MediaStreamer : public QObject {
    Q_OBJECT
public:
    ~MediaStreamer();

protected:
    MediaStreamer(QString LOG_TAG, QObject *parent = 0);

    QGst::PipelinePtr _pipeline;
    QGst::PipelinePtr createPipeline();
    QTcpSocket *_ipcSocket = NULL;
    QString LOG_TAG;

    /**
     * Should be called by the subclass when it is ready to connect to the parent MediaServer
     * @param port The IPC port to connect on
     */
    bool connectToParent(quint16 port);
    void stop();

private slots:
    void onBusMessage(const QGst::MessagePtr & message);
    void ipcSocketReadyRead();
    void ipcSocketError(QAbstractSocket::SocketError error);
    void ipcSocketDisconnected();

signals:
    /**
     * Signal emitted when an EOS messages is received by gstreamer
     */
    void eos();
    /**
     * Signal emitted when an error message i
     * @param message The error message
     */
    void error(QString message);
};

} // namespace Gst
} // namespace Soro

#endif // MEDIASTREAMER_H