#include "CMovementGlobals.h"
#include "WoW/Source/Object/Movement.h"

// VERIFY: 9f9d453f manual
// OFFSET: 0x006EC000
CMovementGlobals::~CMovementGlobals() {
    if (this->m_movementLog) {
        fclose(this->m_movementLog);
    }

    this->m_movementLog = nullptr;

    if (this->m_fallingLog) {
        fclose(this->m_fallingLog);
    }

    this->m_fallingLog = nullptr;
}
