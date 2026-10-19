#ifndef _FMOD_DSP_LOWPASS_SIMPLE_H
#define _FMOD_DSP_LOWPASS_SIMPLE_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_LOWPASS_SIMPLE

#include "fmod.h"
#include "fmod_dsp_filter.h"
        
namespace FMOD
{   
    class DSPLowPassSimple : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:        
        float           mCutoffHz;
        float           mCutoffHzUpdate;

	    float           mFilter_Y[DSP_MAXLEVELS_MAX][2];
        float           mFilter_tc, mFilter_oneminus_tc;

        FMOD_RESULT     createInternal();
        FMOD_RESULT     resetInternal();
        FMOD_RESULT     readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        FMOD_RESULT     setParameterInternal(int index, float value);
        FMOD_RESULT     getParameterInternal(int index, float *value, char *valuestr);

        FMOD_RESULT     updateCoefficients(float cutoffhz);

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

