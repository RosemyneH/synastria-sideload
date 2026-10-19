#include "fmod_settings.h"

#include "fmod_plugin.h"
#include "fmod_file.h"

namespace FMOD
{


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh

	[SEE_ALSO]
]
*/
// VERIFY: c5095ca0 auto
// OFFSET: 0x00923000
FMOD_RESULT Plugin::release()
{
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "Plugin::release", "(%p)\n", this));

    FMOD_Memory_Free(this);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "Plugin::release", "done\n"));

    return FMOD_OK;
}


}

