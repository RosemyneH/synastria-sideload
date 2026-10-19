#ifndef _FMOD_HISTORYBUFFER_POOL_H
#define _FMOD_HISTORYBUFFER_POOL_H

#include "fmod_settings.h"

#include "fmod_memory.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    const int FMOD_HISTORYBUFFERLEN = 16 * 1024;

    typedef struct BufferInfo
    {
        int    numchannels;
        float *buffermemory;
    } BufferInfo;

    class HistoryBufferPool
    {
        DECLARE_MEMORYTRACKER

      private:
        int  mBufferSize;
        int  mMaxHistoryBuffers;

        BufferInfo   *mBufferPool;
        void         *mMemoryBlock;

      public:
        HistoryBufferPool();

        FMOD_RESULT     init(int multichannelbuffercount, int channelsperbuffer);
        FMOD_RESULT     alloc(float **historybuffer, int numchannels);
        FMOD_RESULT     free (float *historybuffer);
        FMOD_RESULT     release();
    };
}

#endif

