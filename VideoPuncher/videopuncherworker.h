#ifndef VIDEOPUNCHERWORKER_H
#define VIDEOPUNCHERWORKER_H

#include <QObject>
#include <QtCore>
#include <QUdpSocket>
#include <QHostAddress>
#include <QFile>

#include "iniparser.h"
#include "logger.h"
#include "socketaddress.h"
#include "soro_global.h"

namespace Soro {

class VideoPuncherWorker : public QObject
{
    Q_OBJECT
public:
    explicit VideoPuncherWorker(QObject *parent = 0);

private:
    QUdpSocket *_inSocket;
    QUdpSocket *_outSocket;
    quint16 _lhPort;
    quint16 _inPort;
    SocketAddress _remoteAddress;
    char _buffer[100000];
    int _punchTimerId = TIMER_INACTIVE;
signals:

private slots:
    void lhSocketError(QAbstractSocket::SocketError err);
    void inSocketError(QAbstractSocket::SocketError err);
    void inReadyRead();

protected:
    void timerEvent(QTimerEvent *e);
};

}

#endif // VIDEOPUNCHERWORKER_H
