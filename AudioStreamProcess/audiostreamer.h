#ifndef AUDIOSTREAMER_H
#define AUDIOSTREAMER_H

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
#include "mediastreamer.h"

namespace Soro {
namespace Rover {

class AudioStreamer : public MediaStreamer {
    Q_OBJECT
public:

    AudioStreamer(QString deviceName, AudioFormat format, SocketAddress bindAddress, SocketAddress address, quint16 ipcPort, QObject *parent = 0);

private:
    QString makeEncodingBinString(AudioFormat format, SocketAddress bindAddress, SocketAddress address);
};

} // namespace Rover
} // namespace Soro

#endif // AUDIOSTREAMER_H
