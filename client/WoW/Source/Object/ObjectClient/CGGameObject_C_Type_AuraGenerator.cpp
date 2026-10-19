#include "CGGameObject_C_Type_AuraGenerator.h"

// VERIFY: 187f1478 auto
// OFFSET: 0x00712800
CGGameObject_C_Type_AuraGenerator::CGGameObject_C_Type_AuraGenerator(CGGameObject_C* owner) : CGGameObject_C_TypeAnimated(owner) {
}

// VERIFY: 8ef8ad10 auto
// OFFSET: 0x008A1420
bool CGGameObject_C_Type_AuraGenerator::CanHighlight() const {
    return true;
}

// VERIFY: bccffcb0 auto
// OFFSET: 0x00427A90
bool CGGameObject_C_Type_AuraGenerator::CanUse() const {
    return false;
}
