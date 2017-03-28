#ifndef LATENCYSERIES_H
#define LATENCYSERIES_H

#include <QObject>
#include <QFile>
#include <QDateTime>

#include "libsoro/nmeamessage.h"
#include "libsoro/csvrecorder.h"

namespace Soro {
namespace MissionControl {

class LatencySeries : public QObject
{
    Q_OBJECT
public:
    explicit LatencySeries(QObject *parent = 0);

    class RealLatencySeries : public CsvDataSeries { friend class LatencySeries;
    public: QString getSeriesName() const { return "Actual Latency"; }
    };
    class SimulatedLatencySeries : public CsvDataSeries { friend class LatencySeries;
    public: QString getSeriesName() const { return "Simulated Latency"; }
    };

    const RealLatencySeries* getRealLatencySeries() const;
    const SimulatedLatencySeries* getSimulatedLatencySeries() const;

public slots:
    void updateRealLatency(int latency);
    void updateSimulatedLatency(int latency);

private:
    RealLatencySeries _realLatencySeries;
    SimulatedLatencySeries _simulatedLatencySeries;
};

} // namespace MissionControl
} // namespace Soro

#endif // LATENCYSERIES_H
