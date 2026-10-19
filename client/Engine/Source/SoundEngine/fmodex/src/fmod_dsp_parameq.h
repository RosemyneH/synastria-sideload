#ifndef _FMOD_DSP_PARAMEQ_H
#define _FMOD_DSP_PARAMEQ_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_PARAMEQ

#include "fmod.h"
#include "fmod_dsp_filter.h"
        
namespace FMOD
{   
    class DSPParamEq : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:
        
        float   mCenter;
        float   mBandwidth;
        float   mGain;

        float   mCenterUpdate;
        float   mBandwidthUpdate;
        float   mGainUpdate;

	    float   mFilterIn[DSP_MAXLEVELS_MAX][2];
	    float   mFilterOut[DSP_MAXLEVELS_MAX][2];
        float   mCoefficient_a0, mCoefficient_a1, mCoefficient_a2;       /* numerator coefficients */
        float   mCoefficient_b0, mCoefficient_b1, mCoefficient_b2;       /* denominator coefficients */      

        FMOD_RESULT     createInternal();
        FMOD_RESULT     resetInternal();
        FMOD_RESULT     readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        FMOD_RESULT     setParameterInternal(int index, float value);
        FMOD_RESULT     getParameterInternal(int index, float *value, char *valuestr);

        FMOD_RESULT     updateCoefficients(float center, float bandwidth, float gain);

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

