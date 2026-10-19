#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_TRAPDOOR_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_TRAPDOOR_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"

class CGGameObject_C_Type_TrapDoor : public CGGameObject_C_Type_TransportBase {
    public:
    virtual void UpdateState(int32_t oldState, int32_t newState);
    virtual void PostInitWithStats();
    virtual void PostReenable();
    virtual void UpdateMovement(uint32_t time, uint32_t elapsed);

    CGGameObject_C_Type_TrapDoor(CGGameObject_C* owner);
};

#endif
