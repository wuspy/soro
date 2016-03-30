#ifndef GPSSERVER_H
#define GPSSERVER_H

#include <QtCore>
#include <QObject>
#include <QUdpSocket>

#include "logger.h"
#include "soroutil.h"
#include "socketaddress.h"

namespace Soro {

class GpsServer : public QObject {
    Q_OBJECT
public:
    explicit GpsServer(QObject *parent = NULL, SocketAddress hostAddress = SocketAddress(QHostAddress::Any, 8999), Logger *log = NULL);

private:
    QUdpSocket *_socket;
    Logger *_log;
    char _buffer[1024];
    void resetConnection();
    int _resetConnectionTimerId = TIMER_INACTIVE;

signals:

private slots:
    void socketReadyRead();
    void sockeError(QAbstractSocket::SocketError err);

protected:
    void timerEvent(QTimerEvent *e);
};

}

#endif // GPSSERVER_H
