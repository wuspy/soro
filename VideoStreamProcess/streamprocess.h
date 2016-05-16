#ifndef STREAMPROCESS_H
#define STREAMPROCESS_H

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
#include "videoencoding.h"

#include "logger.h"

namespace Soro {
namespace Rover {

class StreamProcess : public QObject {
    Q_OBJECT
public:

    StreamProcess(QGst::ElementPtr source, StreamFormat format, SocketAddress bindAddress, SocketAddress address, quint16 ipcPort, QObject *parent = 0);
    StreamProcess(QString deviceName, StreamFormat format, SocketAddress bindAddress, SocketAddress address, quint16 ipcPort, QObject *parent = 0);
    ~StreamProcess();

private:
    QGst::PipelinePtr _pipeline;
    QGst::PipelinePtr createPipeline();
    QTcpSocket *_ipcSocket = NULL;

    bool connectToParent(quint16 port);
    QString makeEncodingBinString(StreamFormat format, SocketAddress bindAddress, SocketAddress address);
    void stop();

private slots:
    void onBusMessage(const QGst::MessagePtr & message);
    void ipcSocketReadyRead();
<<<<<<< HEAD
    void ipcSocketError(QAbstractSocket::SocketError error);
    void ipcSocketDisconnected();
=======
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4

signals:
    void eos();
    void error(QString message);
};

} // namespace Rover
} // namespace Soro

#endif // STREAMPROCESS_H
