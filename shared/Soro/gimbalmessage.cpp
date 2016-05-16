/*********************************************************
 * This code can be compiled on a Qt or mbed enviornment *
 *********************************************************/

#include "gimbalmessage.h"

namespace Soro {
namespace GimbalMessage {

#ifdef QT_CORE_LIB

void setGamepadData(char *message, short XAxis, short YAxis, bool accelerateButton, bool homeButton) {
    message[0] = Header;
    message[Index_Yaw] = axisShortToAxisByte(XAxis);
    message[Index_Pitch] = axisShortToAxisByte(YAxis);
    message[Index_Home] = homeButton ? 1 : 0;
    message[Index_Accelerate] = accelerateButton ? 1 : 0;
}

#endif

}
}
