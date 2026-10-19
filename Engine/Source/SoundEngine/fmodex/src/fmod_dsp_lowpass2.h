#ifndef _FMOD_DSP_LOWPASS2_H
#define _FMOD_DSP_LOWPASS2_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_LOWPASS2

#include "fmod.h"
#include "fmod_dsp_filter.h"
        
namespace FMOD
{   
    class DSPLowPass2 : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:
        
        float           mResonance;
        float           mResonanceUpdate;
        float           mCutoffHz;
        float           mCutoffHzUpdate;

	    float           mFilter_Y[DSP_MAXLEVELS_MAX][2];
        float           mFilter_A0, mFilter_B0, mFilter_B1;

        FMOD_RESULT     createInternal();
        FMOD_RESULT     resetInternal();
        FMOD_RESULT     readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        FMOD_RESULT     setParameterInternal(int index, float value);
        FMOD_RESULT     getParameterInternal(int index, float *value, char *valuestr);

        FMOD_RESULT     updateCoefficients(float resonance, float cutoff);

      public:

        static FMOD_DSP_DESCRIPTION_EX *getDescriptionEx();        

        static FMOD_RESULT F_CALLBACK createCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK resetCallback(FMOD_DSP_STATE *dsp);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        static FMOD_RESULT F_CALLBACK setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value);
        static FMOD_RESULT F_CALLBACK getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr);
    };
}

#endif

#endif  

