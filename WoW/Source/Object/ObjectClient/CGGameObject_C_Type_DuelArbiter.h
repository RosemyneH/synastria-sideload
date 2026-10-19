#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_DUELARBITER_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_DUELARBITER_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"

class CGGameObject_C_Type_DuelArbiter : public CGGameObject_C_TypeAnimated {
    public:
    virtual bool CanHighlight() const;
    virtual bool CanUse() const;

    CGGameObject_C_Type_DuelArbiter(CGGameObject_C* owner);
};

#endif
