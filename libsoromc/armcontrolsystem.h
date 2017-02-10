#ifndef SORO_MISSIONCONTROL_ARMCONTROLSYSTEM_H
#define SORO_MISSIONCONTROL_ARMCONTROLSYSTEM_H

#include <QObject>
#include <QTimerEvent>

#include "libsoro/channel.h"
#include "libsoro/logger.h"
#include "libsoro/masterarmconfig.h"
#include "libsoro/armmessage.h"
#include "libsoro/mbedchannel.h"

#include "soro_missioncontrol_global.h"
#include "controlsystem.h"
namespace Soro {
namespace MissionControl {

class LIBSOROMC_EXPORT ArmControlSystem : public ControlSystem {
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
    void armMessageSent(const char *message, int size);

protected:
    void send();

private:
    MasterArmConfig _armConfig;
    MbedChannel *_mbed = nullptr;
    bool _enabled = false;
    char _buffer[256];

private slots:
    void mbedMessageReceived(const char *message, int size);
    void mbedStateChanged(MbedChannel::State state);
};

}
}

#endif // SORO_MISSIONCONTROL_ARMCONTROLSYSTEM_H
