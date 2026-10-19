#ifndef _FMOD_DSP_FILTER_H
#define _FMOD_DSP_FILTER_H

#include "fmod_settings.h"

#include "fmod_dspi.h"
#include "fmod_os_misc.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    class DSPFilter : public DSPI
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:

        float          *mHistoryBuffer;
        unsigned int    mHistoryPosition;
        unsigned int    mHistoryLength;
        int             mBufferChannels;

      protected:

        FMOD_RESULT     release(bool freethis = true);
        FMOD_RESULT     execute(float *inbuffer, float **outbuffer, unsigned int *length, int inchannels, int *outchannels, FMOD_SPEAKERMODE speakermode);

      public:

        FMOD_RESULT     startBuffering(unsigned int historylength);
        FMOD_RESULT     getHistoryBuffer(float **buffer, unsigned int *position, unsigned int *length);
        FMOD_RESULT     stopBuffering();
    };
}

#endif  

