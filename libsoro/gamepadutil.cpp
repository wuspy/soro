#include "gamepadutil.h"

#include <climits>

namespace Soro {
namespace GamepadUtil {

short filterGamepadDeadzone(short raw, float percent) {
    if (((raw > 0) && (raw < (SHRT_MAX * percent))) || ((raw < 0) && (raw > (SHRT_MIN * percent)))) {
        return 0;
    }
    return raw;
}

char axisShortToAxisByte(short val) {
    val /= (SHRT_MAX / 100);
    val += 100;
    unsigned char uc = (unsigned char)val;
    return reinterpret_cast<char&>(uc);
}

char axisFloatToAxisByte(float val) {
    val = (val + 1) * 100;
    unsigned char uc = (unsigned char)val;
    return reinterpret_cast<char&>(uc);
}

char axisIntToAxisByte(int val) {
    val += 100;
    unsigned char uc = (unsigned char)val;
    return reinterpret_cast<char&>(uc);
}

int axisFloatToAxisInt(float val) {
    return (int)(val * 100);
}

float axisByteToAxisFloat(char val) {
    return (float)((int)reinterpret_cast<unsigned char&>(val) - 100.0) / 100.0;
}

int axisByteToAxisInt(char val) {
    return (int)reinterpret_cast<unsigned char&>(val) - 100;
}

float axisShortToAxisFloat(short val) {
    return (float)val / (float)SHRT_MAX;
}

} // namespace GamepadUtil
} // namespace Soro
