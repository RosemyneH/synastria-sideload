#ifndef _FMOD_SAMPLE_SOFTWARE_H
#define _FMOD_SAMPLE_SOFTWARE_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_dsp_resampler.h"
#include "fmod_sound_sample.h"

namespace FMOD
{
    class SampleSoftware : public Sample
    {
        DECLARE_MEMORYTRACKER

        friend class OutputSoftware;
        friend class ChannelSoftware;
        friend class DSPWaveTable;
        friend class DSPCodec;

      private:

        void            *mBuffer;
        void            *mBufferMemory;
        char            *mLoopPointDataEnd;
        char             mLoopPointDataEndMemory[8];
        bool             mDataEndCopied;

      public:

        SampleSoftware();

        FMOD_RESULT     release(bool freethis = true);
        FMOD_RESULT     lockInternal(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        FMOD_RESULT     unlockInternal(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);
        FMOD_RESULT     setBufferData(void *data);

        FMOD_RESULT     setLoopPoints(unsigned int loopstart, unsigned int looplength);
        FMOD_RESULT     setLoopPointData();
        FMOD_RESULT     restoreLoopPointData();
        FMOD_RESULT     setMode(FMOD_MODE mode);
    };
}

#endif

#endif

