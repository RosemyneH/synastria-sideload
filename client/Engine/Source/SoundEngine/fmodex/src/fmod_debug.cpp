#include "fmod_settings.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "fmod.h"
#include "fmod_debug.h"
#include "fmod_globals.h"
#include "fmod_os_misc.h"
#include "fmod_time.h"
#include "fmod_string.h"
#include "fmod_systemi.h"

extern "C"
{

/*
[API]
[
	[DESCRIPTION]
    Sets the level of debug logging to the tty / output for logging versions of FMOD Ex.

	[PARAMETERS]
    'level'          Logging level to set.

 	[RETURN_VALUE]
 
	[REMARKS]
    This only has an effect with 'logging' versions of FMOD Ex.  For example on windows it must be via fmodexL.dll, not fmodex.dll.<br>
    On Xbox it would be fmodxboxL.lib not fmodxbox.lib.<br>
    FMOD_ERR_UNSUPPORTED will be returned on non logging versions of FMOD Ex (ie full release).<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Debug_GetLevel
    FMOD_DEBUGLEVEL
]
*/
// VERIFY: 6fd409ab auto
// OFFSET: 0x008D0750
FMOD_RESULT F_API FMOD_Debug_SetLevel(FMOD_DEBUGLEVEL level)
{
    #ifdef FMOD_DEBUG
    if (!FMOD::gGlobal)
    {
        #if !defined(FMOD_STATICFORPLUGINS) && !defined(PLATFORM_PS2_IOP)
        FMOD::SystemI::getGlobals(&FMOD::gGlobal);
        #else
        return FMOD_ERR_UNINITIALIZED;
        #endif
    }
    FMOD::gGlobal->gDebugLevel = (FMOD_DEBUGLEVEL)level;
    return FMOD_OK;
    #else
    return FMOD_ERR_UNSUPPORTED;
    #endif
}

/*
[API]
[
	[DESCRIPTION]
    Retrieves the current debug logging level.

	[PARAMETERS]
    'level'          Address of a variable to receieve current debug level.

 	[RETURN_VALUE]
 
	[REMARKS]
    This only has an effect with 'logging' versions of FMOD Ex.  For example on windows it must be via fmodexL.dll, not fmodex.dll.<br>
    On Xbox it would be fmodxboxL.lib not fmodxbox.lib.<br>
    FMOD_ERR_UNSUPPORTED will be returned on non logging versions of FMOD Ex (ie full release).<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Debug_SetLevel
    FMOD_DEBUGLEVEL
]
*/
FMOD_RESULT F_API FMOD_Debug_GetLevel(FMOD_DEBUGLEVEL *level)
{
    if (!level)
    {
        return FMOD_ERR_INVALID_PARAM;
    }
    #ifdef FMOD_DEBUG
    *level = FMOD::gGlobal->gDebugLevel;
    return FMOD_OK;
    #else
    *level = 0;
    return FMOD_ERR_UNSUPPORTED;
    #endif
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
    'mode'  Bitfield containing debug settings.
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    Debug_GetMode
]
*/    
FMOD_RESULT F_API FMOD_Debug_SetMode(unsigned int mode)
{
    #ifdef FMOD_DEBUG
    if (!FMOD::gGlobal)
    {
        return FMOD_ERR_UNINITIALIZED;
    }
    FMOD::gGlobal->gDebugMode = (FMOD::FMOD_DEBUGMODE)mode;
    #endif
    return FMOD_OK;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
    'mode'  Bitfield containing debug settings.
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    Debug_SetMode
]
*/    
FMOD_RESULT F_API FMOD_Debug_GetMode(unsigned int *mode)
{
    if (!mode)
    {
        return FMOD_ERR_INVALID_PARAM;
    }
    #ifdef FMOD_DEBUG
    *mode = (unsigned int )FMOD::gGlobal->gDebugMode;
    #endif
    return FMOD_OK;
}

}


#ifdef FMOD_DEBUG

namespace FMOD
{

static int      gDebugMemory    = 0;
int            *gDebugIndent    = &gDebugMemory;


#ifndef PLATFORM_PS2_IOP

#ifdef PLATFORM_PS2_EE

#include <eekernel.h>
#include <sifdev.h>

void DebugFile(const char *s)
{								
	static int      debugFP = -666;
    static bool     debugfirsttime = true;

	if (debugfirsttime)
	{
		debugFP = sceOpen(FMOD::gGlobal->gDebugFilename, SCE_CREAT | SCE_RDWR);
        debugfirsttime = false;
	}

	if (debugFP >= 0)
	{
		sceWrite(debugFP, s, FMOD_strlen(s));
	}
    else
    {
        char s[256];

        sprintf(s, "ERROR - failed to open %s! (result = %d)\n", FMOD::gGlobal->gDebugFilename, debugFP);
        FMOD_OS_Debug_OutputStr(s);
    }
}	

#else

void DebugFile(const char *s)
{								
	FILE	           *debugFP;
	const char         *openstr = "atc";
    static bool         debugfirstfime = true;

	if (debugfirstfime)
	{
		debugfirstfime = false;
		openstr = "wt";
	}

    debugFP = fopen(FMOD::gGlobal->gDebugFilename, openstr);
	if (debugFP)
	{
		fputs(s, debugFP);
        fflush(debugFP);
		fclose(debugFP);
	}
}	

#endif
#endif

void Debug(FMOD_DEBUGLEVEL type, const char *file, const int line, const char *fnname, const char *format, ...)
{								
	va_list             arglist;
    unsigned int        now;
    static unsigned int oldtime = 0;
    const int           BUFFER_SIZE = 256;
    const int           INDENT_SIZE = 64;
    char                s[BUFFER_SIZE], tmp[BUFFER_SIZE], indent[INDENT_SIZE];

    if (!(type & FMOD::gGlobal->gDebugLevel))
    {
        return;
    }

    /*
        Filter out users if one is specified.
    */
    if (FMOD::gGlobal->gDebugLevel & FMOD_DEBUG_USER_ALL && type & FMOD_DEBUG_USER_ALL)
    {
        if (!((type & FMOD_DEBUG_USER_ALL) & (FMOD::gGlobal->gDebugLevel & FMOD_DEBUG_USER_ALL)))
        {
            return;
        }
    }

    FMOD_OS_Time_GetMs(&now);
    if (oldtime == 0)
    {
        oldtime = now;
    }

	va_start(arglist, format);

    FMOD_vsnprintf(s, BUFFER_SIZE, format, arglist);

    FMOD_snprintf(indent, INDENT_SIZE, "%*s", *gDebugIndent, "");

    if (FMOD::gGlobal->gDebugLevel & FMOD_DEBUG_DISPLAY_LINENUMBERS)
    {
        char tmp2[BUFFER_SIZE];

        FMOD_snprintf(tmp, BUFFER_SIZE, "%s(%d)", file, line);

        #ifdef PLATFORM_PS2
            #define LEADINGSPACES 40
        #else
            #define LEADINGSPACES 60
        #endif
                
        if (FMOD_strlen(tmp) < LEADINGSPACES)
        {
            FMOD_strncat(tmp, "                                                                                          ", LEADINGSPACES - FMOD_strlen(tmp));
        }

        if (FMOD::gGlobal->gDebugLevel & FMOD_DEBUG_DISPLAY_TIMESTAMPS)
        {
            if (FMOD::gGlobal->gDebugLevel & FMOD_DEBUG_DISPLAY_THREAD)
            {
                FMOD_UINT_NATIVE id;
                FMOD_OS_Thread_GetCurrentID(&id), 
                FMOD_snprintf(tmp2, BUFFER_SIZE, ": [THREADID %d] [%8d ms delta = %4d] %-30s : %s%s", id, now, now - oldtime, fnname, indent, s);
            }
            else
            {
                FMOD_snprintf(tmp2, BUFFER_SIZE, ": [%8d ms delta = %4d] %-30s : %s%s", now, now - oldtime, fnname, indent, s);
            }
        }
        else
        {
            if (FMOD::gGlobal->gDebugLevel & FMOD_DEBUG_DISPLAY_THREAD)
            {
                FMOD_UINT_NATIVE id;
                FMOD_OS_Thread_GetCurrentID(&id), 
                FMOD_snprintf(tmp2, BUFFER_SIZE, ": [THREADID %d] %-30s : %s%s", id, fnname, indent, s);
            }
            else
            {
                FMOD_snprintf(tmp2, BUFFER_SIZE, ": %-30s : %s%s", fnname, indent, s);
            }
        }

        FMOD_strncat(tmp, tmp2, BUFFER_SIZE - FMOD_strlen(tmp) - 1);
    }
    else
    {
        if (FMOD::gGlobal->gDebugLevel & FMOD_DEBUG_DISPLAY_TIMESTAMPS)
        {
            if (FMOD::gGlobal->gDebugLevel & FMOD_DEBUG_DISPLAY_THREAD)
            {
                FMOD_UINT_NATIVE id;
                FMOD_OS_Thread_GetCurrentID(&id), 
                FMOD_snprintf(tmp, BUFFER_SIZE, "FMOD: [THREADID %d] [%8d ms delta = %4d] %-30s : %s%s", id, now, now - oldtime, fnname, indent, s);
            }
            else
            {
                FMOD_snprintf(tmp, BUFFER_SIZE, "FMOD: [%8d ms delta = %4d] %-30s : %s%s", now, now - oldtime, fnname, indent, s);
            }
        }
        else
        {
            if (FMOD::gGlobal->gDebugLevel & FMOD_DEBUG_DISPLAY_THREAD)
            {
                FMOD_UINT_NATIVE id;
                FMOD_OS_Thread_GetCurrentID(&id), 
                FMOD_snprintf(tmp, BUFFER_SIZE, "FMOD: [THREADID %d] %-30s : %s%s", id, fnname, indent, s);
            }
            else
            {
                FMOD_snprintf(tmp, BUFFER_SIZE, "FMOD: %-30s : %s%s", fnname, indent, s);
            }
        }
    }

    if (FMOD::gGlobal->gDebugLevel & FMOD_DEBUG_DISPLAY_COMPRESS)
    {
        static char lastmessage[BUFFER_SIZE] = "";
        static char lasts[BUFFER_SIZE]       = "";
        static int  numrepeats       = 0;

        if (!FMOD_strcmp(lasts, s) && (numrepeats < 100))
        {
            numrepeats++;
            if (numrepeats > 5)
            {
                return;
            }
        }
        else
        {
            if (numrepeats > 5)
            {
                char p[64];
                FMOD_snprintf(p, 64, "FMOD: Last message repeated %d times\n", numrepeats);
#ifndef PLATFORM_PS2_IOP
                if (FMOD::gGlobal->gDebugMode == DEBUG_FILE)
                {
                    DebugFile(p);
                }
                else
#endif
                {    
                    FMOD_OS_Debug_OutputStr(p);
                }
            }

            FMOD_strcpy(lasts, s);
            FMOD_strcpy(lastmessage, tmp);
            numrepeats = 0;
        }
    }

#ifndef PLATFORM_PS2_IOP
    if (FMOD::gGlobal->gDebugMode == DEBUG_FILE)
    {
        DebugFile(tmp);
    }
    else
#endif
    {    
        FMOD_OS_Debug_OutputStr(tmp);
    }

	va_end( arglist );

    oldtime = now;
}	

}

#endif
