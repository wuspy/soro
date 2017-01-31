#ifndef GPSLOGGER_H
#define GPSLOGGER_H

#include <QObject>
#include <QFile>
#include <QDateTime>

#include "nmeamessage.h"

namespace Soro {

class GpsLogger : public QObject
{
    Q_OBJECT
public:
    explicit GpsLogger(QObject *parent = 0);

    /* Starts logging data in the specified file, and calculates all timestamps offset from
     * the provided start time.
     */
    bool startLog(QString file, QDateTime loggedStartTime=QDateTime::currentDateTime());

    /* Stops logging, if it is currently active.
     */
    void stopLog();

public slots:
    void addLocation(NmeaMessage location);

private:
    QFile *_file = NULL;
    QDataStream *_fileStream = NULL;
    qint64 _logStartTime;
};

} // namespace Soro

#endif // GPSLOGGER_H
