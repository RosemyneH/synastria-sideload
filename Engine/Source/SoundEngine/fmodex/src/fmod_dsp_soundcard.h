#ifndef _FMOD_DSP_SOUNDCARD_H
#define _FMOD_DSP_SOUNDCARD_H

#include "fmod_settings.h"

#include "fmod_dsp_filter.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    class DSPSoundCard : public DSPFilter
    {            
      public:

        float          *mConvertBuffer;
        void           *mConvertBufferMemory;

        FMOD_RESULT     alloc(FMOD_DSP_DESCRIPTION_EX *description);
        FMOD_RESULT     release(bool freethis = true);
        FMOD_RESULT     execute(void *inbuffer, void **outbuffer, unsigned int *length, int inchannels, int *outchannels, FMOD_SPEAKERMODE speakermode);
    };
}

#endif  
