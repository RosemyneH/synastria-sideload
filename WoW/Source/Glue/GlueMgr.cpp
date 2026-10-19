#include "GlueMgr.h"

// STATIC: 0x00B6AFF8
const char* g_glueScriptEvents[NUM_GLUE_SCRIPT_EVENTS];

// VERIFY: bc35e000 auto
// OFFSET: 0x004DBC70
void GlueScriptEventsInitialize() {
#define GLUE_SCRIPT_EVENT_NAME(name) g_glueScriptEvents[GLUE_EVENT_##name] = #name;
    GLUE_SCRIPT_EVENTS(GLUE_SCRIPT_EVENT_NAME)
#undef GLUE_SCRIPT_EVENT_NAME
}
