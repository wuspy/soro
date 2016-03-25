#ifndef ARMGLFWMAP_H
#define ARMGLFWMAP_H

#include "glfwmap.h"

namespace Soro {

    /* Struct for a glfw gamepad mapping for arm control input
     */
    struct ArmGlfwMap: public GlfwMap {
        ArmGlfwMap();
        int axisCount() const;
        int buttonCount() const;

        const GlfwMap::AxisMapItem& XAxis() const;
        const GlfwMap::AxisMapItem& YAxis() const;
        const GlfwMap::AxisMapItem& YawAxis() const;
        const GlfwMap::AxisMapItem& WristAxis() const;
        const GlfwMap::AxisMapItem& BucketAxis() const;
        const GlfwMap::ButtonMapItem& BucketOpenButton() const;
        const GlfwMap::ButtonMapItem& BucketCloseButton() const;
        const GlfwMap::ButtonMapItem& BucketFullOpenButton() const;
        const GlfwMap::ButtonMapItem& BucketFullCloseButton() const;
        const GlfwMap::ButtonMapItem& WristUpButton() const;
        const GlfwMap::ButtonMapItem& WristDownButton() const;
        const GlfwMap::ButtonMapItem& StowButton() const;
    };
}

#endif // ARMGLFWMAP_H
