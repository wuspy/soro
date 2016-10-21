#ifndef CONTROLSYSTEM_H
#define CONTROLSYSTEM_H

#include <QObject>
#include <QTimerEvent>
#include <QHostAddress>

#include "soro_global.h"
#include "channel.h"

namespace Soro {
namespace MissionControl {

class ControlSystem : public QObject {
    Q_OBJECT

private:
    QHostAddress _roverAddress;

public:
    virtual void enable()=0;
    virtual void disable()=0;
    virtual bool init(QString *errorString)=0;

    ~ControlSystem();

signals:
    void connectionStateChanged(bool connected);

protected:
    Channel *_channel = NULL;

    bool init(QString channelName, quint16 channelPort, QString *errorString);
    explicit ControlSystem(const QHostAddress& roverAddress, QObject *parent = 0);

protected slots:
    void channelStateChanged(Channel *channel, Channel::State state);

public:
    inline const Channel* getChannel() const {
        return _channel;
    }

    inline Channel* getChannel() {
        return _channel;
    }
};

}
}

#endif // CONTROLSYSTEM_H
