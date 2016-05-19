/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

#ifndef DRIVEMESSAGE_H
#define DRIVEMESSAGE_H

#ifdef QT_CORE_LIB
#   include <QtCore>
#else
#   include "mbed.h"
#endif  //TARGET_LPC1768

#include "soro_global.h"

namespace Soro {

/* Defines the structure of a drive movement command.
 */
namespace DriveMessage  {

    /* The size a drive message should be
     */
    const int RequiredSize = 5;
    /* These list the indicies of values in a drive message
     */
    const int Index_LeftOuter = 1;
    const int Index_LeftMiddle = 2;
    const int Index_RightOuter = 3;
    const int Index_RightMiddle = 4;

#ifdef QT_CORE_LIB

    /* Fills a drive message with SDL gamepad data.
     * This version maps the Y axes for the left and right sticks
     * to the left and right sides of the rover (like a zero-turn lawnmower)
     */
    void setGamepadData_DualStick(char *driveMessage, short leftYAxis, short rightYAxis,
                               float middleSkidSteerFactor);

    /* Fills a drive message with SDL gamepad data.
     * This version maps movement to a single analog stick.
     */
    void setGamepadData_SingleStick(char *driveMessage, short XAxis, short YAxis,
                               float middleSkidSteerFactor);
#endif

    /* Gets the left outer wheels speed value and converts
     * it to a float ranging from -1 to 1
     */
    inline float getLeftOuter(const char *message) {
        return axisByteToAxisFloat(message[Index_LeftOuter]);
    }

    /* Gets the left middle wheel speed value and converts
     * it to a float ranging from -1 to 1
     */
    inline float getLeftMiddle(const char *message) {
        return axisByteToAxisFloat(message[Index_LeftMiddle]);
    }

    /* Gets the right outer wheels speed value and converts
     * it to a float ranging from -1 to 1
     */
    inline float getRightOuter(const char *message) {
        return axisByteToAxisFloat(message[Index_RightOuter]);
    }

    /* Gets the right middle wheel speed value and converts
     * it to a float ranging from -1 to 1
     */
    inline float getRightMiddle(const char *message) {
        return axisByteToAxisFloat(message[Index_RightMiddle]);
    }
}
}

#endif // DRIVEMESSAGE_H
