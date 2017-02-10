#ifndef SORO_MISSIONCONTROL_CAMERACONTROLSYSTEM_H
#define SORO_MISSIONCONTROL_CAMERACONTROLSYSTEM_H

#include <QObject>
#include <QTimerEvent>

#include "libsoro/channel.h"
#include "libsoro/logger.h"
#include "libsoro/gimbalmessage.h"

#include "soro_missioncontrol_global.h"

#include "gamepadmanager.h"
#include "controlsystem.h"

namespace Soro {
namespace MissionControl {

class LIBSOROMC_EXPORT CameraControlSystem : public ControlSystem {
    Q_OBJECT
public:
    explicit CameraControlSystem(const QHostAddress& roverAddress, GamepadManager *input, QObject *parent = 0);
    bool init(QString *errorString);
    void enable();
    void disable();

signals:
    void cameraMessageSent(const char *message, int size);

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
