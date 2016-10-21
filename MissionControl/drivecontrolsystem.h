#ifndef SORO_MISSIONCONTROL_DRIVECONTROLSYSTEM_H
#define SORO_MISSIONCONTROL_DRIVECONTROLSYSTEM_H

#include <QObject>
#include <QTimerEvent>

#include "soro_global.h"
#include "channel.h"
#include "logger.h"
#include "gamepadmanager.h"
#include "controlsystem.h"
#include "drivemessage.h"

namespace Soro {
namespace MissionControl {

class DriveControlSystem : public ControlSystem {
    Q_OBJECT
public:
    explicit DriveControlSystem(const QHostAddress& roverAddress, GamepadManager *input, QObject *parent = 0);
    bool init(QString *errorString);
    void enable();
    void disable();
    void setMode(DriveGamepadMode mode);
    void setMiddleSkidFactor(float factor);

protected:
    void timerEvent(QTimerEvent *e);

private:
    DriveGamepadMode _mode = DualStickDrive;
    int _controlSendTimerId = TIMER_INACTIVE;
    float _midSkidFactor = 0.2; //The higher this is, the slower the middle wheels turn while skid steering
    const GamepadManager *_input = NULL;
    char _buffer[256];
};

}
}

#endif // SORO_MISSIONCONTROL_DRIVECONTROLSYSTEM_H
