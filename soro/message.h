#ifndef MESSAGE_H
#define MESSAGE_H

#include <soro_global.h>

class Message
{
public:
    const quint16 RECEIVE_SUCCESS = 0;
    const quint16 RECEIVE_TIMEOUT = 1;
    const quint16 RECEIVE_UNKNOWN_ERROR = 2;

    quint32 ID;
    QByteArray* data;
    Message();
    QByteArray serialize(Protocol protocol);
    quint16 receive(QUdpSocket socket, SocketAddress *sender);
    quint16 receive(QTcpSocket socket);
};

#endif // MESSAGE_H
