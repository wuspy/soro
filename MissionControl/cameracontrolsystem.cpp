#include "cameracontrolsystem.h"

namespace Soro {
namespace MissionControl {

CameraControlSystem::CameraControlSystem(const Configuration *config, GamepadManager *input, QObject *parent) : ControlSystem(config, parent) {
    _input = input;
}

bool CameraControlSystem::init(QString *errorString) {
    if (!_input) {
        if (errorString) *errorString = QString("The gamepad input handler did not initialize successfully.");
        return false;
    }
    return ControlSystem::init(CHANNEL_NAME_GIMBAL, _config->GimbalChannelPort, errorString);
}

void CameraControlSystem::enable() {
    START_TIMER(_controlSendTimerId, 50);
}

void CameraControlSystem::disable() {
    KILL_TIMER(_controlSendTimerId);
}

void CameraControlSystem::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if ((e->timerId() == _controlSendTimerId) && _input->isGamepadConnected()) {
        //send the rover a camera control packet
        GimbalMessage::setGamepadData(_buffer, _input->axisLeftX, _input->axisRightY,
                                      _input->buttonX, _input->buttonY, _input->buttonB, _input->buttonA);
        qDebug() << QString::number(_input->axisLeftX);
        _channel->sendMessage(_buffer, GimbalMessage::RequiredSize);
    }
}

}
}
