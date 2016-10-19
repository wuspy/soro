/*
 * Copyright 2016 The University of Oklahoma.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
