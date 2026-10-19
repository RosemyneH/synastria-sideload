#ifndef ENGINE_SOURCE_EVENT_EVTCONTEXTQUEUE_H
#define ENGINE_SOURCE_EVENT_EVTCONTEXTQUEUE_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Event/EvtContext.h"

class EvtContext;

class EvtContextQueue : public TSPriorityQueue<EvtContext> {
    public:
    EvtContextQueue();
};

// INLINED
inline EvtContextQueue::EvtContextQueue() : TSPriorityQueue<EvtContext>(offsetof(EvtContext, m_schedNextWakeTime)) {
}

#endif
