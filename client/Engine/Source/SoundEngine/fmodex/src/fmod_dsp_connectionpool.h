#ifndef _FMOD_DSP_CONNECTIONPOOL_H
#define _FMOD_DSP_CONNECTIONPOOL_H

#include "fmod_settings.h"

#include "fmod_dsp_connectioni.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

#define DSP_MAX_CONNECTION_BLOCKS 32

namespace FMOD
{
    class SystemI;
    class DSPConnectionI;

    class DSPConnectionPool
    {
        DECLARE_MEMORYTRACKER

      private:

        SystemI        *mSystem;
        DSPConnectionI *mConnection[DSP_MAX_CONNECTION_BLOCKS];
        DSPConnectionI *mConnectionMemory[DSP_MAX_CONNECTION_BLOCKS];
        LinkedListNode *mNodeMemory[DSP_MAX_CONNECTION_BLOCKS];
        int             mNumInputLevels;
        int             mNumOutputLevels;
        int             mNumConnections;
        LinkedListNode  mUsedListHead;
        LinkedListNode  mFreeListHead;
        DSP_LEVEL_TYPE *mLevelData[DSP_MAX_CONNECTION_BLOCKS];
        DSP_LEVEL_TYPE *mLevelDataMemory[DSP_MAX_CONNECTION_BLOCKS];

      public:
        
        FMOD_RESULT     init(SystemI *system, int numconnections, int numoutputlevels, int numinputlevels);
        FMOD_RESULT     close();

        FMOD_RESULT     alloc(DSPConnectionI **connection);
        FMOD_RESULT     free(DSPConnectionI *connection);
    };
}

#endif

