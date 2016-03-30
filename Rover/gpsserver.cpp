#include "gpsserver.h"

#define LOG_TAG "GpsServer"

using namespace Soro;

GpsServer::GpsServer(QObject *parent, SocketAddress hostAddress, Logger *log) : QObject(parent) {
    _socket = new QUdpSocket(this);
    _log = log;
    connect(_socket, SIGNAL(readyRead()),
            this, SLOT(socketReadyRead()));
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(sockeError(QAbstractSocket::SocketError)));
    _socket->bind(hostAddress.address, hostAddress.port);
    _socket->open(QIODevice::ReadWrite);
}

void GpsServer::socketReadyRead() {
    LOG_D("socketReadyRead() called");
    SocketAddress address;
    qint64 status;
    while (_socket->hasPendingDatagrams()) {
        //read in a datagram
        status = _socket->readDatagram(_buffer, 1024, &address.address, &address.port);
        if (status < 0) {
            //an error occurred reading from the socket, the onSocketError slot will handle it
            return;
        }
        if (_buffer[0] != '$') {
            LOG_W("Received non-NMEA message from gps provider");
            return;
        }
        QString nmea = QString(_buffer);
        LOG_I(nmea);
    }
}

void GpsServer::sockeError(QAbstractSocket::SocketError err) {

}

void GpsServer::resetConnection() {
    if (_socket->isOpen()) _socket->close();
}

void GpsServer::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _resetConnectionTimerId) {
        resetConnection();
        KILL_TIMER(_resetConnectionTimerId);
    }
}

