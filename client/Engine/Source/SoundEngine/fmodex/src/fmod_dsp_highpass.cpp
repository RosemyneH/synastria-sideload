#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_HIGHPASS

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_highpass.h"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_systemi_spu.h"
#else
    #include "fmod_systemi.h"
#endif

#include <stdio.h>

#ifdef PLATFORM_PS3
extern unsigned int _binary_spu_fmod_dsp_highpass_pic_start[];
#endif

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dsphighpass;

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
        return DSPHighPass::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */

#ifndef PLATFORM_PS3_SPU

FMOD_DSP_PARAMETERDESC dsphighpass_param[2] =
{
    { 1.0f,  22000.0f,  5000.0f, "Cutoff freq", "hz", "Highpass cutoff frequency in hz.   10.0 to 22000.0.  Default = 5000.0." },
    { 1.0f,     10.0f,     1.0f, "Resonance",   "",   "Highpass resonance Q value. 1.0 to 10.0.  Default = 1.0." }
};

#endif // PLATFORM_PS3_SPU

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
// VERIFY: 548e73dd auto
// OFFSET: 0x008F1950
FMOD_DSP_DESCRIPTION_EX *DSPHighPass::getDescriptionEx()
{
#ifndef PLATFORM_PS3_SPU
    FMOD_memset(&dsphighpass, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dsphighpass.name, "FMOD Highpass");
    dsphighpass.version       = 0x00010100;
    dsphighpass.create        = DSPHighPass::createCallback;
    dsphighpass.reset         = DSPHighPass::resetCallback;

    #ifdef PLATFORM_PS3
    dsphighpass.read          = (FMOD_DSP_READCALLBACK)_binary_spu_fmod_dsp_highpass_pic_start;    /* SPU PIC entry address */
    #else
    dsphighpass.read          = DSPHighPass::readCallback;
    #endif

    dsphighpass.numparameters = sizeof(dsphighpass_param) / sizeof(dsphighpass_param[0]);
    dsphighpass.paramdesc     = dsphighpass_param;
    dsphighpass.setparameter  = DSPHighPass::setParameterCallback;
    dsphighpass.getparameter  = DSPHighPass::getParameterCallback;

    dsphighpass.mType         = FMOD_DSP_TYPE_HIGHPASS;
    dsphighpass.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dsphighpass.mSize         = sizeof(DSPHighPass);
#else
    dsphighpass.read          = DSPHighPass::readCallback;                  /* We only care about read function on SPU */
#endif

    return &dsphighpass;
}

#ifndef PLATFORM_PS3_SPU


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
// VERIFY: 359da32d manual
// OFFSET: 0x008F1820
FMOD_RESULT DSPHighPass::createInternal()
{
    int count;

    init();
   
    for (count = 0; count < mDescription.numparameters; count++)
    {
        FMOD_RESULT result;

        result = setParameter(count, mDescription.paramdesc[count].defaultval);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    resetInternal();

    /*
        Do a forced update here to make sure everything is calculated correctly first.
    */
    mResonance = mResonanceUpdate;
    mCutoffHz  = mCutoffHzUpdate;
    updateCoefficients(mResonance, mCutoffHz);

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

	[SEE_ALSO]
]
*/
// VERIFY: 9b8b8883 manual
// OFFSET: 0x008F09C0
FMOD_RESULT DSPHighPass::resetInternal()
{
    int count;

    for (count=0; count < DSP_MAXLEVELS_MAX; count++)
    {
        mIn1[count]  = mIn2[count]  = 0;
        mOut1[count] = mOut2[count] = 0;
    }

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
    
	[SEE_ALSO]
]
*/
// VERIFY: 6f311519 auto
// OFFSET: 0x008F09F0
FMOD_RESULT DSPHighPass::updateCoefficients(float resonance, float cutoff)
{
    int   outputrate;
    float w;

#ifdef PLATFORM_PS3_SPU
    outputrate = 48000;
#else
    mSystem->getSoftwareFormat(&outputrate, 0, 0, 0, 0, 0);
#endif

    w = FMOD_PI2 * (float)cutoff / outputrate;

    mCoefficient_B0 =  (1.0f + FMOD_COS(w)) / 2.0f;
    mCoefficient_B1 = -(1.0f + FMOD_COS(w));
    mCoefficient_B2 =  (1.0f + FMOD_COS(w)) / 2.0f;
    mCoefficient_A0 =   1.0f + FMOD_SIN(w) / (2 * resonance);
    mCoefficient_A1 =  -2.0f * FMOD_COS(w);
    mCoefficient_A2 =   1.0f - FMOD_SIN(w) / (2 * resonance);

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
    
	[SEE_ALSO]
]
*/
// VERIFY: c6a99045 manual
// OFFSET: 0x008F0A60
FMOD_RESULT DSPHighPass::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    unsigned int count;
    int count2;
    static float dc = (float)1E-20;

    if (mResonance != mResonanceUpdate || mCutoffHz != mCutoffHzUpdate)
    {
        float cutoffdelta = mCutoffHz - mCutoffHzUpdate;

        mResonance = mResonanceUpdate;
        mCutoffHz  = mCutoffHzUpdate;

        updateCoefficients(mResonance, mCutoffHz);

        if (FMOD_FABS(cutoffdelta) >= 1000.0f)
        {
            resetInternal();
        }
    }

    if (mCutoffHz == 1.0f)
    {
        FMOD_memcpy(outbuffer, inbuffer, sizeof(float)*length*inchannels);
        return FMOD_OK;
    }
    if (mCutoffHz == 22000.0f)
    {
        FMOD_memset(outbuffer, 0, sizeof(float)*length*inchannels);

        for (count2 = 0; count2 < inchannels; count2++)
        {
            mIn1[count2]  = mIn2[count2]  = 0;
            mOut1[count2] = mOut2[count2] = 0;
        }
        return FMOD_OK;
    }

    if (inchannels == 1)
    {
        for (count = 0; count < length; count++)
        {
            float in0, out0;

            in0 = inbuffer[count] + dc;

            out0 = (mCoefficient_B0 * in0 + mCoefficient_B1 * mIn1[0] + mCoefficient_B2 * mIn2[0] - mCoefficient_A1 * mOut1[0] - mCoefficient_A2 * mOut2[0]) / mCoefficient_A0;

	        mIn2[0] = mIn1[0];
            mIn1[0] = in0;

            mOut2[0] = mOut1[0];
            mOut1[0] = out0;

            outbuffer[count] = out0;

            dc = -dc;
        }
    }
    else if (inchannels == 2)
    {
        for (count = 0; count < length; count++)
        {
            float in0l, out0l;
            float in0r, out0r;

            in0l = inbuffer[count*2 + 0] + dc;
            in0r = inbuffer[count*2 + 1] + dc;

            out0l = (mCoefficient_B0 * in0l + mCoefficient_B1 * mIn1[0] + mCoefficient_B2 * mIn2[0] - mCoefficient_A1 * mOut1[0] - mCoefficient_A2 * mOut2[0]) / mCoefficient_A0;
            out0r = (mCoefficient_B0 * in0r + mCoefficient_B1 * mIn1[1] + mCoefficient_B2 * mIn2[1] - mCoefficient_A1 * mOut1[1] - mCoefficient_A2 * mOut2[1]) / mCoefficient_A0;

	        mIn2[0] = mIn1[0];
            mIn1[0] = in0l;
	        mIn2[1] = mIn1[1];
            mIn1[1] = in0r;

            mOut2[0] = mOut1[0];
            mOut1[0] = out0l;
            mOut2[1] = mOut1[1];
            mOut1[1] = out0r;

            outbuffer[count*2 + 0] = out0l;
            outbuffer[count*2 + 1] = out0r;

            dc = -dc;
        }
    }
    else
    {
        for (count = 0; count < length; count++)
        {
            for (count2 = 0; count2 < inchannels; count2++)
            {
                float in0, out0;

                in0 = inbuffer[(count * inchannels) + count2] + dc;

                out0 = (mCoefficient_B0 * in0 + mCoefficient_B1 * mIn1[count2] + mCoefficient_B2 * mIn2[count2] - mCoefficient_A1 * mOut1[count2] - mCoefficient_A2 * mOut2[count2]) / mCoefficient_A0;

                mIn2[count2] = mIn1[count2];
                mIn1[count2] = in0;

                mOut2[count2] = mOut1[count2];
                mOut1[count2] = out0;

                outbuffer[(count * inchannels) + count2] = out0;
            }

            dc = -dc;
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
    
	[SEE_ALSO]
]
*/
// VERIFY: 5e9fb324 auto
// OFFSET: 0x008F1660
FMOD_RESULT DSPHighPass::setParameterInternal(int index, float value)
{
    int outputrate;

    mSystem->getSoftwareFormat(&outputrate, 0, 0, 0, 0, 0);

    switch (index)
    {
        case FMOD_DSP_HIGHPASS_CUTOFF:
        {
            mCutoffHzUpdate = value;
            if (mCutoffHzUpdate >= ((float)outputrate / 2.0f) - 10.0f)
            {
                mCutoffHzUpdate = ((float)outputrate / 2.0f) - 10.0f;
            }
            break;
        }
        case FMOD_DSP_HIGHPASS_RESONANCE:
        {
            mResonanceUpdate = value;
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
// VERIFY: 5d9b7300 manual
// OFFSET: 0x008F16D0
FMOD_RESULT DSPHighPass::getParameterInternal(int index, float *value, char *valuestr)
{
    switch (index)
    {
        case FMOD_DSP_HIGHPASS_CUTOFF:
        {
            *value = mCutoffHzUpdate;
            sprintf(valuestr, "%.02f", mCutoffHzUpdate);
            break;
        }
        case FMOD_DSP_HIGHPASS_RESONANCE:
        {
            *value = mResonanceUpdate;
            sprintf(valuestr, "%.02f", mResonanceUpdate);
            break;
        }
    }

    return FMOD_OK;
}

#endif


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

#ifndef PLATFORM_PS3_SPU

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
// VERIFY: 1fb4d930 manual
// OFFSET: 0x008F1920
FMOD_RESULT F_CALLBACK DSPHighPass::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPHighPass *highpass = (DSPHighPass *)dsp;

    return highpass->createInternal();
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
// VERIFY: dd9e7d4a manual
// OFFSET: 0x008F1740
FMOD_RESULT F_CALLBACK DSPHighPass::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPHighPass *highpass = (DSPHighPass *)dsp;

    return highpass->resetInternal();
}

#endif //!PLATFORM_PS3_SPU

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
// VERIFY: 1d248086 auto
// OFFSET: 0x008F1780
FMOD_RESULT F_CALLBACK DSPHighPass::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPHighPass *highpass = (DSPHighPass *)dsp;

    return highpass->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
}

#ifndef PLATFORM_PS3_SPU

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
// VERIFY: 59819236 manual
// OFFSET: 0x008F17B0
FMOD_RESULT F_CALLBACK DSPHighPass::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPHighPass *highpass = (DSPHighPass *)dsp;

    return highpass->setParameterInternal(index, value);
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
// VERIFY: 65ffdb2e auto
// OFFSET: 0x008F17F0
FMOD_RESULT F_CALLBACK DSPHighPass::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPHighPass *highpass = (DSPHighPass *)dsp;

    return highpass->getParameterInternal(index, value, valuestr);
}



#endif //!PLATFORM_PS3_SPU

}

#endif
