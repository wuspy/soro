#include "soroudpsocket.h"

namespace Soro {

SoroUdpSocket::SoroUdpSocket(QObject *parent) : QUdpSocket(parent) { }

qint64 SoroUdpSocket::writeDatagram(const char *data, qint64 len, const QHostAddress &host, quint16 port) {

}

}
