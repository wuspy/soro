#ifndef GPSLOGGER_H
#define GPSLOGGER_H

#include <QObject>
#include <QFile>
#include <QDateTime>

#include "nmeamessage.h"
#include "abstractdatarecorder.h"

namespace Soro {

class LIBSORO_EXPORT GpsDataRecorder : public AbstractDataRecorder
{
    Q_OBJECT
public:
    explicit GpsDataRecorder(QObject *parent = 0);

public slots:
    void addLocation(NmeaMessage location);
};

} // namespace Soro

#endif // GPSLOGGER_H
