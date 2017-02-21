#ifndef MASTERDATARECORDER_H
#define MASTERDATARECORDER_H

#include <QTimerEvent>

#include "libsoro/abstractdatarecorder.h"
#include "libsoro/channel.h"

namespace Soro {
namespace MissionControl {

class MasterDataRecorder: public AbstractDataRecorder
{
    Q_OBJECT

public:
    MasterDataRecorder(const Channel* driveChannel, const Channel* sharedChannel, QObject *parent=0);

    enum Event {
        Event_DriveChannelConnected,
        Event_DriveChannelDisconnected,
        Event_SharedChannelConnected,
        Event_SharedChannelDisconnected,
        Event_CommentEntered,
        Event_RealLatencyUpdate,
        Event_FakeLatencyUpdate
    };

protected:
    void timerEvent(QTimerEvent *event);

public slots:
    void addComment(QString comment);
    void fakeLatencyUpdated(int latency);

private slots:
    void driveChannelStateChanged(Channel::State state);
    void sharedChannelStateChanged(Channel::State state);

private:
    int _latencyPollTimerId = TIMER_INACTIVE;
    const Channel *_driveChannel;
    const Channel *_sharedChannel;
};

} // namespace MissionControl
} // namespace Soro

#endif // MASTERDATARECORDER_H
