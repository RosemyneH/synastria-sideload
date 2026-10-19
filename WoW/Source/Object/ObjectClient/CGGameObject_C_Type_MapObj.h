#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_MAPOBJ_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_MAPOBJ_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"

class CGGameObject_C_Type_MapObj : public CGGameObject_C_Type_WorldObj {
    public:
    CGGameObject_C_Type_MapObj(CGGameObject_C* owner);
};

#endif
