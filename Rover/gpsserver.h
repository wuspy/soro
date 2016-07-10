#ifndef SORO_ROVER_GPSSERVER_H
#define SORO_ROVER_GPSSERVER_H

#include <QtCore>
#include <QObject>
#include <QUdpSocket>

#include "logger.h"
#include "soro_global.h"
#include "socketaddress.h"
#include "nmeamessage.h"

namespace Soro {
namespace Rover {

/* Listens for NMEA formatted GPS messages on a UDP socket
 */
class GpsServer : public QObject {
    Q_OBJECT
public:
    explicit GpsServer(SocketAddress hostAddress, QObject *parent = NULL);
    ~GpsServer();

private:
    QUdpSocket *_socket;
    char _buffer[1024];
    void resetConnection();
    SocketAddress _hostAddress;
    int _resetConnectionTimerId = TIMER_INACTIVE;

signals:
    void connectionError(QAbstractSocket::SocketError err);
    void gpsUpdate(NmeaMessage update);

private slots:
    void socketReadyRead();
    void sockeError(QAbstractSocket::SocketError err);

protected:
    void timerEvent(QTimerEvent *e);
};

} // namespace Rover
} // namespace Soro

#endif // SORO_ROVER_GPSSERVER_H
