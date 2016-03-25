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

        const GlfwMap::AxisMapItem& xAxis() const;
        const GlfwMap::AxisMapItem& yAxis() const;
        const GlfwMap::AxisMapItem& yawAxis() const;
        const GlfwMap::AxisMapItem& wristAxis() const;
        const GlfwMap::AxisMapItem& bucketAxis() const;
        const GlfwMap::ButtonMapItem& bucketOpenButton() const;
        const GlfwMap::ButtonMapItem& bucketCloseButton() const;
        const GlfwMap::ButtonMapItem& bucketFullOpenButton() const;
        const GlfwMap::ButtonMapItem& bucketFullCloseButton() const;
        const GlfwMap::ButtonMapItem& wristUpButton() const;
        const GlfwMap::ButtonMapItem& wristDownButton() const;
        const GlfwMap::ButtonMapItem& stowButton() const;

    protected:
        QString getClassName() const;
    };
}

#endif // ARMGLFWMAP_H
