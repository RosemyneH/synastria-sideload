#ifndef ENGINE_SOURCE_BASE_PROP_H
#define ENGINE_SOURCE_BASE_PROP_H

#include "Engine/Source/Base/Types.h"

HPROPCONTEXT PropCreateContext();
void PropDeleteContext(HPROPCONTEXT context);
void PropDestroy();
void* PropGet(PROPERTY id);
HPROPCONTEXT PropGetSelectedContext();
void PropInitialize();
void PropSelectContext(HPROPCONTEXT context);
void PropSet(PROPERTY id, void* value);

#endif
