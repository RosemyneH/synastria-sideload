#ifndef ENGINE_SOURCE_EVENT_CEVENT_H
#define ENGINE_SOURCE_EVENT_CEVENT_H

#include <cstdint>
#include "Engine/Source/Base/RefCount.h"

class CEvent : public TRefCnt {
    public:
    /* 0008 */ uint32_t m_id;
    /* 000C */ void* m_param;

    CEvent();
};

#endif
