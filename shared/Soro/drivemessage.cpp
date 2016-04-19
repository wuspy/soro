/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

#include "drivemessage.h"

namespace Soro {
namespace DriveMessage {

#ifdef QT_CORE_LIB

void clampMiddleSkidSteerFactor(float& factor) {
    if (factor > 1.0) factor = 1.0;
    else if (factor < 0.0) factor = 0.0;
}

void setGamepadData_DualStick(char *driveMessage, short leftYAxis, short rightYAxis,
                           float middleSkidSteerFactor) {
    driveMessage[0] = reinterpret_cast<const char&>(Header);
    clampMiddleSkidSteerFactor(middleSkidSteerFactor);

    float midScale = middleSkidSteerFactor * ((float)qAbs(leftYAxis - rightYAxis)/SHRT_MAX);
    driveMessage[Index_FrontLeft] = axisShortToAxisByte(leftYAxis);
    driveMessage[Index_FrontRight] = axisShortToAxisByte(rightYAxis);
    driveMessage[Index_MiddleLeft] = axisShortToAxisByte(leftYAxis - (short)(midScale * leftYAxis));
    driveMessage[Index_MiddleRight] = axisShortToAxisByte(rightYAxis - (short)(midScale * rightYAxis));
    driveMessage[Index_BackLeft] = axisShortToAxisByte(leftYAxis);
    driveMessage[Index_BackRight] = axisShortToAxisByte(rightYAxis);
}

void setGamepadData_SingleStick(char *driveMessage, short XAxis, short YAxis,
                           float middleSkidSteerFactor) {
    driveMessage[0] = reinterpret_cast<const char&>(Header);
    clampMiddleSkidSteerFactor(middleSkidSteerFactor);

    float y = axisShortToAxisFloat(YAxis);
    float x =  -axisShortToAxisFloat(XAxis);
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
        left = -left;
        right = -right;
    }
    if (left > 1) left = 1;
    else if (left < -1) left = -1;
    if (right > 1) right = 1;
    else if (right < -1) right = -1;

    float midScale = middleSkidSteerFactor * (qAbs(x)/1.0);

    driveMessage[Index_FrontLeft] = axisFloatToAxisByte(left);
    driveMessage[Index_FrontRight] = axisFloatToAxisByte(right);
    driveMessage[Index_MiddleLeft] = axisFloatToAxisByte(left - (midScale * left));
    driveMessage[Index_MiddleRight] = axisFloatToAxisByte(right - (midScale * right));
    driveMessage[Index_BackLeft] = axisFloatToAxisByte(left);
    driveMessage[Index_BackRight] = axisFloatToAxisByte(right);
}

#endif

}
}
