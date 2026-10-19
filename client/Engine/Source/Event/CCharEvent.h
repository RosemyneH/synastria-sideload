#ifndef ENGINE_SOURCE_EVENT_CCHAREVENT_H
#define ENGINE_SOURCE_EVENT_CCHAREVENT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Event/CEvent.h"
#include "Engine/Source/Event/Types.h"

class CCharEvent : public CEvent {
    public:
    /* 0010 */ EVENT_DATA_CHAR m_data;
};

#endif
