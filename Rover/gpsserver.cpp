#include "gpsserver.h"

#define LOG_TAG "GpsServer"

#define RECOVERY_DELAY 500

namespace Soro {
namespace Rover {

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
        status = _socket->readDatagram(_buffer, 1024, &address.host, &address.port);
        if (status < 0) {
            //an error occurred reading from the socket, the onSocketError slot will handle it
            return;
        }
        QString nmea = QString(_buffer);
        NmeaMessage nmeaMessage;
        LOG_I("Received packet");
        int ggaPos = nmea.indexOf("$GPGGA,");
        int vtgPos = nmea.indexOf("$GPVTG,");
        if (ggaPos >= 0) {
            QStringList ggaList = nmea.mid(ggaPos).split(",");
            QString time = ggaList[1];
            QString latitude = ggaList[2];
            QString latDirection = ggaList[3];
            QString longitude = ggaList[4];
            QString lngDirection = ggaList[5];
            QString fixQuality = ggaList[6];
            QString satellites = ggaList[7];
            QString hdop = ggaList[8];
            QString altitude = ggaList[9];
            QString altitudeUnits = ggaList[10];

            nmeaMessage.Latitude = latitude.isEmpty() ? -1 :
                                            (latitude.mid(0, latitude.indexOf(".") - 2).toDouble()
                                                        + (latitude.mid(latitude.indexOf(".") - 2).toDouble() / 60.0));
            nmeaMessage.Longitude = longitude.isEmpty() ? -1 :
                                            (longitude.mid(0, longitude.indexOf(".") - 2).toDouble()
                                                        + (longitude.mid(longitude.indexOf(".") - 2).toDouble() / 60.0));
            nmeaMessage.Longitude = -nmeaMessage.Longitude;

            nmeaMessage.Satellites = satellites.isEmpty() ? 0 : satellites.toInt();
            nmeaMessage.Altitude = altitude.isEmpty() ? -1 :  qRound(altitude.toDouble());
        }
        else {
            nmeaMessage.Latitude = 0;
            nmeaMessage.Longitude = 0;
            nmeaMessage.Satellites = 0;
            nmeaMessage.Altitude = -1;
        }
        if (vtgPos >= 0) {
            QStringList vtgList = nmea.mid(vtgPos).split(",");
            QString trackTrueNorth = vtgList[1];
            QString trackMagNorth = vtgList[3];
            QString groundSpeedKnots = vtgList[5];
            QString groundSpeedKph = vtgList[7];

            nmeaMessage.Heading = trackMagNorth.isEmpty() ? -1 : qRound(trackMagNorth.toDouble());
            nmeaMessage.GroundSpeed = groundSpeedKnots.isEmpty() ? -1 : qRound(groundSpeedKph.toDouble());
        }
        else {
            nmeaMessage.Heading = -1;
            nmeaMessage.GroundSpeed = -1;
        }
        emit gpsUpdate(nmeaMessage);
    }
}

void GpsServer::sockeError(QAbstractSocket::SocketError err) {
    emit connectionError(err);
    LOG_E("Server Error: " + _socket->errorString());
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
