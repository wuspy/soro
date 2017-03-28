#ifndef GPSLOGGER_H
#define GPSLOGGER_H

#include <QObject>
#include <QFile>
#include <QDateTime>

#include "soro_global.h"
#include "nmeamessage.h"
#include "csvrecorder.h"

namespace Soro {

class LIBSORO_EXPORT GpsDataSeries : public QObject
{
    Q_OBJECT
public:
    explicit GpsDataSeries(QObject *parent = 0);

    class LatitudeSeries : public CsvDataSeries { friend class GpsDataSeries;
    public:     QString getSeriesName() const { return "GPS Latitude"; }
    private:    void update(NmeaMessage location) { CsvDataSeries::update(QVariant(location.Latitude)); }
    };
    class LongitudeSeries : public CsvDataSeries { friend class GpsDataSeries;
    public:     QString getSeriesName() const { return "GPS Longitude"; }
    private:    void update(NmeaMessage location) { CsvDataSeries::update(QVariant(location.Longitude)); }
    };

    const LatitudeSeries* getLatitudeSeries() const;
    const LongitudeSeries* getLongitudeSeries() const;

signals:
    void locationUpdated(NmeaMessage location);

public slots:
    void addLocation(NmeaMessage location);

private:
    LatitudeSeries _latitudeSeries;
    LongitudeSeries _longitudeSeries;
};

} // namespace Soro

#endif // GPSLOGGER_H
