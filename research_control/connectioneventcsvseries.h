#ifndef CONNECTIONEVENTCSVSERIES_H
#define CONNECTIONEVENTCSVSERIES_H

#include <QObject>
#include <QTimerEvent>

#include "libsoro/csvrecorder.h"
#include "libsoro/channel.h"

namespace Soro {
namespace MissionControl {

class ConnectionEventCsvSeries: public QObject, public CsvDataSeries
{
    Q_OBJECT
public:
    ConnectionEventCsvSeries(const Channel* driveChannel, const Channel* sharedChannel, QObject *parent=0);
    QString getSeriesName() const;
    bool shouldKeepOldValues() const;

private slots:
    void driveChannelStateChanged(Channel::State state);
    void sharedChannelStateChanged(Channel::State state);
};

} // namespace MissionControl
} // namespace Soro

#endif // CONNECTIONEVENTCSVSERIES_H
