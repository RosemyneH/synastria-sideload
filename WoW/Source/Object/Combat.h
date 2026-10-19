#ifndef WOW_SOURCE_OBJECT_COMBAT_H
#define WOW_SOURCE_OBJECT_COMBAT_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/WGUID.h"

class CClientObjCreate;

#pragma pack(push, 4)
class CCombat {
    public:
    /* 0000 */ WOWGUID m_attacking;

    void SetAttacking(WOWGUID guid);
    void SetClientInitData(const CClientObjCreate& create);
};

class CCombatClient : public CCombat {
    public:
    /* 0008 */ uint32_t m_attackSent;

    void SetAttackSent(WOWGUID guid);
};
#pragma pack(pop)

#endif
