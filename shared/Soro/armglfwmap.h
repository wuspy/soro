#ifndef ARMGLFWMAP_H
#define ARMGLFWMAP_H

#include "glfwmap.h"

namespace Soro {

    /* Struct for a glfw gamepad mapping for arm control input
     */
    struct ArmGlfwMap: public GlfwMap {
        //Axis maps
        MapItem XAxis = MapItem(MapItem::UNMAPPED, AXIS, "Extend/Retract");
        MapItem YAxis = MapItem(MapItem::UNMAPPED, AXIS, "Raise/Lower");
        MapItem YawAxis = MapItem(MapItem::UNMAPPED, AXIS, "Left/Right");
        MapItem WristAxis = MapItem(MapItem::UNMAPPED, AXIS, "Articulate Wrist");
        MapItem BucketAxis = MapItem(MapItem::UNMAPPED, AXIS, "Open/Close Bucket");
        //Button maps
        MapItem BucketOpenBtn = MapItem(MapItem::UNMAPPED, BUTTON, "Open Bucket");
        MapItem BucketCloseBtn = MapItem(MapItem::UNMAPPED, BUTTON, "Close Buckeet");
        MapItem BucketFastOpenBtn = MapItem(MapItem::UNMAPPED, BUTTON, "Fast Open Bucket");
        MapItem BucketFastCloseBtn = MapItem(MapItem::UNMAPPED, BUTTON, "Fast Close Bucket");
        MapItem WristUpBtn = MapItem(MapItem::UNMAPPED, BUTTON, "Wrist Up");
        MapItem WristDownBtn = MapItem(MapItem::UNMAPPED, BUTTON, "Wrist Down");
        MapItem StowMacroBtn = MapItem(MapItem::UNMAPPED, BUTTON, "Stow Arm");
        MapItem Macro1Btn = MapItem(MapItem::UNMAPPED, BUTTON, "Macro 1");
        MapItem Macro2Btn = MapItem(MapItem::UNMAPPED, BUTTON, "Macro 2");
        MapItem Macro3Btn = MapItem(MapItem::UNMAPPED, BUTTON, "Macro 3");
        MapItem Macro4Btn = MapItem(MapItem::UNMAPPED, BUTTON, "Macro 4");
        MapItem MacroRecordBtn = MapItem(MapItem::UNMAPPED, BUTTON, "Record Macro");

        void reset();
        int count() const;
        MapItem& operator[](int index);
        const MapItem& operator[](int index) const;
    };
}

#endif // ARMGLFWMAP_H
