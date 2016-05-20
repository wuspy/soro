/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

#include "gimbalmessage.h"

namespace Soro {
namespace GimbalMessage {

#ifdef QT_CORE_LIB

void setGamepadData(char *message, short yawAxis, short pitchAxis, bool leftButton, bool homeButton, bool rightButton, bool armButton) {
    MbedMessageType messageType = MbedMessage_Gimbal;
    message[0] = (unsigned char)reinterpret_cast<unsigned int&>(messageType);
    message[Index_Yaw] = axisShortToAxisByte(filterGamepadDeadzone(yawAxis, GAMEPAD_DEADZONE));
    message[Index_Pitch] = axisShortToAxisByte(filterGamepadDeadzone(pitchAxis, GAMEPAD_DEADZONE));
    message[Index_LookHome] = homeButton ? 1 : 0;
    message[Index_LookLeft] = leftButton ? 1 : 0;
    message[Index_LookRight] = rightButton ? 1 : 0;
    message[Index_LookArm] = armButton ? 1 : 0;
}

#endif

}
}
