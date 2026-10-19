#ifndef ENGINE_SOURCE_EVENT_EVTQUEUE_H
#define ENGINE_SOURCE_EVENT_EVTQUEUE_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Event/Types.h"

class EvtContext;

int32_t IEvtQueueCheckSyncKeyState(EvtContext* context, KEY key);
int32_t IEvtQueueCheckSyncMouseState(EvtContext* context, MOUSEBUTTON button);
void IEvtQueueDestroy();
void IEvtQueueInitialize();
void IEvtQueueDispatch(EvtContext* context, EVENTID id, const void* data);
void IEvtQueueDispatchAll(EvtContext* context);
void IEvtQueuePost(EvtContext* context, EVENTID id, const void* data, uint32_t bytes);
void IEvtQueueRegister(EvtContext* context, EVENTID id, EVENTHANDLERFUNC handler, void* param, float priority);
void IEvtQueueUnregister(EvtContext* context, EVENTID id, EVENTHANDLERFUNC handler, void* param, uint32_t flags);

#endif
