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

QString ArmGlfwMap::getClassName() const {
    return "ArmGlfwMap";
}

int ArmGlfwMap::axisCount() const {
    return 5;
}

int ArmGlfwMap::buttonCount() const {
    return 7;
}

const GlfwMap::AxisMapItem& ArmGlfwMap::xAxis() const {
    return AxisList[0];
}

const GlfwMap::AxisMapItem& ArmGlfwMap::yAxis() const {
    return AxisList[1];
}

const GlfwMap::AxisMapItem& ArmGlfwMap::yawAxis() const {
    return AxisList[2];
}

const GlfwMap::AxisMapItem& ArmGlfwMap::wristAxis() const {
    return AxisList[3];
}

const GlfwMap::AxisMapItem& ArmGlfwMap::bucketAxis() const {
    return AxisList[4];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::bucketOpenButton() const {
    return ButtonList[0];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::bucketCloseButton() const {
    return ButtonList[1];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::bucketFullOpenButton() const {
    return ButtonList[2];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::bucketFullCloseButton() const {
    return ButtonList[3];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::wristUpButton() const {
    return ButtonList[4];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::wristDownButton() const {
    return ButtonList[5];
}

const GlfwMap::ButtonMapItem& ArmGlfwMap::stowButton() const {
    return ButtonList[6];
}
