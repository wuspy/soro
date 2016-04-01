#include "gimbalglfwmap.h"

using namespace Soro;

GimbalGlfwMap::GimbalGlfwMap() {
    _classname = "GimbalGlfwMap";
    AxisList = new AxisMapItem[3] {
        //Axis maps
        AxisMapItem("Pitch"),
        AxisMapItem("Roll"),
        AxisMapItem("Yaw"),
    };
}

int GimbalGlfwMap::axisCount() const {
    return 3;
}

int GimbalGlfwMap::buttonCount() const {
    return 0;
}

const GlfwMap::AxisMapItem& GimbalGlfwMap::pitchAxis() const {
    return AxisList[0];
}

const GlfwMap::AxisMapItem& GimbalGlfwMap::rollAxis() const {
    return AxisList[1];
}

const GlfwMap::AxisMapItem& GimbalGlfwMap::yawAxis() const {
    return AxisList[2];
}
