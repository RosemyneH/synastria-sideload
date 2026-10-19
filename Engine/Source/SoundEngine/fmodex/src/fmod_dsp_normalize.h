#ifndef _FMOD_DSP_NORMALIZE_H
#define _FMOD_DSP_NORMALIZE_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_NORMALIZE

#include "fmod.h"
#include "fmod_dsp_filter.h"
        
namespace FMOD
{   
    class DSPNormalize : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:
        
        float           FMOD_PPCALIGN16(mThreshold);
        float           FMOD_PPCALIGN16(mMaxAmp);
        float           FMOD_PPCALIGN16(mFadeTime);
        float           FMOD_PPCALIGN16(mAttackSpeed);
        float           FMOD_PPCALIGN16(mMaximum);
        float           FMOD_PPCALIGN16(mTargetMaximum);
        int             FMOD_PPCALIGN16(mOutputRate);

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

