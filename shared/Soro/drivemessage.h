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

    /* This identifies a char array as a drive message.
     * It should be unique between drive/arm/gimbal messages
     * to avoid unfortunate mistakes
     */
    const unsigned char Header = 3;
    /* The size a drive message should be
     */
    const int RequiredSize = 7;
    /* These list the indicies of values in a drive message
     */
    const int Index_FrontLeft = 1;
    const int Index_FrontRight = 2;
    const int Index_MiddleLeft = 3;
    const int Index_MiddleRight = 4;
    const int Index_BackLeft = 5;
    const int Index_BackRight = 6;

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

    /* Gets the front left wheel speed value and converts
     * it to a float ranging from -1 to 1
     */
    inline float getFrontLeft(const char *message) {
        return axisByteToAxisFloat(message[Index_FrontLeft]);
    }

    /* Gets the front right wheel speed value and converts
     * it to a float ranging from -1 to 1
     */
    inline float getFrontRight(const char *message) {
        return axisByteToAxisFloat(message[Index_FrontRight]);
    }

    /* Gets the middle left wheel speed value and converts
     * it to a float ranging from -1 to 1
     */
    inline float getMiddleLeft(const char *message) {
        return axisByteToAxisFloat(message[Index_MiddleLeft]);
    }

    /* Gets the middle right wheel speed value and converts
     * it to a float ranging from -1 to 1
     */
    inline float getMiddleRight(const char *message) {
        return axisByteToAxisFloat(message[Index_MiddleRight]);
    }

    /* Gets the back left wheel speed value and converts
     * it to a float ranging from -1 to 1
     */
    inline float getBackLeft(const char *message) {
        return axisByteToAxisFloat(message[Index_BackLeft]);
    }

    /* Gets the back right wheel speed value and converts
     * it to a float ranging from -1 to 1
     */
    inline float getBackRight(const char *message) {
        return axisByteToAxisFloat(message[Index_BackRight]);
    }
}
}

#endif // DRIVEMESSAGE_H
