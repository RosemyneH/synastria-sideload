#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_QUESTGIVER_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_QUESTGIVER_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"

class CGGameObject_C_Type_QuestGiver : public CGGameObject_C_TypeAnimated {
    public:
    virtual int32_t CanChangeCursor() const;
    virtual void StartInteraction();
    virtual void CloseInteraction();

    CGGameObject_C_Type_QuestGiver(CGGameObject_C* owner);
};

#endif
