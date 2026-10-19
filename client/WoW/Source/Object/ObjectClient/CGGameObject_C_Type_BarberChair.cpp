#include "CGGameObject_C_Type_BarberChair.h"
#include "WoW/Source/Object/ObjectClient/Player_C.h"
#include "WoW/Source/Object/ObjectClient/Unit_C.h"
#include "WoW/Source/ObjectMgrClient/ObjectMgrClient.h"
#include "WoW/Source/Ui/Types.h"

// VERIFY: 7687419a manual
// OFFSET: 0x0070C7F0
CGGameObject_C_Type_BarberChair::CGGameObject_C_Type_BarberChair(CGGameObject_C* owner) : CGGameObject_C_Type_Chair(owner) {
}

// VERIFY: 4885b81c auto
// OFFSET: 0x008C8DE0
uint32_t CGGameObject_C_Type_BarberChair::GetNumSlots() const {
    return 1;
}

// VERIFY: 69c29ea6 manual
// OFFSET: 0x00711EB0
bool CGGameObject_C_Type_BarberChair::CanUseNow(GAME_ERROR_TYPE* error, float* distance, const char** errorText) const {
    if (!CGGameObject_C_Type_Chair::CanUseNow(error, distance, errorText)) {
        return false;
    }

    CGPlayer_C* player = ClntObjMgrGetActivePlayerObj();

    if (player->m_unit->m_unitFieldDisplayid == player->m_unit->m_unitFieldNativedisplayid && !player->IsShapeShifted()) {
        return true;
    }

    if (error) {
        *error = static_cast<GAME_ERROR_TYPE>(ERR_NOT_WHILE_SHAPESHIFTED);
    }

    return false;
}
