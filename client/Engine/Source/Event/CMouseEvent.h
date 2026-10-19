#ifndef ENGINE_SOURCE_EVENT_CMOUSEEVENT_H
#define ENGINE_SOURCE_EVENT_CMOUSEEVENT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Event/CEvent.h"
#include "Engine/Source/Event/Types.h"

class CMouseEvent : public CEvent {
    public:
    /* 0010 */ MouseEventData m_data;

    CMouseEvent();
    CMouseEvent& operator=(const EVENT_DATA_MOUSE& data);
};

#endif
