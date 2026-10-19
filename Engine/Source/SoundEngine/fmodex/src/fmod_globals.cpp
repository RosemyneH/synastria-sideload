#include "fmod_globals.h"

#include "fmod_memory.h"
#include "fmod_string.h"

#ifndef PLATFORM_PS2_IOP
    #include "fmod_systemi.h"
#endif

namespace FMOD
{

#if defined(FMOD_STATICFORPLUGINS) || defined(FMOD_USE_GETGLOBALS)

#ifndef PLATFORM_SOLARIS    // Solaris merges the gGlobals, get rid of this one, keeping the valid non fmod_event one
Global *gGlobal = 0;
#endif

#else

static Global  gGlobalMem;

#ifndef PLATFORM_PS3_SPU
#ifndef PLATFORM_PS2_IOP
static SystemI gSystemHeadMem;
#endif
static MemPool gSystemPoolMem;
#endif

Global *gGlobal = &gGlobalMem;

/*
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
*/
void Global::init()
{
#ifndef PLATFORM_PS3_SPU
#ifndef PLATFORM_PS2_IOP
    gSystemHead = &gSystemHeadMem;
#endif
    gSystemPool = &gSystemPoolMem;
#endif

#ifdef FMOD_DEBUG
    gDebugLevel = FMOD_DEBUG_LEVEL_LOG | FMOD_DEBUG_LEVEL_WARNING | FMOD_DEBUG_LEVEL_ERROR;
    #ifdef FMOD_DEBUG_TOFILE
    gDebugMode  = DEBUG_FILE;
    #else
    gDebugMode  = DEBUG_STDOUT;
    #endif

    #ifdef PLATFORM_PS2
    FMOD_strcpy(FMOD::gGlobal->gDebugFilename, "host0:fmod.log");
    #else
    FMOD_strcpy(FMOD::gGlobal->gDebugFilename, "fmod.log");
    #endif

#endif

#if defined(PLATFORM_MAC) || defined(PLATFORM_LINUX) || defined(PLATFORM_SOLARIS) || defined(PLATFORM_IPHONE)
    gStartTimeSeconds          = 0;
#endif

	gDSPClock.mHi              = 0;
	gDSPClock.mLo	           = 0;
    gDSPClockTimeStamp         = 0;
    gSystemInitCount           = 0;
    gFileBusy                  = 0;


}

#endif


}
