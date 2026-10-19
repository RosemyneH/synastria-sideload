#include "CGGameObject_C_Type_FishingHole.h"

// VERIFY: af84b3b1 auto
// OFFSET: 0x00712800
CGGameObject_C_Type_FishingHole::CGGameObject_C_Type_FishingHole(CGGameObject_C* owner) : CGGameObject_C_TypeAnimated(owner) {
}

// VERIFY: 0d205ca6 auto
// OFFSET: 0x008A1420
bool CGGameObject_C_Type_FishingHole::CanHighlight() const {
    return true;
}

// VERIFY: c724583f auto
// OFFSET: 0x00427A90
bool CGGameObject_C_Type_FishingHole::CanUse() const {
    return false;
}
