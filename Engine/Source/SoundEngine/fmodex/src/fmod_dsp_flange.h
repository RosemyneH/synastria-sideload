#ifndef _FMOD_DSP_FLANGE_H
#define _FMOD_DSP_FLANGE_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_FLANGE

#include "fmod.h"
#include "fmod_dsp_filter.h"

#define FLANGE_USEFLOAT

namespace FMOD
{   
    const int DSP_FLANGE_COSTABBITS = 13;
    const int DSP_FLANGE_COSTABSIZE = (1 << DSP_FLANGE_COSTABBITS);
    const int DSP_FLANGE_TABLERANGE = (DSP_FLANGE_COSTABSIZE * 4);
    const int DSP_FLANGE_TABLEMASK  = (DSP_FLANGE_TABLERANGE - 1);
    const float DSP_FLANGE_MAXBUFFERLENGTHMS = 40.0f;

    #if !defined(PLATFORM_PS3) && !defined(PLATFORM_WINDOWS_PS3MODE)
    #define DSP_FLANGE_USECOSTAB
    #endif

    class DSPFlange : public DSPFilter
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:

        float           mDepth;
        float           mDryMix;
        float           mWetMix;
        float           mRateHz;
    
#if defined(PLATFORM_PS3) || defined(PLATFORM_WINDOWS_PS3MODE)
        signed short    FMOD_PPCALIGN16(mFlangeBuffer[15872]); //31744 bytes
#else
        #ifdef FLANGE_USEFLOAT
        float          *mFlangeBuffer;
        #else
        short          *mFlangeBuffer;
        #endif
#endif
        unsigned int    FMOD_PPCALIGN16(mFlangeBufferLength);
        unsigned int    FMOD_PPCALIGN16(mFlangeBufferLengthBytes);
        unsigned int    FMOD_PPCALIGN16(mFlangeBufferPosition);
        float           FMOD_PPCALIGN16(mFlangePosition);
        float           FMOD_PPCALIGN16(mFlangeTick);
        float           FMOD_PPCALIGN16(mFlangeSpeed);
        int             FMOD_PPCALIGN16(mOutputRate);

#ifdef DSP_FLANGE_USECOSTAB
        float mCosTab[DSP_FLANGE_COSTABSIZE];

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

