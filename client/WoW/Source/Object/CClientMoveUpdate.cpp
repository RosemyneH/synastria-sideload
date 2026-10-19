#include "Engine/Source/Base/CDataStore.h"
#include "WoW/Source/Object/CClientMoveUpdate.h"
#include "WoW/Source/Object/Types.h"

// VERIFY: 02297c12 auto
// OFFSET: 0x004F5050
void CClientMoveUpdate::Skip(CDataStore* msg) {
    uint32_t moveFlags = CMovementStatus::Skip(msg);

    void* data;
    msg->GetDataInSitu(data, 36);

    if (moveFlags & MOVEMENTFLAG_SPLINE_ENABLED) {
        CMoveSpline::Skip(msg);
    }
}

// VERIFY: 96a84c7b auto
// OFFSET: 0x004F5090
CDataStore& operator>>(CDataStore& msg, CClientMoveUpdate& update) {
    (msg >> update.m_status).Get(update.m_walkSpeed).Get(update.m_runSpeed).Get(update.m_runBackSpeed).Get(update.m_swimSpeed).Get(update.m_swimBackSpeed).Get(update.m_flightSpeed).Get(update.m_flightBackSpeed).Get(update.m_turnRate).Get(update.m_pitchRate);

    if (update.m_status.m_moveFlags & MOVEMENTFLAG_SPLINE_ENABLED) {
        msg >> update.m_moveSpline;
    }

    return msg;
}
