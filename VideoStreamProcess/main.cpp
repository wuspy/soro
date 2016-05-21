#include <QCoreApplication>
#include <QByteArray>
#include <QDataStream>

#include <flycapture/FlyCapture2.h>
#include <flycapcamera.h>

#include <Qt5GStreamer/QGst/Init>
#include <Qt5GStreamer/QGst/Element>
#include <Qt5GStreamer/QGst/ElementFactory>

#include "socketaddress.h"
#include "soro_global.h"
#include "videostreamer.h"
#include "flycapcamera.h"

using namespace Soro;
using namespace Soro::Rover;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QGst::init();

    qDebug() << "Starting up";

    if (argc < 8) {
        //no arguments
        qCritical() << "Invalid arguments";
        qCritical() << "Example: VideoStreamProcess [device] [encoding] [address] [port] [bindAddress] [bindPort] [IPCPort]";
        return STREAMPROCESS_ERR_NOT_ENOUGH_ARGUMENTS;
    }

    bool ok;
    VideoFormat format;
    QString device;
    SocketAddress address;
    SocketAddress bindAddress;
    quint16 ipcPort;

    //parse device
    device = argv[1];
    //parse encoding
    quint32 encodingUInt = QString(argv[2]).toUInt(&ok);
    if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    format = reinterpret_cast<VideoFormat&>(encodingUInt);

    //parse address/port
    address.host = QHostAddress(argv[3]);
    address.port = QString(argv[4]).toInt(&ok);
    if ((address.host == QHostAddress::Null) | (address.host == QHostAddress::Any) | !ok) {
        // invalid address
        return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    }
    //parse bindAddress/bindPort
    bindAddress.host = QHostAddress(argv[5]);
    bindAddress.port = QString(argv[6]).toInt(&ok);
    if ((bindAddress.host == QHostAddress::Null) | !ok) {
        // invalid host
        return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    }
    ipcPort = QString(argv[7]).toInt(&ok);
    if (!ok) {
        // invalid IPC port
        return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    }

    a.setApplicationName("VideoStream for " + device + " to " + address.toString());

    if (device.startsWith("FlyCapture2:", Qt::CaseInsensitive)) {
        QGst::ElementPtr source;
        // parse GUID
        FlyCapture2::PGRGuid guid;
        device = device.mid(device.indexOf(":") + 1);
        guid.value[0] = device.mid(0, device.indexOf(":")).toUInt(&ok);
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
        device = device.mid(device.indexOf(":") + 1);
        guid.value[1] = device.mid(0, device.indexOf(":")).toUInt(&ok);
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
        device = device.mid(device.indexOf(":") + 1);
        guid.value[2] = device.mid(0, device.indexOf(":")).toUInt(&ok);
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
        device = device.mid(device.indexOf(":") + 1);
        guid.value[3] = device.toUInt(&ok);
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;

        FlycapCamera camera(guid, &a);
        source = camera.element();

        qDebug() << "Parset parameters for FlyCapture successfully";
        VideoStreamer stream(source, format, bindAddress, address, ipcPort, &a);
        qDebug() << "Stream initialized for FlyCapture successfully";
        return a.exec();
    }
    else {
        qDebug() << "Setting UVD device " << device;
        VideoStreamer stream(device, format, bindAddress, address, ipcPort, &a);
        qDebug() << "Stream initialized successfully";
        return a.exec();
    }
}
