#include "driveglfwmap.h"

using namespace Soro;

DriveGlfwMap::DriveGlfwMap() {
    AxisList = new AxisMapItem[4] {
        //Axis maps
        AxisMapItem("Forward/Back"),
        AxisMapItem("Turn Left/Right"),
        AxisMapItem("Left-side Drive"),
        AxisMapItem("Right-side Drive")
    };
}

QString DriveGlfwMap::getClassName() const {
    return "DriveGlfwMap";
}

int DriveGlfwMap::axisCount() const {
    return 4;
}

int DriveGlfwMap::buttonCount() const {
    return 0;
}

const GlfwMap::AxisMapItem& DriveGlfwMap::forwardAxis() const {
    return AxisList[0];
}

const GlfwMap::AxisMapItem& DriveGlfwMap::turnAxis() const {
    return AxisList[1];
}

const GlfwMap::AxisMapItem& DriveGlfwMap::leftWheelsAxis() const {
    return AxisList[2];
}

const GlfwMap::AxisMapItem& DriveGlfwMap::rightWheelsAxis() const {
    return AxisList[3];
}
