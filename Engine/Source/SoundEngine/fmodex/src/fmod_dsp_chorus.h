#ifndef _FMOD_DSP_CHORUS_H
#define _FMOD_DSP_CHORUS_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_CHORUS

#include "fmod.h"
#include "fmod_dsp_filter.h"
        
namespace FMOD
{   
    const int DSP_CHORUS_COSTABBITS = 13;
    const int DSP_CHORUS_COSTABSIZE = (1 << DSP_CHORUS_COSTABBITS);
    const int DSP_CHORUS_TABLERANGE = (DSP_CHORUS_COSTABSIZE * 4);
    const int DSP_CHORUS_TABLEMASK  = (DSP_CHORUS_TABLERANGE - 1);
    const float DSP_CHORUS_MAXBUFFERLENGTHMS = 200.0f;

    #define TEMPBUFFSAMPLES     256
    #define TEMPBUFFHALFSAMPLES TEMPBUFFSAMPLES / 2

    #if !defined(PLATFORM_PS3) && !defined(PLATFORM_WINDOWS_PS3MODE)
    #define DSP_CHORUS_USECOSTAB
    #endif

    class DSPChorus : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:

        FMOD_PPCALIGN16(float           mDepth);
        FMOD_PPCALIGN16(float           mDryMix);
        FMOD_PPCALIGN16(float           mWetMix1);
        FMOD_PPCALIGN16(float           mWetMix2);
        FMOD_PPCALIGN16(float           mWetMix3);
        FMOD_PPCALIGN16(float           mFeedback);
        FMOD_PPCALIGN16(float           mDelay);
        FMOD_PPCALIGN16(float           mRateHz);
        FMOD_PPCALIGN16(signed short   *mChorusBuffer);
        FMOD_PPCALIGN16(signed short   *mChorusBufferMemory);
        FMOD_PPCALIGN16(unsigned int    mChorusBufferLength);
        FMOD_PPCALIGN16(unsigned int    mChorusBufferLengthBytes);
        FMOD_PPCALIGN16(unsigned int    mChorusBufferPosition);
        FMOD_PPCALIGN16(float           mChorusPosition[3]);
        FMOD_PPCALIGN16(float           mChorusTick);
        FMOD_PPCALIGN16(float           mChorusSpeed);
        FMOD_PPCALIGN16(int             mOutputRate);

#if defined(PLATFORM_PS3) || defined(PLATFORM_WINDOWS_PS3MODE)
        FMOD_PPCALIGN16(int             mMinPos0_A);
        FMOD_PPCALIGN16(int             mMaxPos0_A);
        FMOD_PPCALIGN16(int             mMinPos0_B);
        FMOD_PPCALIGN16(int             mMaxPos0_B);
        FMOD_PPCALIGN16(int             mMinPos1_A);
        FMOD_PPCALIGN16(int             mMaxPos1_A);
        FMOD_PPCALIGN16(int             mMinPos1_B);
        FMOD_PPCALIGN16(int             mMaxPos1_B);
        FMOD_PPCALIGN16(int             mMinPos2_A);
        FMOD_PPCALIGN16(int             mMaxPos2_A);
        FMOD_PPCALIGN16(int             mMinPos2_B);
        FMOD_PPCALIGN16(int             mMaxPos2_B);
        FMOD_PPCALIGN16(signed short    mTempBuff0[4096 + 8]);
        FMOD_PPCALIGN16(signed short    mTempBuff1[4096 + 8]);
        FMOD_PPCALIGN16(signed short    mTempBuff2[4096 + 8]);
#endif

#ifdef DSP_CHORUS_USECOSTAB
        float mCosTab[DSP_CHORUS_COSTABSIZE];

        FMOD_INLINE const float    cosine(float x);
        FMOD_INLINE const float    sine(float x);
#else
        FMOD_INLINE const float    cosine(float x) { return FMOD_COS(x); }
        FMOD_INLINE const float    sine(float x)   { return FMOD_SIN(x); }
#endif

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

