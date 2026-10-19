#include "CGGameObject_C_Type_DuelArbiter.h"

// VERIFY: e4f6b7de auto
// OFFSET: 0x00712800
CGGameObject_C_Type_DuelArbiter::CGGameObject_C_Type_DuelArbiter(CGGameObject_C* owner) : CGGameObject_C_TypeAnimated(owner) {
}

// VERIFY: 5fa615a6 auto
// OFFSET: 0x008A1420
bool CGGameObject_C_Type_DuelArbiter::CanHighlight() const {
    return true;
}

// VERIFY: 75661770 auto
// OFFSET: 0x00427A90
bool CGGameObject_C_Type_DuelArbiter::CanUse() const {
    return false;
}
