#ifndef _FMOD_DSP_RESAMPLER_MULTIINPUT_H
#define _FMOD_DSP_RESAMPLER_MULTIINPUT_H

#include "fmod_settings.h"

#include "fmod_dsp_filter.h"
#include "fmod_dsp_resampler.h"

namespace FMOD
{
    class DSPResamplerMultiInput : public DSPResampler
    {            
      public:

        FMOD_RESULT     execute(float *inbuffer, float **outbuffer, unsigned int *length, int inchannels, int *outchannels, FMOD_SPEAKERMODE speakermode);
        FMOD_RESULT     addInput(DSPI *target);
    };
}

#endif  

