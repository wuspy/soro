/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

#ifndef GIMBALMESSAGE_H
#define GIMBALMESSAGE_H

#ifdef TARGET_LPC1768
#   include "mbed.h"
#endif  //QT_CORE_LIB

#include "soro_global.h"

namespace Soro {

/* Defines the structure of a gimbal movement command for the rover
 */
namespace GimbalMessage  {

    /* This identifies a char array as a gimbal message.
     * It should be unique between drive/arm/gimbal messages
     * to avoid unfortunate mistakes
     */
    const char Header = 4;
    /* The size a gimbal message should be
     */
    const int RequiredSize = 6;
    /* These list the indicies of values in a gimbal message
     */
    const int Index_Yaw = 1;
    const int Index_Pitch = 2;
    const int Index_LookHome = 3;
    const int Index_LookLeft = 4;
    const int Index_LookRight = 5;

#ifdef QT_CORE_LIB

    /* Fills a gimbal message with SDL gamepad data
     */
    void setGamepadData(char *message, short XAxis, short YAxis, bool xButton, bool yButton, bool bButton);

#endif

    /* Gets the pitch value in a gimbal message and
     * converts it to a float ranging from -1 to 1
     */
    inline float getPitch(const char *message) {
        return axisByteToAxisFloat(message[Index_Pitch]);
    }

    /* Gets the yaw value in a gimbal message and
     * converts it to a float ranging from -1 to 1
     */
    inline float getYaw(const char *message) {
        return axisByteToAxisFloat(message[Index_Yaw]);
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

}
}


#endif // GIMBALMESSAGE_H
