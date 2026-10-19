#ifndef ENGINE_SOURCE_EVENT_EVTINPUT_H
#define ENGINE_SOURCE_EVENT_EVTINPUT_H

#include <cstdint>
#include "Engine/Source/Event/Types.h"
#include "Engine/Source/Os/Input.h"
#include "Engine/Source/Tempest/CRect.h"

class EvtContext;

namespace Input {
    extern MOUSEMODE s_mouseMode;
    extern uint32_t s_mouseHoldButton;
    extern uint32_t s_metaKeyState;
    extern uint32_t s_buttonState;
    extern CRect s_boundingRect;
}

void ConvertPosition(int32_t x, int32_t y, float* outX, float* outY);
int32_t IEvtConfirmClose();
void IEvtDispatchFocusChanged(EvtContext* context, int32_t focus);
void IEvtDispatchInput(EvtContext* context, OSINPUT id, int32_t* param, int32_t* shutdown);
void IEvtDispatchJoystickAxis(EvtContext* context, int32_t id, int32_t value, uint32_t time);
void IEvtDispatchJoystickStick(EvtContext* context, int32_t id, int32_t x, int32_t y, uint32_t time);
void IEvtDispatchKeyDown(EvtContext* context, KEY key, uint32_t repeat, uint32_t time);
void IEvtDispatchKeyUp(EvtContext* context, KEY key, uint32_t repeat, uint32_t time);
void IEvtDispatchMouseDown(EvtContext* context, MOUSEBUTTON button, int32_t x, int32_t y, uint32_t time);
void IEvtDispatchMouseEvent(EvtContext* context, MOUSEBUTTON button, int32_t x, int32_t y, uint32_t flags, uint32_t time);
void IEvtDispatchMouseEvents(EvtContext* context, int32_t x, int32_t y);
void IEvtDispatchMouseMove(EvtContext* context, int32_t x, int32_t y, uint32_t time);
void IEvtDispatchMouseMoveRelative(EvtContext* context, int32_t x, int32_t y, uint32_t time);
void IEvtDispatchMouseWheel(EvtContext* context, int32_t wheelDistance, int32_t x, int32_t y, uint32_t time);
void IEvtInputDestroy();
void IEvtInputGetMousePosition(float* x, float* y);
void IEvtInputInitialize();
int32_t IEvtInputProcess(EvtContext* context, int32_t* shutdown);
void IEvtInputSetConfirmCloseCallback(int32_t (*callback)(void*), void* param);
void IEvtInputSetMouseMode(EvtContext* context, MOUSEMODE mode, uint32_t holdButton);
void PostMouseModeChanged(EvtContext* context, MOUSEMODE mode);
void PostString(EvtContext* context, const uint16_t* string, int32_t length);

#endif
