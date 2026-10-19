#ifndef _FMOD_PLUGIN_H
#define _FMOD_PLUGIN_H

#include "fmod_settings.h"

#include "fmod.h"
#include "fmod_debug.h"
#include "fmod_globals.h"
#include "fmod_linkedlist.h"
#include "fmod_memory.h"
#include "fmod_os_misc.h"
#include "fmod_string.h"

#include <string.h>

namespace FMOD
{
    class File;
    class MemPool;

    class Plugin : public LinkedListNode
    {
        friend class SystemI;

      public:

        SystemI *mSystem;

        /*
            Stuff that is passed into the driver for internal use
        */
        Global  *mGlobal;

      public:

        Plugin() 
        { 
            mGlobal = gGlobal;
        }

        virtual FMOD_RESULT release();

        FMOD_RESULT  init()
        {
            gGlobal = mGlobal;
            return FMOD_OK;
        }
    };
}

#endif

