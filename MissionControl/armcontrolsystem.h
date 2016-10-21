#ifndef SORO_MISSIONCONTROL_ARMCONTROLSYSTEM_H
#define SORO_MISSIONCONTROL_ARMCONTROLSYSTEM_H

#include <QObject>
#include <QTimerEvent>

#include "soro_global.h"
#include "channel.h"
#include "logger.h"
#include "masterarmconfig.h"
#include "armmessage.h"
#include "controlsystem.h"
#include "mbedchannel.h"

namespace Soro {
namespace MissionControl {

class ArmControlSystem : public ControlSystem {
    Q_OBJECT
public:
    explicit ArmControlSystem(const QHostAddress& roverAddress, QObject *parent = 0);
    ~ArmControlSystem();
    bool init(QString *errorString);
    void enable();
    void disable();
    bool isMasterArmConnected() const;

public slots:
    bool reloadMasterArmConfig();

signals:
    void masterArmStateChanged(bool connected);
    void masterArmUpdate(const char *message);

protected:
    void send();

private:
    MasterArmConfig _armConfig;
    MbedChannel *_mbed = NULL;
    bool _enabled = false;
    char _buffer[256];

private slots:
    void mbedMessageReceived(const char *message, int size);
    void mbedStateChanged(MbedChannel *channel, MbedChannel::State state);
};

}
}

#endif // SORO_MISSIONCONTROL_ARMCONTROLSYSTEM_H
