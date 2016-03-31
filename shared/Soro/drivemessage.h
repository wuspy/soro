#ifndef DRIVEMESSAGE_H
#define DRIVEMESSAGE_H

#ifdef QT_CORE_LIB
#   include <QByteArray>
#   include <QSerialPort>
#   include "soroutil.h"
#   include "driveglfwmap.h"
#else
#   include "mbed.h"
#endif  //QT_CORE_LIB

//Indicies and info for the structure of a drive serial message
#define DRIVE_MESSAGE_SIZE 7
#define DRIVE_MESSAGE_ID (unsigned char)247
#define DRIVE_MESSAGE_FL_INDEX 1
#define DRIVE_MESSAGE_FR_INDEX 2
#define DRIVE_MESSAGE_ML_INDEX 3
#define DRIVE_MESSAGE_MR_INDEX 4
#define DRIVE_MESSAGE_BL_INDEX 5
#define DRIVE_MESSAGE_BR_INDEX 6

#define DRIVE_FL_SIGN 1
#define DRIVE_FR_SIGN -1
#define DRIVE_ML_SIGN 1
#define DRIVE_MR_SIGN -1
#define DRIVE_BL_SIGN 1
#define DRIVE_BR_SIGN -1


#define DRIVE_SERIAL_CHANNEL_NAME "drive-serial-channel"

namespace Soro {

/* Defines the structure of a drivetrain command for the rover
 *
 * This is not an instanced class, it only contains static methods that can be used to
 * manipulate and read from char arrays.
 *
 * This header can be compiled on a Qt or mbed enviornment, and is intended to
 * be shared between the two to have the same code handling communication on both ends
 * of the serial connection.
 *
 * If compiled with Qt, it can also be used to translate glfw joystick information and
 * extends QByteArray for sending and receiving across a network.
 *
 * Compiled on an mbed enviornment, it will still mimic a byte array with the at(i)
 * function and provides a function to read from a serial port.
 *
 * To read the input from glfw, you will need to supply a controller map specifying
 * the index in glfw output arrays corresponding to each command.
 *
 * Joystick messages are quite different than master/slave ones, but they can be
 * identified by different header bytes and message length.
 */
class DriveMessage  {
public:
#ifdef QT_CORE_LIB

    /* Fills a message with instructions based on data from glfw joystick data and an input map
     */
    static void setGlfwData(char *message, const float *glfwAxes, const unsigned char *glfwButtons,
                          int axisCount, int buttonCount, DriveGlfwMap& mapping) {
        message[0] = DRIVE_MESSAGE_ID;
        if (mapping.forwardAxis().isMapped() & mapping.turnAxis().isMapped()) {
            float y = (float)joyAxisToByte(mapping.forwardAxis().value(glfwAxes, axisCount));
            float x = 1 - (float)joyAxisToByte(mapping.turnAxis().value(glfwAxes, axisCount));
            float right, left;

            // First hypotenuse
            float z = sqrt(x*x + y*y);
            // angle in radians
            float rad = qAcos(qAbs(x)/z);
            // and in degrees
            float angle = rad*180/3.1415926;

            // Now angle indicates the measure of turn
            // Along a straight line, with an angle o, the turn co-efficient is same
            // this applies for angles between 0-90, with angle 0 the co-eff is -1
            // with angle 45, the co-efficient is 0 and with angle 90, it is 1
            float tcoeff = -1 + (angle/90)*2;
            float turn = tcoeff * qAbs(qAbs(y) - qAbs(x));

            // And max of y or x is the movement
            float move = qMax(qAbs(y), qAbs(x));

            // First and third quadrant
            if(((x >= 0) & (y >= 0)) | ((x < 0) &  (y < 0))) {
                left = move;
                right = turn;
            } else {
                right = move;
                left = turn;
            }

            // Reverse polarity
            if(y < 0) {
                left = 0 - left;
                right = 0 - right;
            }
            if (left > 100) left = 100;
            else if (left < -100) left = -100;
            if (right > 100) right = 100;
            else if (right < -100) right = -100;

            float midScale = 0.6 * (qAbs(turn)/100.0);

            message[DRIVE_MESSAGE_FL_INDEX] = DRIVE_FL_SIGN * (signed char)left;
            message[DRIVE_MESSAGE_FR_INDEX] = DRIVE_FR_SIGN * (signed char)right;
            message[DRIVE_MESSAGE_ML_INDEX] = DRIVE_ML_SIGN * (signed char)(left - (midScale * left));
            message[DRIVE_MESSAGE_MR_INDEX] = DRIVE_MR_SIGN * (signed char)(right - (midScale * right));
            message[DRIVE_MESSAGE_BL_INDEX] = DRIVE_BL_SIGN * (signed char)left;
            message[DRIVE_MESSAGE_BR_INDEX] = DRIVE_BR_SIGN * (signed char)right;
        }
        else {
            signed char leftDrive = joyAxisToByte(mapping.leftWheelsAxis().value(glfwAxes, axisCount));
            signed char rightDrive = joyAxisToByte(mapping.rightWheelsAxis().value(glfwAxes, axisCount));
            //scale the middle wheels by 0.6 at full turn
            int diff = qAbs((int)(leftDrive - rightDrive));
            float midScale = 0.6 * (diff/200.0);
            message[DRIVE_MESSAGE_FL_INDEX] = DRIVE_FL_SIGN * leftDrive;
            message[DRIVE_MESSAGE_FR_INDEX] = DRIVE_FR_SIGN * rightDrive;
            message[DRIVE_MESSAGE_ML_INDEX] = DRIVE_ML_SIGN * (signed char)(leftDrive - (midScale * leftDrive));
            message[DRIVE_MESSAGE_MR_INDEX] = DRIVE_MR_SIGN * (signed char)(rightDrive - (midScale * rightDrive));
            message[DRIVE_MESSAGE_BL_INDEX] = DRIVE_BL_SIGN * leftDrive;
            message[DRIVE_MESSAGE_BR_INDEX] = DRIVE_BR_SIGN * rightDrive;
        }
    }
#endif

    static inline bool hasValidHeader(const char *message) {
        return (unsigned char)message[0] == DRIVE_MESSAGE_ID;
    }

    static inline int size(const char *message) {
        if (hasValidHeader(message)) return DRIVE_MESSAGE_SIZE;
        return -1;
    }

    static inline signed char frontLeft(const char *message) {
        return (signed char)message[DRIVE_MESSAGE_FL_INDEX];
    }

    static inline signed char frontRight(const char *message) {
        return (signed char)message[DRIVE_MESSAGE_FR_INDEX];
    }

    static inline signed char middleLeft(const char *message) {
        return (signed char)message[DRIVE_MESSAGE_ML_INDEX];
    }

    static inline signed char middleRight(const char *message) {
        return (signed char)message[DRIVE_MESSAGE_MR_INDEX];
    }

    static inline signed char backLeft(const char *message) {
        return (signed char)message[DRIVE_MESSAGE_BL_INDEX];
    }

    static inline signed char backRight(const char *message) {
        return (signed char)message[DRIVE_MESSAGE_BR_INDEX];
    }
};

}

#endif // DRIVEMESSAGE_H
