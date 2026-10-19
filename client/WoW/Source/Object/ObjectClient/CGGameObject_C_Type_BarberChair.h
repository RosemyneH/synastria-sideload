#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_BARBERCHAIR_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_CGGAMEOBJECT_C_TYPE_BARBERCHAIR_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/ObjectClient/GameObject_C.h"
#include "WoW/Source/Object/ObjectClient/CGGameObject_C_Type_Chair.h"

class CGGameObject_C_Type_BarberChair : public CGGameObject_C_Type_Chair {
    public:
    virtual bool CanUseNow(GAME_ERROR_TYPE* error, float* distance, const char** errorText) const;
    virtual uint32_t GetNumSlots() const;

    CGGameObject_C_Type_BarberChair(CGGameObject_C* owner);
};

#endif
