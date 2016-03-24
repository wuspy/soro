#ifndef GLFWMAP_H
#define GLFWMAP_H

#include "iniparser.h"

namespace Soro {

    /* Abstract struct for glfw control mapping
     *
     * Provides methods to load and save from a file
     */
    struct GlfwMap {

        enum InputType {
            AXIS, BUTTON
        };

        struct MapItem {
            static const int UNMAPPED = -1;

            int GlfwIndex = UNMAPPED;
            InputType Type;
            QString Name;

            MapItem(int index, InputType type, QString name) {
                GlfwIndex = index;
                Type = type;
                Name = name;
            }

            inline bool isMapped() const {
                return GlfwIndex != UNMAPPED;
            }
        };

        virtual void reset()=0;
        virtual int count() const=0;
        virtual MapItem& operator[](int index)=0;
        virtual const MapItem& operator[](int index) const=0;

        /* If a button is mapped to something that should be controlled by
         * an axis, this value controls the speed (-100 to 100) that will
         * be simulated
         */
        signed char AxisButtonSpeedFactor;
        QString ControllerName = QString::null;

        bool loadMapping(QFile& file);
        bool writeMapping(QFile& file);
        bool isMapped() const;
    };
}

#endif // GLFWMAP_H
