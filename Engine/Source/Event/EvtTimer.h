#ifndef ENGINE_SOURCE_EVENT_EVTTIMER_H
#define ENGINE_SOURCE_EVENT_EVTTIMER_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Event/Types.h"

class EvtContext;

int32_t IEvtTimerDispatch(EvtContext* context);
uint32_t IEvtTimerGetNextTime(EvtContext* context, uint32_t currTime);
void IEvtTimerKill(EvtContext* context, uint32_t id, EVENTHANDLERFUNC handler, const char* name);
uint32_t IEvtTimerSet(EvtContext* context, uint32_t delay, EVENTHANDLERFUNC handler, void* param, EVENTHANDLERGUIDFUNC guidHandler, uint64_t guidParam, void* guidParam2);

#endif
