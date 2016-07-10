#include "socketaddress.h"
#include <cstring>

namespace Soro {

SocketAddress::SocketAddress(const QHostAddress &address, quint16 port) {
    this->host = address;
    this->port = port;
}

SocketAddress::SocketAddress() {
    this->host = QHostAddress::Any;
    this->port = 0;
}

QString SocketAddress::toString() const {
    return host.toString() + ":" + QString::number(port);
}

bool SocketAddress::operator==(const SocketAddress& other) const {
    //In a situations where we are working with IPv4 addresses, but the :ffff: (IPv6)
    //prefix is added to only one of them, simple 'a == b' will fail
    Q_IPV6ADDR a = host.toIPv6Address();
    Q_IPV6ADDR b = other.host.toIPv6Address();
    return (std::memcmp(&a, &b, sizeof(Q_IPV6ADDR)) == 0)
            & (port == other.port);
}

QDataStream& operator<<(QDataStream& stream, const SocketAddress& address) {
    stream << address.host.toString();
    stream << address.port;

    return stream;
}

QDataStream& operator>>(QDataStream& stream, SocketAddress& address) {
    QString hostStr;

    stream >> hostStr;
    stream >> address.port;

    address.host.setAddress(hostStr);
    return stream;
}

}
