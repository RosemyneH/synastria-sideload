#ifndef _FMOD_DSP_CODECPOOL_H
#define _FMOD_DSP_CODECPOOL_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_DSPCODEC

#include "fmod_dspi.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    class DSPI;
    class SystemI;
    class DSPCodec;

    #define FMOD_DSP_CODECPOOL_MAXCODECS 256

    class DSPCodecPool
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

        friend class DSPCodec;
        
      public:
        
        SystemI         *mSystem;
        int              mNumDSPCodecs;
        DSPCodec       **mPool;
        bool             mAllocated[FMOD_DSP_CODECPOOL_MAXCODECS];
        unsigned char   *mReadBuffer;
#if defined(FMOD_SUPPORT_XMA) && defined(FMOD_SUPPORT_XMA_NEWHAL)
        unsigned char   *mFileBufferPool;
#endif        

        FMOD_RESULT init(FMOD_DSP_CATEGORY category, int resamplerpcmblocksize, int numdspcodecs);
        FMOD_RESULT close();

        FMOD_RESULT alloc(DSPCodec **dspcodec);
        FMOD_RESULT areAnyFree();
    };
}

#endif

#endif

