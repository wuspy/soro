/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

#ifndef SORO_GIMBALMESSAGE_H
#define SORO_GIMBALMESSAGE_H

#include "gamepadutil.h"

namespace Soro {

/* Defines the structure of a gimbal movement command for the rover
 */
namespace GimbalMessage  {

    /* The size a gimbal message should be
     */
    const int RequiredSize = 7;
    /* These list the indicies of values in a gimbal message
     */
    const int Index_Yaw = 1;
    const int Index_Pitch = 2;
    const int Index_LookHome = 3;
    const int Index_LookLeft = 4;
    const int Index_LookRight = 5;
    const int Index_LookArm = 6;

#ifdef QT_CORE_LIB

    /* Fills a gimbal message with SDL gamepad data
     */
    void setGamepadData(char *message, short yawAxis, short pitchAxis, bool leftButton, bool homeButton, bool rightButton, bool armButton);

#endif

    /* Gets the pitch value in a gimbal message and
     * converts it to a float ranging from -1 to 1
     */
    inline float getPitch(const char *message) {
        return GamepadUtil::axisByteToAxisFloat(message[Index_Pitch]);
    }

    /* Gets the yaw value in a gimbal message and
     * converts it to a float ranging from -1 to 1
     */
    inline float getYaw(const char *message) {
        return GamepadUtil::axisByteToAxisFloat(message[Index_Yaw]);
    }

    inline bool getLookHome(const char *message) {
        return message[Index_LookHome] == 1;
    }

    inline bool getLookLeft(const char *message) {
        return message[Index_LookLeft] == 1;
    }

    inline bool getLookRight(const char *message) {
        return message[Index_LookRight] == 1;
    }

    inline bool getLookArm(const char *message) {
        return message[Index_LookArm] == 1;
    }
}
}


#endif // SORO_GIMBALMESSAGE_H
