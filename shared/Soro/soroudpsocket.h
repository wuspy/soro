#ifndef SOROUDPSOCKET_H
#define SOROUDPSOCKET_H

#include <QUdpSocket>

namespace Soro {

class SoroUdpSocket: public QUdpSocket {
    Q_OBJECT
public:
    explicit SoroUdpSocket(QObject *parent=0);

    qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &host, quint16 port);
    inline qint64 writeDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port)
        { return writeDatagram(datagram.constData(), datagram.size(), host, port); }

private:


};

}

#endif // SOROUDPSOCKET_H
