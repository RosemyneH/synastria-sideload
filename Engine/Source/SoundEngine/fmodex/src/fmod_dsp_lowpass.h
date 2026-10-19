#ifndef _FMOD_DSP_LOWPASS_H
#define _FMOD_DSP_LOWPASS_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_LOWPASS

#include "fmod.h"
#include "fmod_dsp_filter.h"

namespace FMOD
{   
    const int LOWPASS_FILTER_SECTIONS = 2;   /* 2 filter sections for 24 db/oct filter */

    typedef struct 
    {
        float a0, a1, a2;       /* numerator coefficients */
        float b0, b1, b2;       /* denominator coefficients */
    } LOWPASS_BIQUAD;

    class DSPLowPass : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:

        float           FMOD_PPCALIGN16(mResonance);
        float           FMOD_PPCALIGN16(mResonanceUpdate);
        float           FMOD_PPCALIGN16(mCutoffHz);
        float           FMOD_PPCALIGN16(mCutoffHzUpdate);
        float           FMOD_PPCALIGN16(mGainFactor[LOWPASS_FILTER_SECTIONS]);
        float           FMOD_PPCALIGN16(mHistory[DSP_MAXLEVELS_MAX][2 * LOWPASS_FILTER_SECTIONS]);   /* history in filter */
        float           FMOD_PPCALIGN16(mCoefficients[4 * LOWPASS_FILTER_SECTIONS + 1]);             /* pointer to coefficients of filter */
        LOWPASS_BIQUAD  FMOD_PPCALIGN16(mProtoCoef[LOWPASS_FILTER_SECTIONS]);                        /* Filter prototype coefficients */
        
        FMOD_INLINE float filter(float input, int channel);
        FMOD_RESULT     prewarp(float *a0, float *a1, float *a2, float fc, float fs);
        FMOD_RESULT     szxform(float *a0, float *a1, float *a2, /* numerator coefficients */
                                float *b0, float *b1, float *b2, /* denominator coefficients */
                                float fc,                                  /* Filter cutoff frequency */
                                float fs,                                  /* sampling rate */
                                float *k,                                  /* overall gain factor */
                                float *coef);                              /* pointer to 4 iir coefficients */
        FMOD_RESULT     bilinear(float a0, float a1, float a2,    /* numerator coefficients */
                                float b0, float b1, float b2,    /* denominator coefficients */
                                float *k,                                  /* overall gain factor */
                                float fs,                                  /* sampling rate */
                                float *coef);                              /* pointer to 4 iir coefficients */

        FMOD_RESULT     createInternal();
        FMOD_RESULT     readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
        FMOD_RESULT     setParameterInternal(int index, float value);
        FMOD_RESULT     getParameterInternal(int index, float *value, char *valuestr);
        FMOD_RESULT     updateState(float resonance, float cutoff);

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

