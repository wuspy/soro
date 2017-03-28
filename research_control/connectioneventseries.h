#ifndef CONNECTIONEVENTSERIES_H
#define CONNECTIONEVENTSERIES_H

#include <QObject>
#include <QTimerEvent>

#include "libsoro/csvrecorder.h"
#include "libsoro/channel.h"

namespace Soro {
namespace MissionControl {

class ConnectionEventSeries: public QObject, public CsvDataSeries
{
    Q_OBJECT
public:
    ConnectionEventSeries(const Channel* driveChannel, const Channel* sharedChannel, QObject *parent=0);
    QString getSeriesName() const;

private slots:
    void driveChannelStateChanged(Channel::State state);
    void sharedChannelStateChanged(Channel::State state);
};

} // namespace MissionControl
} // namespace Soro

#endif // CONNECTIONEVENTSERIES_H
