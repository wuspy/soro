#ifndef MASTERDATARECORDER_H
#define MASTERDATARECORDER_H

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
        Event_CommentEntered
    };

public slots:
    void addComment(QString comment);

private slots:
    void driveChannelStateChanged(Channel::State state);
    void sharedChannelStateChanged(Channel::State state);
};

} // namespace MissionControl
} // namespace Soro

#endif // MASTERDATARECORDER_H
