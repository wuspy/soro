#ifndef DRIVEMESSAGE2_H
#define DRIVEMESSAGE2_H

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
#define DRIVE_MESSAGE_BLOCK_START (unsigned char)247
#define DRIVE_MESSAGE_FL_INDEX 1
#define DRIVE_MESSAGE_FR_INDEX 2
#define DRIVE_MESSAGE_ML_INDEX 3
#define DRIVE_MESSAGE_MR_INDEX 4
#define DRIVE_MESSAGE_BL_INDEX 5
#define DRIVE_MESSAGE_BR_INDEX 6

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
            message[0] = DRIVE_MESSAGE_BLOCK_START;
            if (mapping.forwardAxis().isMapped() & mapping.turnAxis().isMapped()) {
                signed char forward = joyAxisToByte(mapping.forwardAxis().value(glfwAxes, axisCount));
                signed char turn = joyAxisToByte(mapping.turnAxis().value(glfwAxes, axisCount));
                double angle = qAtan((double)forward / (double)turn);
                int diff = qAbs(turn);
                double midScale = 0.6 * (diff/100.0);
                message[DRIVE_MESSAGE_FR_INDEX] = -turn;
            }
            else {
                signed char leftDrive = joyAxisToByte(mapping.leftWheelsAxis().value(glfwAxes, axisCount));
                signed char rightDrive = joyAxisToByte(mapping.rightWheelsAxis().value(glfwAxes, axisCount));
                //scale the middle wheels by 0.6 at full turn
                int diff = qAbs((int)(leftDrive - rightDrive));
                double midScale = 0.6 * (diff/200.0);
                message[DRIVE_MESSAGE_FL_INDEX] = leftDrive;
                message[DRIVE_MESSAGE_FR_INDEX] = rightDrive;
                message[DRIVE_MESSAGE_ML_INDEX] = (signed char)(leftDrive - (midScale * leftDrive));
                message[DRIVE_MESSAGE_MR_INDEX] = (signed char)(rightDrive - (midScale * rightDrive));
                message[DRIVE_MESSAGE_BL_INDEX] = leftDrive;
                message[DRIVE_MESSAGE_BR_INDEX] = rightDrive;
            }
        }
#endif

        static inline bool hasValidHeader(const char *message) {
            return (unsigned char)message[0] == DRIVE_MESSAGE_BLOCK_START;
        }

        static inline int frontLeft(const char *message) {
            return (signed char)message[DRIVE_MESSAGE_FL_INDEX];
        }

        static inline int frontRight(const char *message) {
            return (signed char)message[DRIVE_MESSAGE_FR_INDEX];
        }

        static inline int middleLeft(const char *message) {
            return (signed char)message[DRIVE_MESSAGE_ML_INDEX];
        }

        static inline int middleRight(const char *message) {
            return (signed char)message[DRIVE_MESSAGE_MR_INDEX];
        }

        static inline int backLeft(const char *message) {
            return (signed char)message[DRIVE_MESSAGE_BL_INDEX];
        }

        static inline int backRight(const char *message) {
            return (signed char)message[DRIVE_MESSAGE_BR_INDEX];
        }
    };
}

#endif // DRIVEMESSAGE2_H
