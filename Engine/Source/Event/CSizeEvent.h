#ifndef ENGINE_SOURCE_EVENT_CSIZEEVENT_H
#define ENGINE_SOURCE_EVENT_CSIZEEVENT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Event/CEvent.h"
#include "Engine/Source/Event/Types.h"

class CSizeEvent : public CEvent {
    public:
    /* 0010 */ EVENT_DATA_SIZE m_data;
};

#endif
