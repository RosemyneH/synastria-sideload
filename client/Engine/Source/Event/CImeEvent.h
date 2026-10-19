#ifndef ENGINE_SOURCE_EVENT_CIMEEVENT_H
#define ENGINE_SOURCE_EVENT_CIMEEVENT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Event/CEvent.h"
#include "Engine/Source/Event/Types.h"

class CImeEvent : public CEvent {
    public:
    /* 0010 */ EVENT_DATA_IME m_data;
};

#endif
