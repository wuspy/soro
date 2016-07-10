#ifndef SORO_MISSIONCONTROL_CAMERACONTROLSYSTEM_H
#define SORO_MISSIONCONTROL_CAMERACONTROLSYSTEM_H

#include <QObject>
#include <QTimerEvent>

#include "soro_global.h"
#include "channel.h"
#include "configuration.h"
#include "logger.h"
#include "gamepadmanager.h"
#include "controlsystem.h"
#include "gimbalmessage.h"

namespace Soro {
namespace MissionControl {

class CameraControlSystem : public ControlSystem {
    Q_OBJECT
public:
    explicit CameraControlSystem(const Configuration *config, GamepadManager *input, QObject *parent = 0);
    bool init(QString *errorString);
    void enable();
    void disable();

protected:
    void timerEvent(QTimerEvent *e);

private:
    int _controlSendTimerId = TIMER_INACTIVE;
    const GamepadManager *_input;
    char _buffer[256];
};

}
}

#endif // SORO_MISSIONCONTROL_CAMERACONTROLSYSTEM_H
