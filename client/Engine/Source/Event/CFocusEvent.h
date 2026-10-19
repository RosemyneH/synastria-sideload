#ifndef ENGINE_SOURCE_EVENT_CFOCUSEVENT_H
#define ENGINE_SOURCE_EVENT_CFOCUSEVENT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Event/CEvent.h"
#include "Engine/Source/Event/Types.h"

class CFocusEvent : public CEvent {
    public:
    /* 0010 */ EVENT_DATA_FOCUS m_data;
};

#endif
