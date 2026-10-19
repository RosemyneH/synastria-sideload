#ifndef _FMOD_DSP_ECHO_H
#define _FMOD_DSP_ECHO_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_ECHO

#include "fmod.h"
#include "fmod_dsp_filter.h"

namespace FMOD
{   
    #if defined(PLATFORM_PS3) || defined(PLATFORM_WINDOWS_PS3MODE)
    class DSPEcho : public DSPI
    #else
    class DSPEcho : public DSPFilter
    #endif
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:

        float           FMOD_PPCALIGN16(mDelay);
        float           FMOD_PPCALIGN16(mDecayRatio);
        float           FMOD_PPCALIGN16(mDryMix);
        float           FMOD_PPCALIGN16(mWetMix);
        int             FMOD_PPCALIGN16(mMaxChannels);
        float          *FMOD_PPCALIGN16(mEchoBuffer);
        float          *FMOD_PPCALIGN16(mEchoBufferMemory);
        unsigned int    FMOD_PPCALIGN16(mEchoBufferLengthBytes);
        unsigned int    FMOD_PPCALIGN16(mEchoPosition);
        unsigned int    FMOD_PPCALIGN16(mEchoLength);
        unsigned int    FMOD_PPCALIGN16(mMaxLength);
        int             FMOD_PPCALIGN16(mOutputRate);
        int             FMOD_PPCALIGN16(mChannels);

        FMOD_RESULT     createInternal();
        FMOD_RESULT     releaseInternal();
        FMOD_RESULT     resetInternal();
        FMOD_RESULT     readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        FMOD_RESULT     setParameterInternal(int index, float value);
        FMOD_RESULT     getParameterInternal(int index, float *value, char *valuestr);

      public:

        static FMOD_DSP_DESCRIPTION_EX *getDescriptionEx();        

        static FMOD_RESULT F_CALLBACK createCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK releaseCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK resetCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        static FMOD_RESULT F_CALLBACK setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value);
        static FMOD_RESULT F_CALLBACK getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr);
    };
}

#endif

#endif  

