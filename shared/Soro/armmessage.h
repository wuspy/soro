#ifndef ARMMESSAGE_H
#define ARMMESSAGE_H

#ifdef QT_CORE_LIB
#   include <QByteArray>
#   include <QSerialPort>
#   include "soroutil.h"
#   include "armglfwmap.h"
#   include "iniparser.h"
#else
#   include "mbed.h"
#endif  //QT_CORE_LIB

#include "serialinterop.h"

//Indicies for data specified in BOTH joystick and master/slave movement messages
#define ARM_MESSAGE_BUCKET_FULL_OPEN_INDEX 1
#define ARM_MESSAGE_BUCKET_FULL_CLOSE_INDEX 2
#define ARM_MESSAGE_STOW_MACRO_INDEX 3
#define ARM_MESSAGE_MACRO1_INDEX 4
#define ARM_MESSAGE_MACRO2_INDEX 5
#define ARM_MESSAGE_MACRO3_INDEX 6
#define ARM_MESSAGE_MACRO4_INDEX 7
#define ARM_MESSAGE_RECORD_MACRO_INDEX 8

//Indicies and info for the structure of a joystick arm message
#define ARM_JOY_MESSAGE_SIZE 14
#define ARM_JOY_MESSAGE_BLOCK_START (unsigned char)249
#define ARM_JOY_MESSAGE_X_INDEX 9
#define ARM_JOY_MESSAGE_Y_INDEX 10
#define ARM_JOY_MESSAGE_YAW_INDEX 11
#define ARM_JOY_MESSAGE_WRIST_INDEX 12
#define ARM_JOY_MESSAGE_BUCKET_INDEX 13

//Indicies and info for the structure of a master/slave arm message
#define ARM_MASTER_MESSAGE_SIZE 19
#define ARM_MASTER_MESSAGE_BLOCK_START (unsigned char)248
#define ARM_MASTER_MESSAGE_YAW_INDEX 9
#define ARM_MASTER_MESSAGE_SHOULDER_INDEX 11
#define ARM_MASTER_MESSAGE_ELBOW_INDEX 13
#define ARM_MASTER_MESSAGE_WRIST_INDEX 15
#define ARM_MASTER_MESSAGE_BUCKET_INDEX 17

#define MARINI_TAG_YAWMIN "yawmin"
#define MARINI_TAG_YAWMAX "yawmax"
#define MARINI_TAG_YAWADD "yawadd"
#define MARINI_TAG_SHOULDERMIN "shouldermin"
#define MARINI_TAG_SHOULDERMAX "shouldermax"
#define MARINI_TAG_SHOULDERADD "shoulderadd"
#define MARINI_TAG_ELBOWMIN "elbowmin"
#define MARINI_TAG_ELBOWMAX "elbowmax"
#define MARINI_TAG_ELBOWADD "elbowadd"
#define MARINI_TAG_WRISTMIN "wristmin"
#define MARINI_TAG_WRISTMAX "wristmax"
#define MARINI_TAG_WRISTADD "wristadd"
#define MARINI_TAG_BUCKETMIN "bucketmin"
#define MARINI_TAG_BUCKETMAX "bucketmax"
#define MARINI_TAG_BUCKETADD "bucketadd"

#define ARM_SERIAL_CHANNEL_NAME "arm-serial-channel"
#define MASTER_ARM_SERIAL_CHANNEL_NAME "master-arm-serial-channel"

namespace Soro {

    /* Defines the structure of a joystick OR master/slave movement command for the arm.
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
    class ArmMessage  {
    public:
        enum ArmMessageType {
            JOYSTICK,
            MASTER_SLAVE,
            INVALID
        };

#ifdef QT_CORE_LIB

        /* Loads a master arm configuration from a file
         */
        struct MasterRanges {
            int yawMax, yawMin, yawAdd;
            int shoulderMax, shoulderMin, shoulderAdd;
            int elbowMax, elbowMin, elbowAdd;
            int wristMax, wristMin, wristAdd;
            int bucketMax, bucketMin, bucketAdd;

            bool load(QFile& file) {
                IniParser parser;
                if (!parser.load(file)) return false;
                bool success = parser.valueAsInt(MARINI_TAG_YAWMIN, &yawMin);
                success &= parser.valueAsInt(MARINI_TAG_YAWMAX, &yawMax);
                success &= parser.valueAsInt(MARINI_TAG_YAWADD, &yawAdd);
                success &= parser.valueAsInt(MARINI_TAG_SHOULDERMIN, &shoulderMin);
                success &= parser.valueAsInt(MARINI_TAG_SHOULDERMAX, &shoulderMax);
                success &= parser.valueAsInt(MARINI_TAG_SHOULDERADD, &shoulderAdd);
                success &= parser.valueAsInt(MARINI_TAG_ELBOWMIN, &elbowMin);
                success &= parser.valueAsInt(MARINI_TAG_ELBOWMAX, &elbowMax);
                success &= parser.valueAsInt(MARINI_TAG_ELBOWADD, &elbowAdd);
                return success;
            }
        };

        /* Fills a message with instructions based on data from glfw joystick data and an input map
         */
        static void setGlfwData(char *message, const float *glfwAxes, const unsigned char *glfwButtons,
                              int axisCount, int buttonCount, ArmGlfwMap& mapping) {
            message[0] = ARM_JOY_MESSAGE_BLOCK_START;  //identify this message as glfw and not master
            message[ARM_JOY_MESSAGE_X_INDEX] = joyAxisToByte(mapping.xAxis().value(glfwAxes, axisCount));
            message[ARM_JOY_MESSAGE_Y_INDEX] = joyAxisToByte(mapping.yAxis().value(glfwAxes, axisCount));
            message[ARM_JOY_MESSAGE_YAW_INDEX] = joyAxisToByte(mapping.yawAxis().value(glfwAxes, axisCount));
            message[ARM_JOY_MESSAGE_WRIST_INDEX] = joyAxisToByte(mapping.wristAxis().value(glfwAxes, axisCount));
            message[ARM_JOY_MESSAGE_BUCKET_INDEX] = joyAxisToByte(mapping.bucketAxis().value(glfwAxes, axisCount));
            if (mapping.wristUpButton().isPressed(glfwButtons, buttonCount)) {
                message[ARM_JOY_MESSAGE_WRIST_INDEX] = 0.7;
            }
            else if (mapping.wristDownButton().isPressed(glfwButtons, buttonCount)) {
                message[ARM_JOY_MESSAGE_WRIST_INDEX] = -0.7;
            }
            if (mapping.bucketOpenButton().isPressed(glfwButtons, buttonCount)) {
                message[ARM_JOY_MESSAGE_BUCKET_INDEX] = 0.7;
            }
            else if (mapping.bucketCloseButton().isPressed(glfwButtons, buttonCount)) {
                message[ARM_JOY_MESSAGE_BUCKET_INDEX] = -0.7;
            }
            message[ARM_MESSAGE_BUCKET_FULL_OPEN_INDEX] =
                    mapping.bucketFullOpenButton().isPressed(glfwButtons, buttonCount) ? 1 : 0;
            message[ARM_MESSAGE_BUCKET_FULL_CLOSE_INDEX] =
                    mapping.bucketFullCloseButton().isPressed(glfwButtons, buttonCount) ? 1 : 0;
            message[ARM_MESSAGE_STOW_MACRO_INDEX] =
                    mapping.stowButton().isPressed(glfwButtons, buttonCount) ? 1 : 0;
        }

        /* Translates a master arm message from potentiometer values to servo percentages that the slave arm can use
         */
        static void translateMasterArmValues(char *message, const MasterRanges& ranges) {
            //yaw
            int yaw = ranges.yawAdd + MAX_VALUE_14BIT * (((float)deserialize_14bit(message, ARM_MASTER_MESSAGE_YAW_INDEX) - (float)ranges.yawMin) / (float)(ranges.yawMax - ranges.yawMin));
            if (yaw > MAX_VALUE_14BIT) yaw = MAX_VALUE_14BIT;
            if (yaw < 0) yaw = 0;
            serialize_14bit((unsigned short)yaw, message, ARM_MASTER_MESSAGE_YAW_INDEX);
            //shoulder
            int shoulder = ranges.shoulderAdd + MAX_VALUE_14BIT * (((float)deserialize_14bit(message, ARM_MASTER_MESSAGE_SHOULDER_INDEX) - (float)ranges.shoulderMin) / (float)(ranges.shoulderMax - ranges.shoulderMin));
            if (shoulder > MAX_VALUE_14BIT) shoulder = MAX_VALUE_14BIT;
            if (shoulder < 0) shoulder = 0;
            serialize_14bit((unsigned short)shoulder, message, ARM_MASTER_MESSAGE_SHOULDER_INDEX);
            //elbow
            int elbow = ranges.elbowAdd + MAX_VALUE_14BIT * (((float)deserialize_14bit(message, ARM_MASTER_MESSAGE_ELBOW_INDEX) - (float)ranges.elbowMin) / (float)(ranges.elbowMax - ranges.elbowMin));
            if (elbow > MAX_VALUE_14BIT) elbow = MAX_VALUE_14BIT;
            if (elbow < 0) elbow = 0;
            serialize_14bit((unsigned short)elbow, message, ARM_MASTER_MESSAGE_ELBOW_INDEX);
            serialize_14bit((unsigned short)3000, message, ARM_MASTER_MESSAGE_WRIST_INDEX); //TODO bill build the fucking master arm
            serialize_14bit((unsigned short)8000, message, ARM_MASTER_MESSAGE_BUCKET_INDEX);
        }

#endif
#ifdef TARGET_LPC1768
        /* Fills an arm message with instructions based on readings from a master arm
         */
        static inline void setMasterArmData(char *message, unsigned short yaw, unsigned short shoulder, unsigned short elbow,
                                    unsigned short wrist, unsigned short bucket) {
            message[0] = ARM_MASTER_MESSAGE_BLOCK_START;  //identify this message as master and not glfw
            serialize_14bit(yaw, message, ARM_MASTER_MESSAGE_YAW_INDEX);
            serialize_14bit(shoulder, message, ARM_MASTER_MESSAGE_SHOULDER_INDEX);
            serialize_14bit(elbow, message, ARM_MASTER_MESSAGE_ELBOW_INDEX);
            serialize_14bit(wrist, message, ARM_MASTER_MESSAGE_WRIST_INDEX);
            serialize_14bit(bucket, message, ARM_MASTER_MESSAGE_BUCKET_INDEX);
        }

#endif

        static inline ArmMessageType messageType(const char *message) {
            switch ((unsigned char)message[0]) {
            case ARM_MASTER_MESSAGE_BLOCK_START:
                return MASTER_SLAVE;
            case ARM_JOY_MESSAGE_BLOCK_START:
                return JOYSTICK;
            default: return INVALID;
            }
        }

        static inline bool hasValidHeader(const char *message) {
            return messageType(message) != INVALID;
        }

        static inline int size(const char *message) {
            switch (messageType(message)) {
            case MASTER_SLAVE:
                return ARM_MASTER_MESSAGE_SIZE;
            case JOYSTICK:
                return ARM_JOY_MESSAGE_SIZE;
            default:
                return -1;
            }
        }

        static inline signed char joyX(const char *message) {
            return (signed char)message[ARM_JOY_MESSAGE_X_INDEX];
        }

        static inline signed char joyY(const char *message) {
            return (signed char)message[ARM_JOY_MESSAGE_Y_INDEX];
        }

        static inline signed char joyYaw(const char *message) {
            return (signed char)message[ARM_JOY_MESSAGE_YAW_INDEX];
        }

        static inline signed char joyWrist(const char *message) {
            return (signed char)message[ARM_JOY_MESSAGE_WRIST_INDEX];
        }

        static inline signed char joyBucket(const char *message) {
            return (signed char)message[ARM_JOY_MESSAGE_BUCKET_INDEX];
        }

        static inline unsigned short masterYaw(const char *message) {
            return deserialize_14bit(message, ARM_MASTER_MESSAGE_YAW_INDEX);
        }

        static inline unsigned short masterShoulder(const char *message) {
            return deserialize_14bit(message, ARM_MASTER_MESSAGE_SHOULDER_INDEX);
        }

        static inline unsigned short masterElbow(const char *message) {
            return deserialize_14bit(message, ARM_MASTER_MESSAGE_ELBOW_INDEX);
        }

        static inline unsigned short masterWrist(const char *message) {
            return deserialize_14bit(message, ARM_MASTER_MESSAGE_WRIST_INDEX);
        }

        static inline unsigned short masterBucket(const char *message) {
            return deserialize_14bit(message, ARM_MASTER_MESSAGE_BUCKET_INDEX);
        }

        static inline bool bucketFullOpen(const char *message) {
            return ((unsigned char)message[ARM_MESSAGE_BUCKET_FULL_OPEN_INDEX]) == 1;
        }

        static inline bool bucketFullClose(const char *message) {
            return ((unsigned char)message[ARM_MESSAGE_BUCKET_FULL_CLOSE_INDEX]) == 1;
        }

        static inline bool stowMacro(const char *message) {
            return ((unsigned char)message[ARM_MESSAGE_STOW_MACRO_INDEX]) == 1;
        }

        static inline bool macro1(const char *message) {
            return ((unsigned char)message[ARM_MESSAGE_MACRO1_INDEX]) == 1;
        }

        static inline bool macro2(const char *message) {
            return ((unsigned char)message[ARM_MESSAGE_MACRO2_INDEX]) == 1;
        }

        static inline bool macro3(const char *message) {
            return ((unsigned char)message[ARM_MESSAGE_MACRO3_INDEX]) == 1;
        }

        static inline bool macro4(const char *message) {
            return ((unsigned char)message[ARM_MESSAGE_MACRO4_INDEX]) == 1;
        }

        static inline bool recordMacro(const char *message) {
            return ((unsigned char)message[ARM_MESSAGE_RECORD_MACRO_INDEX]) == 1;
        }
    };
}

#endif // ARMMESSAGE_H
