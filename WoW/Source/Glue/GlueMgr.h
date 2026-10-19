#ifndef WOW_SOURCE_GLUE_GLUEMGR_H
#define WOW_SOURCE_GLUE_GLUEMGR_H

#include "WoW/Source/Glue/Types.h"

extern const char* g_glueScriptEvents[NUM_GLUE_SCRIPT_EVENTS];

void GlueScriptEventsInitialize();
void GlueScriptEventsRegisterFunctions();
void GlueScriptEventsUnregisterFunctions();

#endif
