#include "armglfwmap.h"

using namespace Soro;

ArmGlfwMap::ArmGlfwMap() {
    AxisList = new AxisMapItem[5] {
        //Axis maps
        AxisMapItem("Extend/Retract"),
        AxisMapItem("Raise/Lower"),
        AxisMapItem("Left/Right"),
        AxisMapItem("Articulate Wrist"),
        AxisMapItem("Open/Close Bucket"),
    };
    ButtonList = new ButtonMapItem[7] {
        //Button maps
        ButtonMapItem("Open Bucket"),
        ButtonMapItem("Close Buckeet"),
        ButtonMapItem("Fast Open Bucket"),
        ButtonMapItem("Fast Close Bucket"),
        ButtonMapItem("Wrist Up"),
        ButtonMapItem("Wrist Down"),
        ButtonMapItem("Stow Arm"),
    };
}

int ArmGlfwMap::axisCount() const {
    return 5;
}

int ArmGlfwMap::buttonCount() const {
    return 7;
}

const GlfwMap::AxisMapItem& ArmGlfwMap::XAxis() const {
    return AxisList[0];
}

const GlfwMap::AxisMapItem& ArmGlfwMap::YAxis() const {
    return AxisList[1];
}

const GlfwMap::AxisMapItem& ArmGlfwMap::YawAxis() const {
    return AxisList[2];
}

const GlfwMap::AxisMapItem& ArmGlfwMap::WristAxis() const {
    return AxisList[3];
}

const GlfwMap::AxisMapItem& ArmGlfwMap::BucketAxis() const {
    return AxisList[4];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::BucketOpenButton() const {
    return ButtonList[0];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::BucketCloseButton() const {
    return ButtonList[1];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::BucketFullOpenButton() const {
    return ButtonList[2];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::BucketFullCloseButton() const {
    return ButtonList[3];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::WristUpButton() const {
    return ButtonList[4];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::WristDownButton() const {
    return ButtonList[5];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::StowButton() const {
    return ButtonList[6];
}
