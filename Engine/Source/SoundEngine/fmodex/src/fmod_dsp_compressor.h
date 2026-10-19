#ifndef _FMOD_DSP_COMPRESSOR_H
#define _FMOD_DSP_COMPRESSOR_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_COMPRESSOR

#include "fmod.h"
#include "fmod_dsp_filter.h"
        
namespace FMOD
{   
    class DSPCompressor : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:
        FMOD_RESULT     createInternal();
        FMOD_RESULT     readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        FMOD_RESULT     setParameterInternal(int index, float value);
        FMOD_RESULT     getParameterInternal(int index, float *value, char *valuestr);

        // Parameters with internal units versions
        float           mHoldConstant;
        float           mAttack_ms;
        float           mAttack_constant;
        float           mRelease_ms;
        float           mRelease_constant;
        float           mThreshold_dB;
        float           mThreshold_lin;
        float           mGainMakeup_dB;
        float           mGainMakeup_lin;

        // Internal state
        float          mGain;
        float          mMaxChannelIn[DSP_MAXLEVELS_MAX];
      public:

        static FMOD_DSP_DESCRIPTION_EX *getDescriptionEx();        

        static FMOD_RESULT F_CALLBACK createCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        static FMOD_RESULT F_CALLBACK seekCallback(FMOD_DSP_STATE *dsp, unsigned int seeklen);
        static FMOD_RESULT F_CALLBACK setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value);
        static FMOD_RESULT F_CALLBACK getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr);
    };
}

#endif

#endif  

