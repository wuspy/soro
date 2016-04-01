#include "glfwmap.h"

#define CONFIG_TAG_CONTROLLER_NAME "ControllerName"
#define CONFIG_TAG_CLASS_TYPE "Classtype"

using namespace Soro;

GlfwMap::~GlfwMap() {
    if (AxisList !=NULL) delete[] AxisList;
    if (ButtonList != NULL) delete[] ButtonList;
}

bool GlfwMap::loadMapping(QFile& file) {
    if (!file.exists()) return false;
    IniParser parser;
    if (!parser.load(file)
            || (_classname != parser.value(CONFIG_TAG_CLASS_TYPE))) return false;
    ControllerName = parser.value(CONFIG_TAG_CONTROLLER_NAME);
    int axes = axisCount();
    for(int i = 0; i < axes; i++) {
        if (!parser.valueAsInt("A" + QString::number(i), &AxisList[i].GlfwIndex)) {
            AxisList[i].reset();
        }
    }
    int buttons = buttonCount();
    for(int i = 0; i < buttons; i++) {
        if (!parser.valueAsInt("B" + QString::number(i), &ButtonList[i].GlfwIndex)) {
            ButtonList[i].reset();
        }
    }
    return true;
}

void GlfwMap::reset() {
    if (AxisList != NULL) {
        int s = axisCount();
        for(int i = 0; i < s; i++) {
            AxisList[i].reset();
        }
    }
    if (ButtonList != NULL) {
        int s = buttonCount();
        for(int i = 0; i < s; i++) {
            ButtonList[i].reset();
        }
    }
}

bool GlfwMap::writeMapping(QFile& file) {
    if (!isMapped()) return false;
    IniParser parser;
    parser.insert(CONFIG_TAG_CLASS_TYPE, _classname);
    parser.insert(CONFIG_TAG_CONTROLLER_NAME, ControllerName);
    int axes = axisCount();
    for (int i = 0; i < axes; i++) {
        if (AxisList[i].isMapped()) {
            parser.insert("A" + QString::number(i), QString::number(AxisList[i].GlfwIndex));
        }
    }
    int buttons = buttonCount();
    for (int i = 0; i < buttons; i++) {
        if (ButtonList[i].isMapped()) {
            parser.insert("B" + QString::number(i), QString::number(ButtonList[i].GlfwIndex));
        }
    }
    return parser.write(file);
}

bool GlfwMap::isMapped() const {
    int axes = axisCount();
    for (int i = 0; i < axes; i++) {
        if (AxisList[i].isMapped()) return true;
    }
    int buttons = buttonCount();
    for (int i = 0; i < buttons; i++) {
        if (ButtonList[i].isMapped()) return true;
    }
    return false;
}
