#ifndef GLFWMAP_H
#define GLFWMAP_H

#include "iniparser.h"

namespace Soro {

    /* Abstract struct for glfw control mapping
     *
     * Provides methods to load and save from a file
     */
    struct GlfwMap {

        struct MapItem {
            int GlfwIndex;
            QString DisplayName;

            MapItem() {
                GlfwIndex = -1;
                DisplayName = QString::null;
            }

            MapItem(QString name) {
                GlfwIndex = -1;
                DisplayName = name;
            }

            MapItem(int index, QString name) {
                GlfwIndex = index;
                DisplayName = name;
            }

            inline bool operator ==(const MapItem& other) const {
                return (GlfwIndex == other.GlfwIndex) & (DisplayName == other.DisplayName);
            }

            inline bool operator !=(const MapItem& other) const {
                return !((*this) == other);
            }

            inline bool isMapped() const {
                return GlfwIndex >= 0;
            }

            inline void reset() {
                GlfwIndex = -1;
            }
        };

        struct AxisMapItem: MapItem {
            AxisMapItem() : MapItem() { }
            AxisMapItem(QString name): MapItem(name) { }
            AxisMapItem(int index, QString name): MapItem(index, name) { }

            float value(const float *glfwAxes, int size) const {
                if (!isMapped() | GlfwIndex >= size) return 0;
                return glfwAxes[GlfwIndex];
            }
        };

        struct ButtonMapItem: MapItem {
            ButtonMapItem() : MapItem() { }
            ButtonMapItem(QString name): MapItem(name) { }
            ButtonMapItem(int index, QString name): MapItem(index, name) { }

            bool isPressed(const unsigned char *glfwButtons, int size) const {
                if (!isMapped() | GlfwIndex >= size) return false;
                return glfwButtons[GlfwIndex] == 1;
            }
        };

        AxisMapItem* AxisList = NULL;
        ButtonMapItem* ButtonList = NULL;
        QString ControllerName = QString::null;

        ~GlfwMap();
        void reset();
        virtual int axisCount() const=0;
        virtual int buttonCount() const=0;
        bool loadMapping(QFile& file);
        bool writeMapping(QFile& file);
        bool isMapped() const;
    };
}

#endif // GLFWMAP_H
