#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_MAILBOX_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_MAILBOX_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"

class CGGameObject_C_Type_Mailbox : public CGGameObject_C_TypeAnimated {
    public:
    virtual int32_t CanChangeCursor() const;
    virtual bool Use();
    virtual void StartInteraction();

    CGGameObject_C_Type_Mailbox(CGGameObject_C* owner);
};

#endif
