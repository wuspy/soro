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

    if (argc < 10) {
        //no arguments
        qCritical() << "This program cannot be run directly (got " + QString::number(argc) + " argument(s))";
        return STREAMPROCESS_ERR_NOT_ENOUGH_ARGUMENTS;
    }

    bool ok;
    StreamFormat format;
    QString device;
    SocketAddress address;
    SocketAddress host;

    //parse stream parameters
    device = argv[1];
    unsigned int encodingUInt = QString(argv[2]).toUInt(&ok);
    if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    format.Encoding = reinterpret_cast<VideoEncoding&>(encodingUInt);
    if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    format.Width = QString(argv[3]).toInt(&ok);
    if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    format.Height = QString(argv[4]).toInt(&ok);
    if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    format.Framerate = QString(argv[5]).toInt(&ok);
    if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    switch (format.Encoding) {
    case MjpegEncoding:
        format.Mjpeg_Quality = QString(argv[6]).toInt(&ok);
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
        break;
    default:
        format.Bitrate = QString(argv[6]).toInt(&ok);
        if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
        break;
    }

    address.host = QHostAddress(argv[7]);
    address.port = QString(argv[8]).toInt(&ok);
    if ((address.host == QHostAddress::Null) | (address.host == QHostAddress::Any) | !ok) {
        // invalid address
        return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    }

    host.host = QHostAddress(argv[9]);
    host.port = QString(argv[10]).toInt(&ok);
    if ((host.host == QHostAddress::Null) | (host.host == QHostAddress::Any) | !ok) {
        // invalid host
        return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    }

    QGst::ElementPtr source;

    if (device.startsWith("FlyCapture2:", Qt::CaseInsensitive)) {

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

        FlycapCamera camera(guid, format.Framerate, &a);
        source = camera.element();

        // Remove the framerate element from the format since the source
        // will manage this
        format.Framerate = 0;

        qDebug() << "Parset parameters for FlyCapture successfully";
        StreamProcess stream(source, format, host, address, &a);
        qDebug() << "Stream initialized for FlyCapture successfully";

        return a.exec();
    }
    else {
#ifdef __linux__
        source = QGst::ElementFactory::make("v4l2src");
        source->setProperty("device", device);
        qDebug() << "Setting UVD device " + device + " for v4l2src";

        qDebug() << "Parset parameters for v4l2 successfully";
        StreamProcess stream(source, format, host, address, &a);
        qDebug() << "Stream initialized for v4l2 successfully";
#endif
        return a.exec();
    }
}
