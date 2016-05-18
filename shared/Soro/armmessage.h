/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

#ifndef ARMMESSAGE_H
#define ARMMESSAGE_H

#ifdef QT_CORE_LIB
#   include "iniparser.h"
#   include "masterarmconfig.h"
#else
#   include "mbed.h"
#endif  //QT_CORE_LIB

#include "soro_global.h"

namespace Soro {

/* Defines the structure of a joystick OR master/slave movement command for the arm.
 */
namespace ArmMessage  {

    /* These identify the type of message (arm gamepad or arm master) and are the first char.
     * They should also be unique between drive/arm/gimbal messages
     * to avoid unfortunate mistakes
     */
    const unsigned char Header_Gamepad = 1;
    const unsigned char Header_Master = 2;

    /* The size each type of arm message should be
     */
    const int RequiredSize_Gamepad = 8;
    const int RequiredSize_Master = 11;
    /* These list the indicies of values in an arm message
     */
    const int Index_CloseBucket = 1;
    const int Index_OpenBucket = 2;
    const int Index_Stow = 3;
    const int Index_GamepadYaw = 4;
    const int Index_GamepadX = 5;
    const int Index_GamepadY = 6;
    const int Index_GamepadWrist = 7;
    const int Index_MasterYaw = 4;
    const int Index_MasterShoulder = 6;
    const int Index_MasterElbow = 8;
    const int Index_MasterWrist = 10;

#ifdef QT_CORE_LIB
    /* Fills an arm message with SDL gamepad data
     */
    void setGamepadData(char *armMessage, short leftXAxis, short leftYAxis, short rightYAxis,
                               short leftTriggerAxis, short rightTriggerAxis, bool rightButton, bool leftButton, bool yButton);

    /* Translates a master arm message from potentiometer values to servo values that the slave arm can use.
     * Requires a MasterRanges mapping to do this.
     */
    void translateMasterArmValues(char *message, const MasterArmConfig& ranges);

#endif
#ifdef TARGET_LPC1768
    /* Fills an arm message with instructions based on readings from a master arm
     */
    void setMasterArmData(char *message, unsigned short yaw, unsigned short shoulder,
                                        unsigned short elbow, unsigned short wrist, bool stow);
#endif

    /* Gets the X speed value in a gamepad arm message, and
     * converts it to a float ranging from -1 to 1
     */
    inline float getGamepadX(const char *message) {
        return axisByteToAxisFloat(message[Index_GamepadX]);
    }

    /* Gets the Y speed value in a gamepad arm message, and
     * converts it to a float ranging from -1 to 1
     */
    inline float getGamepadY(const char *message) {
        return axisByteToAxisFloat(message[Index_GamepadY]);
    }

    /* Gets the yaw speed value in a gamepad arm message, and
     * converts it to a float ranging from -1 to 1
     */
    inline float getGamepadYaw(const char *message) {
        return axisByteToAxisFloat(message[Index_GamepadYaw]);
    }

    /* Gets the wrist speed value in a gamepad arm message, and
     * converts it to a float ranging from -1 to 1
     */
    inline float getGamepadWrist(const char *message) {
        return axisByteToAxisFloat(message[Index_GamepadWrist]);
    }

    /* Gets the master arm's yaw value in a master message, as an
     * unsigned short ranging from 0 to USHRT_MAX
     */
    inline unsigned short getMasterYaw(const char *message) {
        return deserialize<unsigned short>(message + Index_MasterYaw);
    }

    /* Gets the master arm's shoulder value in a master message, as an
     * unsigned short ranging from 0 to USHRT_MAX
     */
    inline unsigned short getMasterShoulder(const char *message) {
        return deserialize<unsigned short>(message + Index_MasterShoulder);
    }

    /* Gets the master arm's elbow value in a master message, as an
     * unsigned short ranging from 0 to USHRT_MAX
     */
    inline unsigned short getMasterElbow(const char *message) {
        return deserialize<unsigned short>(message + Index_MasterElbow);
    }

    /* Gets the master arm's wrist value in a master message, as an
     * unsigned short ranging from 0 to USHRT_MAX
     */
    inline unsigned short getMasterWrist(const char *message) {
        return deserialize<unsigned short>(message + Index_MasterWrist);
    }

    /* Gets the state of the bucket open command in an arm message.
     * This applies to both gamepad and master arm messages.
     */
    inline bool getBucketOpen(const char *message) {
        return message[Index_OpenBucket] == 1;
    }

    /* Gets the state of the bucket close command in an arm message.
     * This applies to both gamepad and master arm messages.
     */
    inline bool getBucketClose(const char *message) {
        return message[Index_CloseBucket] == 1;
    }

    /* Gets the state of the stow command in an arm message.
     * This applies to both gamepad and master arm messages.
     */
    inline bool getStow(const char *message) {
        return message[Index_Stow] == 1;
    }
}

}

#endif // ARMMESSAGE_H
