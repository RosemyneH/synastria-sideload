#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_NORMALIZE

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_normalize.h"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_systemi_spu.h"
#else
    #include "fmod_systemi.h"
#endif

#include <stdio.h>

#ifdef PLATFORM_PS3
extern unsigned int _binary_spu_fmod_dsp_normalize_pic_start[];
#endif

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dspnormalize;

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
        return DSPNormalize::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */

#ifndef PLATFORM_PS3_SPU

FMOD_DSP_PARAMETERDESC dspnormalize_param[3] =
{
    { 0.0f,  20000.0f, 5000.0f, "Fade in time",   "seconds",  "Time to ramp the silence to full in ms.  0.0 to 20000.0. Default = 5000.0." },
    { 0.0f,      1.0f,    0.1f, "Lowest volume",  "",         "Lower volume range threshold to ignore.  0.0 to 1.0.  Default = 0.1.  Raise higher to stop amplification of very quiet signals." },
    { 0.0f, 100000.0f,   20.0f, "Maximum amp",    "x",        "Maximum amplification allowed.  1.0 to 100000.0.  Default = 20.0.  1.0 = no amplifaction, higher values allow more boost." },
};

#endif //PLATFORM_PS3_SPU

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
// VERIFY: 92e0ea35 auto
// OFFSET: 0x008EF1B0
FMOD_DSP_DESCRIPTION_EX *DSPNormalize::getDescriptionEx()
{
#ifndef PLATFORM_PS3_SPU
    FMOD_memset(&dspnormalize, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dspnormalize.name, "FMOD Normalize");
    dspnormalize.version       = 0x00010100;
    dspnormalize.create        = DSPNormalize::createCallback;
    dspnormalize.release       = DSPNormalize::releaseCallback;
    dspnormalize.reset         = DSPNormalize::resetCallback;

    #ifdef PLATFORM_PS3
    dspnormalize.read          = (FMOD_DSP_READCALLBACK)_binary_spu_fmod_dsp_normalize_pic_start;    /* SPU PIC entry address */
    #else
    dspnormalize.read          = DSPNormalize::readCallback;
    #endif

    dspnormalize.numparameters = sizeof(dspnormalize_param) / sizeof(dspnormalize_param[0]);
    dspnormalize.paramdesc     = dspnormalize_param;
    dspnormalize.setparameter  = DSPNormalize::setParameterCallback;
    dspnormalize.getparameter  = DSPNormalize::getParameterCallback;

    dspnormalize.mType         = FMOD_DSP_TYPE_NORMALIZE;
    dspnormalize.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dspnormalize.mSize         = sizeof(DSPNormalize);
#else
    dspnormalize.read          = DSPNormalize::readCallback;                  /* We only care about read function on SPU */
#endif
    return &dspnormalize;
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
// VERIFY: a27fd623 auto
// OFFSET: 0x008EED10
FMOD_RESULT DSPNormalize::createInternal()
{
    FMOD_RESULT result;
    int count;

    init();
   
    mMaximum = mTargetMaximum = 1.0f;

    result = mSystem->getSoftwareFormat(&mOutputRate, 0, 0, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }
          
    for (count = 0; count < mDescription.numparameters; count++)
    {
        result = setParameter(count, mDescription.paramdesc[count].defaultval);
        if (result != FMOD_OK)
        {
            return result;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPNormalize::releaseInternal()
{
    return FMOD_OK;
}


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
FMOD_RESULT DSPNormalize::resetInternal()
{
    mMaximum = mTargetMaximum = 1.0f;

    return FMOD_OK;
}
#endif // !PLATFORM_PS3_SPU


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
// VERIFY: 617530fd manual
// OFFSET: 0x008EED80
FMOD_RESULT DSPNormalize::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{   
    unsigned int count, count2;
    float attackspeed   = mAttackSpeed;
    float threshold     = mThreshold;
    float maxamp        = mMaxAmp;

    for (count = 0; count < length; count++)
    {
        float scale;

        mMaximum -= attackspeed;
        if (mMaximum < threshold)
        {
            mMaximum = threshold;
        }

        for (count2 = 0; count2 < (unsigned int)inchannels; count2++)
        {
			if (FMOD_FABS(inbuffer[(count * inchannels) + count2]) > mMaximum)
			{
				mMaximum = FMOD_FABS(inbuffer[(count * inchannels) + count2]);
			}
        }
        
        scale = 1.0f / mMaximum;

        if (scale > maxamp)
        {
            scale = maxamp;
        }

        for (count2 = 0; count2 < (unsigned int)inchannels; count2++)
        {
			outbuffer[(count * inchannels) + count2] = inbuffer[(count * inchannels) + count2] * scale;
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
// VERIFY: 14eb8520 auto
// OFFSET: 0x008EEF90
FMOD_RESULT DSPNormalize::setParameterInternal(int index, float value)
{
    switch (index)
    {
        case FMOD_DSP_NORMALIZE_FADETIME:
        {
            mFadeTime = value;
            break;
        }
        case FMOD_DSP_NORMALIZE_THRESHHOLD:
        {
            mThreshold = value;
            break;
        }
        case FMOD_DSP_NORMALIZE_MAXAMP:
        {
            mMaxAmp = value;
            break;
        }
    }
        
    if (mFadeTime)
    {
        mAttackSpeed = 1.0f / (mFadeTime * (float)mOutputRate / 1000.0f);
    }
    else
    {
        mAttackSpeed = 1.0f;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: b26f3b82 auto
// OFFSET: 0x008EF010
FMOD_RESULT DSPNormalize::getParameterInternal(int index, float *value, char *valuestr)
{
    switch (index)
    {
        case FMOD_DSP_NORMALIZE_FADETIME:
        {
            *value = mFadeTime;
            sprintf(valuestr, "%.02f", mFadeTime);
            break;
        }
        case FMOD_DSP_NORMALIZE_THRESHHOLD:
        {
            *value = mThreshold;
            sprintf(valuestr, "%.02f", mThreshold);
            break;
        }
        case FMOD_DSP_NORMALIZE_MAXAMP:
        {
            *value = mMaxAmp;
            sprintf(valuestr, "%.02f", mMaxAmp);
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
// VERIFY: 5694072e manual
// OFFSET: 0x008EF0A0
FMOD_RESULT F_CALLBACK DSPNormalize::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPNormalize *normalize = (DSPNormalize *)dsp;

    return normalize->createInternal();
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
FMOD_RESULT F_CALLBACK DSPNormalize::releaseCallback(FMOD_DSP_STATE *dsp)
{
    DSPNormalize *normalize = (DSPNormalize *)dsp;

    return normalize->releaseInternal();
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
// VERIFY: a956b0d2 manual
// OFFSET: 0x008EF0D0
FMOD_RESULT F_CALLBACK DSPNormalize::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPNormalize *normalize = (DSPNormalize *)dsp;

    return normalize->resetInternal();
}

#endif // !PLATFORM_PS3_SPU

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
// VERIFY: a493d8ac auto
// OFFSET: 0x008EF110
FMOD_RESULT F_CALLBACK DSPNormalize::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPNormalize *normalize = (DSPNormalize *)dsp;

    return normalize->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
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
// VERIFY: 80a0d236 manual
// OFFSET: 0x008EF140
FMOD_RESULT F_CALLBACK DSPNormalize::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPNormalize *normalize = (DSPNormalize *)dsp;

    return normalize->setParameterInternal(index, value);
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
// VERIFY: 40a25992 auto
// OFFSET: 0x008EF180
FMOD_RESULT F_CALLBACK DSPNormalize::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPNormalize *normalize = (DSPNormalize *)dsp;

    return normalize->getParameterInternal(index, value, valuestr);
}



#endif // !PLATFORM_PS3_SPU

}

#endif
