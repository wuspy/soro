#ifndef DRIVEGLFWMAP_H
#define DRIVEGLFWMAP_H

#include "glfwmap.h"

namespace Soro {

    /* Struct for a glfw gamepad mapping for arm control input
     */
    struct DriveGlfwMap: public GlfwMap {
        DriveGlfwMap();
        ~DriveGlfwMap();

        int axisCount() const;
        int buttonCount() const;

        const GlfwMap::AxisMapItem& forwardAxis() const;
        const GlfwMap::AxisMapItem& turnAxis() const;
        const GlfwMap::AxisMapItem& leftWheelsAxis() const;
        const GlfwMap::AxisMapItem& rightWheelsAxis() const;
    };
}

#endif // DRIVEGLFWMAP_H
