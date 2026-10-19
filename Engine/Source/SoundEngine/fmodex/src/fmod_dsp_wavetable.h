#ifndef _FMOD_DSP_WAVETABLE_H
#define _FMOD_DSP_WAVETABLE_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_dspi.h"
#include "fmod_dsp_resampler.h"

namespace FMOD
{
    class SoundI;

    typedef enum
    {
        DSPWAVETABLE_SPEEDDIR_FORWARDS,
        DSPWAVETABLE_SPEEDDIR_BACKWARDS,
    } DSPWAVETABLE_SPEEDDIR;

    class DSPWaveTable : public DSPI
    {
      public:

        FMOD_UINT64P                mPosition;
        FMOD_SINT64P                mSpeed;
        DSPWAVETABLE_SPEEDDIR       mDirection;
        float                       mFrequency;
        ChannelReal				   *mChannel;
        SoundI                     *mSound;

      public:

        FMOD_RESULT     alloc(FMOD_DSP_DESCRIPTION_EX *description);
        FMOD_RESULT     addInput(DSPI *target);
        FMOD_RESULT     execute(float *inbuffer, float **outbuffer, unsigned int *length, int inchannels, int *outchannels, FMOD_SPEAKERMODE speakermode);
        FMOD_RESULT     setPositionInternal(unsigned int position);
        FMOD_RESULT     setParameterInternal(int index, float value);
        FMOD_RESULT     getParameterInternal(int index, float *value, char *valuestr);

        FMOD_RESULT     setFrequency(float frequency);
        FMOD_RESULT     getFrequency(float *frequency);

        static FMOD_RESULT F_CALLBACK setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value);
        static FMOD_RESULT F_CALLBACK getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_DSP_STATE *dsp, unsigned int position);
    };
}

#endif  

#endif
