#include "glfwmap.h"

#define CONFIG_TAG_AXIS_BUTTON_SPEED_FACTOR "asbf"
#define CONFIG_TAG_CONTROLLER_NAME "controllername"
#define CONFIG_TAG_CONTROLLER_AXES "axes"
#define CONFIG_TAG_CONTROLLER_BUTTONS "buttons"

using namespace Soro;

bool GlfwMap::loadMapping(QFile& file) {
    IniParser parser;
    if (!parser.load(file)) return false;
    int tmp;
    if (!parser.valueAsInt(CONFIG_TAG_AXIS_BUTTON_SPEED_FACTOR, &tmp)
            || (tmp > 100)
            || (tmp < -100)) {
        tmp = 75; //default speed factor
    }
    AxisButtonSpeedFactor = (signed char)tmp;
    for (int i = 0; i < count(); i++) {
        if (!parser.valueAsInt((*this)[i].Name, &(*this)[i].GlfwIndex)) {
            (*this)[i].GlfwIndex = MapItem::UNMAPPED;
        }
    }
    return true;
}

bool GlfwMap::writeMapping(QFile& file) {
    IniParser parser;
    parser.insert(CONFIG_TAG_AXIS_BUTTON_SPEED_FACTOR, QString::number(AxisButtonSpeedFactor));
    for (int i = 0; i < count(); i++) {
        parser.insert((*this)[i].Name, QString::number((*this)[i].GlfwIndex));
    }
    return parser.write(file);
}

bool GlfwMap::isMapped() const {
    for (int i = 0; i < count(); i++) {
        if ((*this)[i].isMapped()) return true;
    }
    return false;
}
