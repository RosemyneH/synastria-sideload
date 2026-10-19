#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_DUNGEONDIFFICULTY_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_DUNGEONDIFFICULTY_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"

class CGGameObject_C_Type_DungeonDifficulty : public CGGameObject_C_TypeBase {
    public:
    /* 0010 */ SE2SoundKitObject m_sound;

    virtual ~CGGameObject_C_Type_DungeonDifficulty();
    virtual bool CanUse() const;
    virtual float GetBaseAlpha() const;
    virtual void HandleAnimEvent(uint32_t eventId, uint32_t data, const C3Vector& position, uint32_t time);
    virtual void UpdateDisplay(uint32_t time);

    CGGameObject_C_Type_DungeonDifficulty(CGGameObject_C* owner);
    int32_t GetMapID() const;
    int32_t GetDifficulty() const;
};

#endif
