#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_COMPRESSOR

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_compressor.h"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_systemi_spu.h"
#else
    #include "fmod_systemi.h"
#endif

#include <stdio.h>

#ifdef PLATFORM_PS3
extern unsigned int _binary_spu_fmod_dsp_compressor_pic_start[];
#endif

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dspcompressor;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetDSPDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_DSP_DESCRIPTION_EX * F_API FMODGetDSPDescriptionEx()
    {
        return DSPCompressor::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */

#define hold_constant (0.0025f)

#ifndef PLATFORM_PS3_SPU

FMOD_DSP_PARAMETERDESC dspcompressor_param[4] =
{
    { -60.0f, 0.0f,  0.0f, "Threshold", "dB", "Compressor threshold [-60dB, 0dB]  Default = 0dB." },
    { 10.0f, 200.0f,  50.0f, "Attack", "ms", "Compressor attack time. [10ms,200ms]  Default = 50ms." },
    { 20.0f, 1000.0f,  50.0f, "Release", "ms", "Compressor release time. [10ms,1000ms]  Default = 50ms." },
    { 0.0f, 30.0f,  0.0f, "Make up gain", "dB", "Compressor make up gain [0dB, 30dB]  Default = 0dB." }
};

#endif // PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: ac8f2ea9 auto
// OFFSET: 0x008EAB70
FMOD_DSP_DESCRIPTION_EX *DSPCompressor::getDescriptionEx()
{
#ifndef PLATFORM_PS3_SPU
    FMOD_memset(&dspcompressor, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));
    
    FMOD_strcpy(dspcompressor.name, "FMOD Compressor");
    dspcompressor.version       = 0x00010100;
    dspcompressor.create        = DSPCompressor::createCallback;

    #ifdef PLATFORM_PS3
    dspcompressor.read          = (FMOD_DSP_READCALLBACK)_binary_spu_fmod_dsp_compressor_pic_start;    /* SPU PIC entry address */
    #else
    dspcompressor.read          = DSPCompressor::readCallback;
    #endif

    dspcompressor.numparameters = sizeof(dspcompressor_param) / sizeof(dspcompressor_param[0]);
    dspcompressor.paramdesc     = dspcompressor_param;
    dspcompressor.setparameter  = DSPCompressor::setParameterCallback;
    dspcompressor.getparameter  = DSPCompressor::getParameterCallback;

    dspcompressor.mType         = FMOD_DSP_TYPE_COMPRESSOR;
    dspcompressor.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dspcompressor.mSize         = sizeof(DSPCompressor);
#else
    dspcompressor.read          = DSPCompressor::readCallback;                  /* We only care about read function on SPU */
#endif
    return &dspcompressor;
}

#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: dce2b16d auto
// OFFSET: 0x008EA4C0
FMOD_RESULT DSPCompressor::createInternal()
{
    init();

    int count;
    /*
        Setup ...
    */
    for (count = 0; count < mDescription.numparameters; count++)
    {
        setParameter(count, mDescription.paramdesc[count].defaultval);
    }

    for(count = 0; count < DSP_MAXLEVELS_MAX; count++)
    {
        mMaxChannelIn[count] = 0.0f;
    }
    mGain = 1.0f;

    return FMOD_OK;
}

#endif //!PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 1e4c4749 manual
// OFFSET: 0x008EA580
FMOD_RESULT DSPCompressor::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    unsigned int sample;
    int channel;
    float in_abs, x;
    float holdconstant     = mHoldConstant;     /* Copy members for better thread safety. */
    float threshold_lin    = mThreshold_lin;
    float attack_constant  = mAttack_constant;
    float release_constant = mRelease_constant;
    float gainmakeup_lin   = mGainMakeup_lin;

    for (sample = 0; sample < length; sample++)
    {
        unsigned int  base       = sample * inchannels;
        float        *sample_in  = &(inbuffer[base]);
        float        *sample_out = &(outbuffer[base]);
        float         in_max     = 0.0f;
        float         over;

        for (channel = 0; channel < inchannels; channel++)
        {
            float *m;

            x = sample_in[channel];
            m = &(mMaxChannelIn[channel]);

            in_abs = (x<0.0f) ? -x : x;          // ABS input

            *m -= holdconstant;                 // Leak hold

            if (in_abs > *m)                     // Hold new max
            {
                *m = in_abs;
            }

            if (*m > in_max)                     // Update total max
            {
                in_max = *m;
            }
        }

        // Note: This uses linear gain curves, not dB
        // dB is better but more costly

        // Over = linear ratio of input:threshold
        over = in_max/threshold_lin;
        
        // Apply attack or release curve to over
        // to obtain gain reduction
        if (over > 1.0f)
        {
            mGain = over + attack_constant * (mGain-over);
        }
        else
        {
            mGain = over + release_constant * (mGain-over);
        }

        // Apply gain reduction to output if reduction exists.
        if (mGain > 1.0f)
        {
            for (channel = 0; channel < inchannels; channel++)
            {
					sample_out[channel] = sample_in[channel] / mGain * gainmakeup_lin;
            }
        }
        else
        {
            for (channel = 0; channel < inchannels; channel++)
            {
					sample_out[channel] = gainmakeup_lin * sample_in[channel];
            }
        }
    }

    return FMOD_OK;
}

#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: fd98750c auto
// OFFSET: 0x008EA8D0
FMOD_RESULT DSPCompressor::setParameterInternal(int index, float value)
{
    FMOD_RESULT result;
    int         outputrate;

    result = mSystem->getSoftwareFormat(&outputrate, 0, 0, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    mHoldConstant = 10.0f / (float)outputrate;  // 10Hz

    switch (index)
    {  
 
        case FMOD_DSP_COMPRESSOR_THRESHOLD:
        {
            mThreshold_dB = value;
            mThreshold_lin = FMOD_POW(10.0f, mThreshold_dB/20.0f);
            break;
        }
        case FMOD_DSP_COMPRESSOR_ATTACK:
        {
           mAttack_ms = value;
           mAttack_constant = FMOD_EXP(-1000.0f / (mAttack_ms * outputrate) );
           break;
        }
        case FMOD_DSP_COMPRESSOR_RELEASE:
        {
           mRelease_ms = value;
           mRelease_constant = FMOD_EXP(-1000.0f / (mRelease_ms * outputrate) );
           break;
        }
        case FMOD_DSP_COMPRESSOR_GAINMAKEUP:
        {
           mGainMakeup_dB = value;
           mGainMakeup_lin = FMOD_POW(10.0f, mGainMakeup_dB/20.0f);
           break;
        }
    }
/*
    if (mResonance >= 1.0f)
    {
        unsigned int  nInd;
        float         a0, a1, a2, b0, b1, b2;
        float         fs;                  // Sampling frequency, cutoff frequency 
        float         k[2];                // overall gain factor 
        float         ktotal;
        float        *coef;

	    k[0] = 1.0f;                       // Set overall filter gain 
        k[1] = 1.0f;
        ktotal = 1.0f;
	    coef = mCoefficients + 1;       // Skip k, or gain 
	    fs = (float)outputrate;         // Sampling frequency (Hz) 



        // Update overall filter gain in coef array 
        mCoefficients[0] = ktotal;
    }
*/

    return FMOD_OK;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 35ddee41 auto
// OFFSET: 0x008EA9F0
FMOD_RESULT DSPCompressor::getParameterInternal(int index, float *value, char *valuestr)
{

    switch (index)
    {
        case FMOD_DSP_COMPRESSOR_THRESHOLD:
        {
            *value = mThreshold_dB;
            sprintf(valuestr, "%.02f", mThreshold_dB);
            break;
        }
        case FMOD_DSP_COMPRESSOR_ATTACK:
        {
            *value = mAttack_ms;
            sprintf(valuestr, "%.02f", mAttack_ms);
            break;
        }
        case FMOD_DSP_COMPRESSOR_RELEASE:
        {
            *value = mRelease_ms;
            sprintf(valuestr, "%.02f", mRelease_ms);
            break;
        }
        case FMOD_DSP_COMPRESSOR_GAINMAKEUP:
        {
            *value = mGainMakeup_dB;
            sprintf(valuestr, "%.02f", mGainMakeup_dB);
            break;
        }
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
 
	[REMARKS]

    [PLATFORMS]
 
	[SEE_ALSO]
]
*/



/*
    ==============================================================================================================

    CALLBACK INTERFACE

    ==============================================================================================================
*/


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 17066cf8 manual
// OFFSET: 0x008EAAA0
FMOD_RESULT F_CALLBACK DSPCompressor::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPCompressor *compressor = (DSPCompressor *)dsp;

    return compressor->createInternal();
}

#endif //!PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 1c349ee6 auto
// OFFSET: 0x008EAAD0
FMOD_RESULT F_CALLBACK DSPCompressor::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPCompressor *compressor = (DSPCompressor *)dsp;

    return compressor->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
}

#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: f6a16be8 manual
// OFFSET: 0x008EAB00
FMOD_RESULT F_CALLBACK DSPCompressor::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPCompressor *compressor = (DSPCompressor *)dsp;

    return compressor->setParameterInternal(index, value);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: f1c679cc auto
// OFFSET: 0x008EAB40
FMOD_RESULT F_CALLBACK DSPCompressor::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPCompressor *compressor = (DSPCompressor *)dsp;

    return compressor->getParameterInternal(index, value, valuestr);
}



#endif //!PLATFORM_PS3_SPU

}

#endif
