#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_DOOR_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_DOOR_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"

class CGGameObject_C_Type_Door : public CGGameObject_C_TypeAnimated {
    public:
    virtual bool CanBeSolid() const;
    virtual void UpdateAnimState(uint32_t state);
    virtual bool CanUseNow(GAME_ERROR_TYPE* error, float* distance) const;

    CGGameObject_C_Type_Door(CGGameObject_C* owner);
    bool CheckPropState() const;
};

#endif
