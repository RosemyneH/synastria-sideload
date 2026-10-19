#include "CGGameObject_C_Type_SpellFocus.h"

// VERIFY: a7ebf79f auto
// OFFSET: 0x00712800
CGGameObject_C_Type_SpellFocus::CGGameObject_C_Type_SpellFocus(CGGameObject_C* owner) : CGGameObject_C_TypeAnimated(owner) {
}

// VERIFY: 45c10771 auto
// OFFSET: 0x008A1420
bool CGGameObject_C_Type_SpellFocus::CanHighlight() const {
    return true;
}

// VERIFY: 1ca88b2c auto
// OFFSET: 0x00427A90
bool CGGameObject_C_Type_SpellFocus::CanUse() const {
    return false;
}
