#include "gpsserver.h"

#define LOG_TAG "GpsServer"

#define RECOVERY_DELAY 500

namespace Soro {
namespace Rover {

GpsServer::GpsServer(SocketAddress hostAddress, QObject *parent) : QObject(parent) {
    _socket = new QUdpSocket(this);
    connect(_socket, SIGNAL(readyRead()),
            this, SLOT(socketReadyRead()));
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(sockeError(QAbstractSocket::SocketError)));
    LOG_I(LOG_TAG, "Creating GPS server with host address " + hostAddress.toString());
    _hostAddress = hostAddress;
    resetConnection();
}

GpsServer::~GpsServer() {
    delete _socket;
}

void GpsServer::socketReadyRead() {
    LOG_D(LOG_TAG, "socketReadyRead() called");
    SocketAddress address;
    qint64 status;
    while (_socket->hasPendingDatagrams()) {
        //read in a datagram
        status = _socket->readDatagram(_buffer, 1024, &address.host, &address.port);
        if (status < 0) {
            //an error occurred reading from the socket, the onSocketError slot will handle it
            return;
        }
        QString nmea = QString(_buffer);
        NmeaMessage nmeaMessage(nmea);

        if ((nmeaMessage.Latitude != 0) && (nmeaMessage.Longitude != 0)) {
            LOG_I(LOG_TAG, "Received GPS packet");
            emit gpsUpdate(nmeaMessage);
        }
        else {
            LOG_W(LOG_TAG, "Received invalid GPS message on server port");
        }
    }
}

void GpsServer::sockeError(QAbstractSocket::SocketError err) {
    emit connectionError(err);
    LOG_E(LOG_TAG, "Server Error: " + _socket->errorString());
    START_TIMER(_resetConnectionTimerId, RECOVERY_DELAY);
}

void GpsServer::resetConnection() {
    _socket->abort();
    _socket->bind(_hostAddress.host, _hostAddress.port);
    _socket->open(QIODevice::ReadWrite);
}

void GpsServer::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _resetConnectionTimerId) {
        resetConnection();
        KILL_TIMER(_resetConnectionTimerId);
    }
}

} // namespace Rover
} // namespace Soro
