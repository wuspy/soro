#include "gpsserver.h"

#define LOG_TAG "GpsServer"

#define RECOVERY_DELAY 500

using namespace Soro;

GpsServer::GpsServer(QObject *parent, SocketAddress hostAddress, Logger *log) : QObject(parent) {
    _socket = new QUdpSocket(this);
    _log = log;
    connect(_socket, SIGNAL(readyRead()),
            this, SLOT(socketReadyRead()));
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(sockeError(QAbstractSocket::SocketError)));
    LOG_I("Creating GPS server with host address " + hostAddress.toString());
    _hostAddress = hostAddress;
    resetConnection();
}

GpsServer::~GpsServer() {
    delete _socket;
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
    emit connectionError(err);
    LOG_E("Server Error: " + _socket->errorString());
    START_TIMER(_resetConnectionTimerId, RECOVERY_DELAY);
}

void GpsServer::resetConnection() {
    _socket->abort();
    _socket->bind(_hostAddress.address, _hostAddress.port);
    _socket->open(QIODevice::ReadWrite);

}

void GpsServer::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _resetConnectionTimerId) {
        resetConnection();
        KILL_TIMER(_resetConnectionTimerId);
    }
}

