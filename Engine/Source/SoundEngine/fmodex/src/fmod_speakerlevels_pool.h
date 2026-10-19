#ifndef _FMOD_SPEAKERLEVELS_POOL_H
#define _FMOD_SPEAKERLEVELS_POOL_H

#include "fmod_settings.h"

#include "fmod_memory.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    typedef struct LevelsInfo
    {
        bool   inuse;
        float *levelsmemory;

    } LevelsInfo;


    class SpeakerLevelsPool
    {
        DECLARE_MEMORYTRACKER

      friend class SystemI;

      private:
        LevelsInfo  *mLevelsPool;
        SystemI     *mSystem;
     
      public:

        SpeakerLevelsPool();

        FMOD_RESULT     alloc(float **levels);
        FMOD_RESULT     free (float *levels);
        FMOD_RESULT     release();
    };
}

#endif

