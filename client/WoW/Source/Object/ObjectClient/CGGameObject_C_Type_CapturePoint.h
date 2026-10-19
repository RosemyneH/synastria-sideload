#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_CAPTUREPOINT_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_CAPTUREPOINT_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"

class CGGameObject_C_Type_CapturePoint : public CGGameObject_C_TypeAnimated {
    public:
    virtual bool CanHighlight() const;
    virtual bool CanUse() const;
    virtual void PostInitWithStats();
    virtual void Reenable();
    virtual void Disable();

    CGGameObject_C_Type_CapturePoint(CGGameObject_C* owner);
};

#endif
