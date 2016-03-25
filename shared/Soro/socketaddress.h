#ifndef SOCKETADDRESS_H
#define SOCKETADDRESS_H

#include <QHostAddress>

namespace Soro {

    /* Wrapper for a QHostAddress (IP) and quint16 (port) used to identify
     * the address of a network socket
     */
    struct SocketAddress {
        QHostAddress address;
        quint16 port;

        SocketAddress(const QHostAddress &address, quint16 port) {
            this->address = address;
            this->port = port;
        }

        SocketAddress() {
            this->address = QHostAddress::Any;
            this->port = 0;
        }

        QString toString() const {
            return address.toString() + ":" + QString::number(port);
        }

        bool operator==(const SocketAddress& other) const {
            //In a situations where we are working with IPv4 addresses, but the :ffff: (IPv6)
            //prefix is added to only one of them, simple 'a == b' will fail
            Q_IPV6ADDR a = address.toIPv6Address();
            Q_IPV6ADDR b = other.address.toIPv6Address();
            return (std::memcmp(&a, &b, sizeof(Q_IPV6ADDR)) == 0)
                    & (port == other.port);
        }

        inline bool operator!=(const SocketAddress& other) const {
            return !(*this == other);

        }



    };

}

#endif // SOCKETADDRESS_H