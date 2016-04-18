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
    const int RequiredSize = 3;
    /* These list the indicies of values in a gimbal message
     */
    const int Index_Yaw = 1;
    const int Index_Pitch = 2;

#ifdef QT_CORE_LIB

    /* Fills a gimbal message with SDL gamepad data
     */
    void setGamepadData(char *message, short XAxis, short YAxis);

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

}
}


#endif // GIMBALMESSAGE_H
