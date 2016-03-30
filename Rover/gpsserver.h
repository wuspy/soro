#ifndef GPSSERVER_H
#define GPSSERVER_H

#include <QObject>

class GpsServer : public QObject
{
    Q_OBJECT
public:
    explicit GpsServer(QObject *parent = 0);

signals:

public slots:
};

#endif // GPSSERVER_H