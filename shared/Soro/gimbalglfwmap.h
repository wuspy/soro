#ifndef GIMBALGLFWMAP_H
#define GIMBALGLFWMAP_H

#include "glfwmap.h"

namespace Soro {

    /* Struct for a glfw gamepad mapping for arm control input
     */
    struct GimbalGlfwMap: public GlfwMap {
        GimbalGlfwMap();
        ~GimbalGlfwMap();

        int axisCount() const;
        int buttonCount() const;

        const GlfwMap::AxisMapItem& pitchAxis() const;
        const GlfwMap::AxisMapItem& rollAxis() const;
        const GlfwMap::AxisMapItem& yawAxis() const;
    };
}

#endif // GIMBALGLFWMAP_H
