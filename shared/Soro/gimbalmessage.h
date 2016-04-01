#ifndef GIMBALMESSAGE_H
#define GIMBALMESSAGE_H

#ifdef QT_CORE_LIB
#   include <QByteArray>
#   include <QSerialPort>
#   include "soroutil.h"
#   include "gimbalglfwmap.h"
#else
#   include "mbed.h"
#endif  //QT_CORE_LIB

#include "serialchannel3.h"

//Indicies and info for the structure of a drive serial message
#define GIMBAL_MESSAGE_SIZE 4
#define GIMBAL_MESSAGE_ID (unsigned char)246
#define GIMBAL_MESSAGE_ROLL_INDEX 1
#define GIMBAL_MESSAGE_PITCH_INDEX 2
#define GIMBAL_MESSAGE_YAW_INDEX 3

#define GIMBAL_SERIAL_CHANNEL_NAME "gimbal-serial-channel"

namespace Soro {

/* Defines the structure of a gimbal movement command for the rover
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
class GimbalMessage  {
public:
#ifdef QT_CORE_LIB

    /* Fills a message with instructions based on data from glfw joystick data and an input map
     */
    static void setGlfwData(char *message, const float *glfwAxes, const unsigned char *glfwButtons,
                          int axisCount, int buttonCount, GimbalGlfwMap& mapping) {
        message[0] = GIMBAL_MESSAGE_ID;
        message[GIMBAL_MESSAGE_PITCH_INDEX] = joyFloatToByte(mapping.pitchAxis().value(glfwAxes, axisCount));
        message[GIMBAL_MESSAGE_PITCH_INDEX] = joyFloatToByte(mapping.rollAxis().value(glfwAxes, axisCount));
        message[GIMBAL_MESSAGE_PITCH_INDEX] = joyFloatToByte(mapping.yawAxis().value(glfwAxes, axisCount));
    }
#endif

    static inline bool hasValidHeader(const char *message) {
        return (unsigned char)message[0] == GIMBAL_MESSAGE_ID;
    }

    static inline int size(const char *message) {
        if (hasValidHeader(message)) return GIMBAL_MESSAGE_SIZE;
        return -1;
    }

    static inline int pitch(const char *message) {
        return joyByteToInt(message[GIMBAL_MESSAGE_PITCH_INDEX]);
    }

    static inline int roll(const char *message) {
        return joyByteToInt(message[GIMBAL_MESSAGE_ROLL_INDEX]);
    }

    static inline int yaw(const char *message) {
        return joyByteToInt(message[GIMBAL_MESSAGE_YAW_INDEX]);
    }
};
}


#endif // GIMBALMESSAGE_H
