#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_AURAGENERATOR_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_AURAGENERATOR_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"

class CGGameObject_C_Type_AuraGenerator : public CGGameObject_C_TypeAnimated {
    public:
    virtual bool CanHighlight() const;
    virtual bool CanUse() const;

    CGGameObject_C_Type_AuraGenerator(CGGameObject_C* owner);
};

#endif
