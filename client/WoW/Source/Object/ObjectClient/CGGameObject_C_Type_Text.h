#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_TEXT_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_TEXT_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"

class CGGameObject_C_Type_Text : public CGGameObject_C_TypeAnimated {
    public:
    virtual int32_t CanChangeCursor() const;
    virtual bool Use();
    virtual void PostInitWithStats();
    virtual void StartInteraction();
    virtual void CloseInteraction();

    CGGameObject_C_Type_Text(CGGameObject_C* owner);
};

#endif
