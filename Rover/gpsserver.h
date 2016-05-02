#ifndef GPSSERVER_H
#define GPSSERVER_H

#include <QtCore>
#include <QObject>
#include <QUdpSocket>

#include "logger.h"
#include "soro_global.h"
#include "socketaddress.h"
#include "latlng.h"

namespace Soro {
namespace Rover {

class GpsServer : public QObject {
    Q_OBJECT
public:
    explicit GpsServer(QObject *parent = NULL, SocketAddress hostAddress = SocketAddress(QHostAddress::Any, 8999), Logger *log = NULL);
    ~GpsServer();

private:
    QUdpSocket *_socket;
    Logger *_log;
    char _buffer[1024];
    void resetConnection();
    SocketAddress _hostAddress;
    int _resetConnectionTimerId = TIMER_INACTIVE;

signals:
    void connectionError(QAbstractSocket::SocketError err);
    void gpsUpdate(LatLng coordinates);

private slots:
    void socketReadyRead();
    void sockeError(QAbstractSocket::SocketError err);

protected:
    void timerEvent(QTimerEvent *e);
};

} // namespace Rover
} // namespace Soro

#endif // GPSSERVER_H
