#ifndef ENGINE_SOURCE_EVENT_CKEYEVENT_H
#define ENGINE_SOURCE_EVENT_CKEYEVENT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Event/CEvent.h"
#include "Engine/Source/Event/Types.h"

class CKeyEvent : public CEvent {
    public:
    /* 0010 */ EVENT_DATA_KEY m_data;
};

#endif
