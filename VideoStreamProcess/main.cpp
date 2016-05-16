#include <QCoreApplication>
#include <QByteArray>
#include <QDataStream>

#include <flycapture/FlyCapture2.h>
#include <flycapcamera.h>

#include <Qt5GStreamer/QGst/Init>
#include <Qt5GStreamer/QGst/Element>
#include <Qt5GStreamer/QGst/ElementFactory>

#include "videoencoding.h"
#include "socketaddress.h"
#include "soro_global.h"
#include "streamprocess.h"
#include "flycapcamera.h"

using namespace Soro;
using namespace Soro::Rover;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QGst::init();

    qDebug() << "Starting up";

<<<<<<< HEAD
    if (argc < 10) {
        //no arguments
        qCritical() << "Invalid arguments";
        qCritical() << "Example: VideoStreamProcess [device] [encoding] [height] [bitrate/quality] [address] [port] [bindAddress] [bindPort] [IPCPort]";
=======
    if (argc < 11) {
        //no arguments
        qCritical() << "Invalid arguments";
        qCritical() << "Example: VideoStreamProcess device encoding width height framerate bitrate/quality address port bindAddress bindPort IPCPort";
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
        return STREAMPROCESS_ERR_NOT_ENOUGH_ARGUMENTS;
    }

    bool ok;
    StreamFormat format;
    QString device;
    SocketAddress address;
    SocketAddress bindAddress;
    quint16 ipcPort;

    //parse device
    device = argv[1];
    //parse encoding
    unsigned int encodingUInt = QString(argv[2]).toUInt(&ok);
    if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    format.Encoding = reinterpret_cast<VideoEncoding&>(encodingUInt);
<<<<<<< HEAD
    //parse height
    format.Height = QString(argv[3]).toInt(&ok);
=======
    //parse width
    format.Width = QString(argv[3]).toInt(&ok);
    if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    //parse height
    format.Height = QString(argv[4]).toInt(&ok);
    if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    //parse framerate
    format.Framerate = QString(argv[5]).toInt(&ok);
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
    if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;

    switch (format.Encoding) {
    case MjpegEncoding:
        //parse mjpeg quality
<<<<<<< HEAD
        format.Mjpeg_Quality = QString(argv[4]).toInt(&ok);
=======
        format.Mjpeg_Quality = QString(argv[6]).toInt(&ok);
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
        break;
    default:
        //parse bitrate
<<<<<<< HEAD
        format.Bitrate = QString(argv[4]).toInt(&ok);
=======
        format.Bitrate = QString(argv[6]).toInt(&ok);
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
        break;
    }

    //parse address/port
<<<<<<< HEAD
    address.host = QHostAddress(argv[5]);
    address.port = QString(argv[6]).toInt(&ok);
=======
    address.host = QHostAddress(argv[7]);
    address.port = QString(argv[8]).toInt(&ok);
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
    if ((address.host == QHostAddress::Null) | (address.host == QHostAddress::Any) | !ok) {
        // invalid address
        return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    }
    //parse bindAddress/bindPort
<<<<<<< HEAD
    bindAddress.host = QHostAddress(argv[7]);
    bindAddress.port = QString(argv[8]).toInt(&ok);
=======
    bindAddress.host = QHostAddress(argv[9]);
    bindAddress.port = QString(argv[10]).toInt(&ok);
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
    if ((bindAddress.host == QHostAddress::Null) | !ok) {
        // invalid host
        return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    }
<<<<<<< HEAD
    ipcPort = QString(argv[9]).toInt(&ok);
=======
    ipcPort = QString(argv[11]).toInt(&ok);
>>>>>>> 428dd76f9c40301c2fc62fa976ee6f582d8d93a4
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
        StreamProcess stream(source, format, bindAddress, address, ipcPort, &a);
        qDebug() << "Stream initialized for FlyCapture successfully";
        return a.exec();
    }
    else {
        qDebug() << "Setting UVD device " << device;
        StreamProcess stream(device, format, bindAddress, address, ipcPort, &a);
        qDebug() << "Stream initialized successfully";
        return a.exec();
    }
}
