#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_FISHINGHOLE_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_FISHINGHOLE_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"

class CGGameObject_C_Type_FishingHole : public CGGameObject_C_TypeAnimated {
    public:
    virtual bool CanHighlight() const;
    virtual bool CanUse() const;

    CGGameObject_C_Type_FishingHole(CGGameObject_C* owner);
};

#endif
