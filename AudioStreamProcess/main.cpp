#include <QCoreApplication>
#include <QByteArray>
#include <QDataStream>

#include <Qt5GStreamer/QGst/Init>

#include "socketaddress.h"
#include "soro_global.h"
#include "audiostreamer.h"

using namespace Soro;
using namespace Soro::Rover;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QGst::init();

    qDebug() << "Starting up";

    if (argc < 8) {
        //no arguments
        qCritical() << "Invalid arguments";
        qCritical() << "Example: AudioStreamProcess [device] [encoding] [address] [port] [bindAddress] [bindPort] [IPCPort]";
        return STREAMPROCESS_ERR_NOT_ENOUGH_ARGUMENTS;
    }

    bool ok;
    AudioFormat format;
    QString device;
    SocketAddress address;
    SocketAddress bindAddress;
    quint16 ipcPort;

    //parse device
    device = argv[1];
    //parse encoding
    quint32 encodingUInt = QString(argv[2]).toUInt(&ok);
    if (!ok) return STREAMPROCESS_ERR_INVALID_ARGUMENT;
    format = reinterpret_cast<AudioFormat&>(encodingUInt);

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

    a.setApplicationName("AudioStream for " + device + " to " + address.toString());

    qDebug() << "Setting audio device " << device;
    AudioStreamer stream(device, format, bindAddress, address, ipcPort, &a);
    qDebug() << "Stream initialized successfully";
    return a.exec();
}
