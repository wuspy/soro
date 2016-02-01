#include "message.h"

Message::Message()
{

}

quint16 Message::receive(QUdpSocket socket, SocketAddress *sender, qint64 maxLength) {
    int status = socket.readDatagram(buffer, maxLength, &sender->address, &sender->port);
    if (status < 0) {
        //an error occurred reading from the socket
        break;
    }
    //check that the datagram came fromt the correct address
    if ((rcvAddress.address != _remote ? _address.address : _remoteAddress.address)
            | (rcvAddress.port != _remote ? _address.port : _remoteAddress.port)) {
        //datagram from unknown address
    }
    else {
        processMessage(unpackageDatagram(QByteArray::fromRawData(buffer, udpLength)));
    }
}

quint16 Message::receive(QTcpSocket socket) {

}
