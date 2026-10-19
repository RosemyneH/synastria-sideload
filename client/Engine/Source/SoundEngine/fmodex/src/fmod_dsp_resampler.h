#ifndef _FMOD_DSP_RESAMPLER_H
#define _FMOD_DSP_RESAMPLER_H

#include "fmod_settings.h"

#include "fmod_dsp_filter.h"

namespace FMOD
{
    const int FMOD_DSP_RESAMPLER_OVERFLOWLENGTH = 4;    /* Enough extra samples for the pcm wraparound */

    typedef enum
    {
        FMOD_RESAMPLER_END_MIXBUFFER,
        FMOD_RESAMPLER_END_RESAMPLEBUFFER,
        FMOD_RESAMPLER_END_SOUND,
        FMOD_RESAMPLER_END_SUBSOUND
    } FMOD_RESAMPLER_END;

    class DSPResampler : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      public:

        FMOD_UINT64P                mPosition;
        FMOD_SINT64P                mSpeed;
        float                       mFrequency;
        int                         mTargetFrequency;
        FMOD_UINT64P                mResamplePosition;
        void                       *mResampleBufferMemory;
        void                       *mResampleBuffer;
        int                         mResampleBufferChannels;
        unsigned int                mResampleBlockLength;
        unsigned int                mResampleBufferLength;
        unsigned int                mResampleBufferPos;
        unsigned int                mResampleFinishPos;
        unsigned int                mOverflowLength;
        unsigned int                mReadPosition;
        int                         mFill;
        unsigned int                mLength;
        unsigned int                mLoopStart;
        unsigned int                mLoopLength;
        int                         mLoopCount;
        FMOD_MODE                   mMode;

        DSPResampler();

        virtual FMOD_RESULT     release(bool freethis = true);
        FMOD_RESULT             alloc(FMOD_DSP_DESCRIPTION_EX *description);
        FMOD_RESULT             setFrequency(float frequency);
        FMOD_RESULT             getFrequency(float *frequency);
        FMOD_RESULT             setPosition(unsigned int position, bool processinputs);

        virtual FMOD_RESULT     addInput(DSPI *target);
        virtual FMOD_RESULT     execute(float *inbuffer, float **outbuffer, unsigned int *length, int inchannels, int *outchannels, FMOD_SPEAKERMODE speakermode);
    };
}

#endif  

