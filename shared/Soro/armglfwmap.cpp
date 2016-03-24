#include "armglfwmap.h"

using namespace Soro;

void ArmGlfwMap::reset() {
    XAxis.GlfwIndex = MapItem::UNMAPPED;
    YAxis.GlfwIndex = MapItem::UNMAPPED;
    YawAxis.GlfwIndex = MapItem::UNMAPPED;
    WristAxis.GlfwIndex = MapItem::UNMAPPED;
    BucketAxis.GlfwIndex = MapItem::UNMAPPED;
    BucketOpenBtn.GlfwIndex = MapItem::UNMAPPED;
    BucketCloseBtn.GlfwIndex = MapItem::UNMAPPED;
    BucketFastOpenBtn.GlfwIndex = MapItem::UNMAPPED;
    BucketFastCloseBtn.GlfwIndex = MapItem::UNMAPPED;
    WristUpBtn.GlfwIndex = MapItem::UNMAPPED;
    WristDownBtn.GlfwIndex = MapItem::UNMAPPED;
    StowMacroBtn.GlfwIndex = MapItem::UNMAPPED;
    Macro1Btn.GlfwIndex = MapItem::UNMAPPED;
    Macro2Btn.GlfwIndex = MapItem::UNMAPPED;
    Macro3Btn.GlfwIndex = MapItem::UNMAPPED;
    Macro4Btn.GlfwIndex = MapItem::UNMAPPED;
    MacroRecordBtn.GlfwIndex = MapItem::UNMAPPED;
}

int ArmGlfwMap::count() const {
    return 17;
}

GlfwMap::MapItem& ArmGlfwMap::operator[](int index) {
    switch (index) {
    case 0: return XAxis;
    case 1: return YAxis;
    case 2: return YawAxis;
    case 3: return WristAxis;
    case 4: return BucketAxis;
    case 5: return BucketOpenBtn;
    case 6: return BucketCloseBtn;
    case 7: return BucketFastOpenBtn;
    case 8: return BucketFastCloseBtn;
    case 9: return WristUpBtn;
    case 10: return WristDownBtn;
    case 11: return StowMacroBtn;
    case 12: return Macro1Btn;
    case 13: return Macro2Btn;
    case 14: return Macro3Btn;
    case 15: return Macro4Btn;
    case 16: return MacroRecordBtn;
    }
}

const GlfwMap::MapItem& ArmGlfwMap::operator[](int index) const {
    switch (index) {
    case 0: return XAxis;
    case 1: return YAxis;
    case 2: return YawAxis;
    case 3: return WristAxis;
    case 4: return BucketAxis;
    case 5: return BucketOpenBtn;
    case 6: return BucketCloseBtn;
    case 7: return BucketFastOpenBtn;
    case 8: return BucketFastCloseBtn;
    case 9: return WristUpBtn;
    case 10: return WristDownBtn;
    case 11: return StowMacroBtn;
    case 12: return Macro1Btn;
    case 13: return Macro2Btn;
    case 14: return Macro3Btn;
    case 15: return Macro4Btn;
    case 16: return MacroRecordBtn;
    }
}
