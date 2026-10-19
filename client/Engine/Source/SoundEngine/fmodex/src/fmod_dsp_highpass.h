#ifndef _FMOD_DSP_HIGHPASS_H
#define _FMOD_DSP_HIGHPASS_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_HIGHPASS

#include "fmod.h"
#include "fmod_dsp_filter.h"
        
namespace FMOD
{   
    class DSPHighPass : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:
        
        float   FMOD_PPCALIGN16(mCutoffHz);
        float   FMOD_PPCALIGN16(mCutoffHzUpdate);
        float   FMOD_PPCALIGN16(mResonance);
        float   FMOD_PPCALIGN16(mResonanceUpdate);

        float   FMOD_PPCALIGN16(mIn1[DSP_MAXLEVELS_MAX]);
        float   FMOD_PPCALIGN16(mIn2[DSP_MAXLEVELS_MAX]);
        float   FMOD_PPCALIGN16(mOut1[DSP_MAXLEVELS_MAX]);
        float   FMOD_PPCALIGN16(mOut2[DSP_MAXLEVELS_MAX]);
        float   FMOD_PPCALIGN16(mCoefficient_A0);
        float   FMOD_PPCALIGN16(mCoefficient_A1);
        float   FMOD_PPCALIGN16(mCoefficient_A2);       /* numerator coefficients */
        float   FMOD_PPCALIGN16(mCoefficient_B0);
        float   FMOD_PPCALIGN16(mCoefficient_B1);
        float   FMOD_PPCALIGN16(mCoefficient_B2);       /* denominator coefficients */

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

