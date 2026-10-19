#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_FLANGE

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_flange.h"

#ifdef PLATFORM_PS3_SPU
#include "fmod_systemi_spu.h"
#else
#include "fmod_systemi.h"
#endif

#include <stdio.h>

#ifdef PLATFORM_PS3
extern unsigned int _binary_spu_fmod_dsp_flange_pic_start[];
#endif

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dspflange;

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
        return DSPFlange::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */

#ifndef PLATFORM_PS3_SPU

FMOD_DSP_PARAMETERDESC dspflange_param[4] =
{
    { 0.0f,     1.0f, 0.45f, "Drymix",   "%",     "Volume of original signal to pass to output.  0.0 to 1.0. Default = 0.45." },
    { 0.0f,     1.0f, 0.55f, "Wetmix",   "%",     "Volume of flange signal to pass to output.  0.0 to 1.0. Default = 0.55." },
    { 0.01f,    1.0f, 1.00f, "Depth",    "",      "Flange depth.  0.01 to 1.0.  Default = 1.0." },
    { 0.0f,    20.0f, 0.10f, "Rate",     "hz",    "Flange speed in hz.  0.0 to 20.0.  Default = 0.1." }
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
// VERIFY: e7e4d1eb auto
// OFFSET: 0x008EFE00
FMOD_DSP_DESCRIPTION_EX *DSPFlange::getDescriptionEx()
{
#ifndef PLATFORM_PS3_SPU
    FMOD_memset(&dspflange, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dspflange.name, "FMOD Flange");
    dspflange.version       = 0x00010100;
    dspflange.create        = DSPFlange::createCallback;
    dspflange.release       = DSPFlange::releaseCallback;
    dspflange.reset         = DSPFlange::resetCallback;

    #ifdef PLATFORM_PS3
    dspflange.read          = (FMOD_DSP_READCALLBACK)_binary_spu_fmod_dsp_flange_pic_start;    /* SPU PIC entry address */
    #else
    dspflange.read          = DSPFlange::readCallback;
    #endif

    dspflange.numparameters = sizeof(dspflange_param) / sizeof(dspflange_param[0]);
    dspflange.paramdesc     = dspflange_param;
    dspflange.setparameter  = DSPFlange::setParameterCallback;
    dspflange.getparameter  = DSPFlange::getParameterCallback;

    dspflange.mType         = FMOD_DSP_TYPE_FLANGE;
    dspflange.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dspflange.mSize         = sizeof(DSPFlange);
#else
    dspflange.read          = DSPFlange::readCallback;                  /* We only care about read function on SPU */
#endif
    return &dspflange;
}

#ifdef DSP_FLANGE_USECOSTAB

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
FMOD_INLINE const float DSPFlange::cosine(float x)
{
    int y;

    x *= DSP_FLANGE_TABLERANGE;
    y = (int)x;
    if (y < 0)
    {
        y = -y; 
    }

    y &= DSP_FLANGE_TABLEMASK;
    switch (y >> DSP_FLANGE_COSTABBITS)
    {
        case 0 : return  mCosTab[y]; 
        case 1 : return -mCosTab[(DSP_FLANGE_COSTABSIZE - 1) - (y - (DSP_FLANGE_COSTABSIZE * 1))]; 
        case 2 : return -mCosTab[                              (y - (DSP_FLANGE_COSTABSIZE * 2))]; 
        case 3 : return  mCosTab[(DSP_FLANGE_COSTABSIZE - 1) - (y - (DSP_FLANGE_COSTABSIZE * 3))]; 
    }

    return 0.0f;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
FMOD_INLINE const float DSPFlange::sine(float x)
{
    return cosine(x - 0.25f);
}

#endif


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
// VERIFY: 147e38a2 manual
// OFFSET: 0x008EF590
FMOD_RESULT DSPFlange::createInternal()
{
    FMOD_RESULT  result;
    int          channels, count;

    init();
    
#ifdef DSP_FLANGE_USECOSTAB
    {
        int count;

        for (count = 0; count < DSP_FLANGE_COSTABSIZE; count++)
        {
            mCosTab[count] = (float)FMOD_COS(FMOD_PI_2 * (float)count / (float)DSP_FLANGE_COSTABSIZE);
        }
    }
#endif

    result = mSystem->getSoftwareFormat(&mOutputRate, 0, &channels, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    mFlangeBufferLengthBytes = (int)((float)mOutputRate * DSP_FLANGE_MAXBUFFERLENGTHMS) / 1000;
    mFlangeBufferLengthBytes *= channels;
   
    #ifdef FLANGE_USEFLOAT

    mFlangeBufferLengthBytes *= sizeof(float);
    mFlangeBufferLengthBytes += 1024;
    mFlangeBuffer = (float *)FMOD_Memory_Calloc(mFlangeBufferLengthBytes);

    #else

    mFlangeBufferLengthBytes *= sizeof(short);
    mFlangeBufferLengthBytes += 1024;

    #if !defined(PLATFORM_PS3) && !defined(PLATFORM_WINDOWS_PS3MODE)
    mFlangeBuffer = (short *)FMOD_Memory_Calloc(mFlangeBufferLengthBytes);
    #endif

    #endif

    if (!mFlangeBuffer)
    {
        return FMOD_ERR_MEMORY;
    }

    mFlangeTick = 0;
   
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
FMOD_RESULT DSPFlange::releaseInternal()
{
    #if !defined(PLATFORM_PS3) && !defined(PLATFORM_WINDOWS_PS3MODE)
    if (mFlangeBuffer)
    {
        FMOD_Memory_Free(mFlangeBuffer);
        mFlangeBuffer = 0;
    }
    #endif

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
FMOD_RESULT DSPFlange::resetInternal()
{
    mFlangeBufferPosition = 0;

    if (mFlangeBuffer)
    {
        FMOD_memset(mFlangeBuffer, 0, mFlangeBufferLengthBytes);
    }

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
// VERIFY: e96589b6 manual
// OFFSET: 0x008EF6A0
FMOD_RESULT DSPFlange::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{   
    unsigned int count;
    float        halfdepth = mDepth * 0.5f;

	for (count = 0; count < length; count++) 
    {
		unsigned int p1, p2;
		float frac;
        int count2;

		p1 = (mFlangeBufferPosition + (unsigned int)mFlangePosition) % mFlangeBufferLength;
		p2 = p1 + 1;  /* the first sample of the buffer has been  duplicated to the end so it wont click. */

		frac = mFlangePosition - (int)mFlangePosition;

        for (count2 = 0; count2 < inchannels; count2++)
        {
			int offset = (count * inchannels) + count2;
			float val;

			val = inbuffer[offset] * mDryMix;

			#ifdef FLANGE_USEFLOAT
            
			val += ((mFlangeBuffer[(p1 * inchannels) + count2] * (1.0f - frac)) + (mFlangeBuffer[(p2 * inchannels) + count2] * frac)) * mWetMix;

			mFlangeBuffer[(mFlangeBufferPosition * inchannels) + count2] = inbuffer[offset];
            
            #else

			val += (((mFlangeBuffer[(p1 * inchannels) + count2] / 32768.0f) * (1.0f - frac)) + ((mFlangeBuffer[(p2 * inchannels) + count2] / 32768.0f) * frac)) * mWetMix;

			mFlangeBuffer[(mFlangeBufferPosition * inchannels) + count2] = (signed short)(inbuffer[offset] * 32768.0f);

			#endif
		    
			outbuffer[offset] = val;
        }

        if (!mFlangeBufferPosition)
        {
            for (count2 = 0; count2 < inchannels; count2++)
            {
                mFlangeBuffer[(mFlangeBufferLength * inchannels) + count2] = mFlangeBuffer[count2];
            }
        }

		mFlangeBufferPosition++;
		if (mFlangeBufferPosition >= mFlangeBufferLength) 
        {
            mFlangeBufferPosition = 0;
        }

        #ifdef DSP_FLANGE_USECOSTAB
		mFlangePosition = (1.0f + sine(mFlangeTick + 0.00f)) * halfdepth;
        #else
		mFlangePosition = (1.0f + sine((mFlangeTick + 0.00f) * FMOD_PI2)) * halfdepth;
        #endif

        mFlangePosition = mFlangePosition * (mFlangeBufferLength - 1);       
        mFlangeTick += mFlangeSpeed;
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
// VERIFY: e5e214a9 manual
// OFFSET: 0x008EFAC0
FMOD_RESULT DSPFlange::setParameterInternal(int index, float value)
{
    float olddepth;

    olddepth = mDepth;

    mSystem->lockDSP();

    switch (index)
    {
        case FMOD_DSP_FLANGE_DRYMIX:
        {
            mDryMix = value;
            break;
        }
        case FMOD_DSP_FLANGE_WETMIX:
        {
            mWetMix = value;
            break;
        }
        case FMOD_DSP_FLANGE_DEPTH:
        {
            mDepth = value;
            break;
        }
        case FMOD_DSP_FLANGE_RATE:
        {
            mRateHz = value;
            break;
        }
    }
   
    if (mDepth != olddepth)
    {
        float delay = mDepth * 10.0f;

        mFlangeBufferLength = (unsigned int)((float)mOutputRate * delay / 1000.0f);
        if (mFlangeBufferLength < 4)
        {
            mFlangeBufferLength = 4;
        }

        resetInternal();
    }

    mFlangeSpeed = mRateHz / (float)mOutputRate;

    mSystem->unlockDSP();

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
// VERIFY: f91ee0ed auto
// OFFSET: 0x008EFBD0
FMOD_RESULT DSPFlange::getParameterInternal(int index, float *value, char *valuestr)
{
    switch (index)
    {
        case FMOD_DSP_FLANGE_DRYMIX:
        {
            *value = mDryMix;
            sprintf(valuestr, "%.1f", mDryMix * 100.0f);
            break;
        }
        case FMOD_DSP_FLANGE_WETMIX:
        {
            *value = mWetMix;
            sprintf(valuestr, "%.1f", mWetMix * 100.0f);
            break;
        }
        case FMOD_DSP_FLANGE_DEPTH:
        {
            *value = mDepth;
            sprintf(valuestr, "%.02f", mDepth);
            break;
        }
        case FMOD_DSP_FLANGE_RATE:
        {
            *value = mRateHz;
            sprintf(valuestr, "%.02f", mRateHz);
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
// VERIFY: cd07b7d3 manual
// OFFSET: 0x008EFCA0
FMOD_RESULT F_CALLBACK DSPFlange::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPFlange *flange = (DSPFlange *)dsp;

    return flange->createInternal();
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
// VERIFY: 84d9e874 manual
// OFFSET: 0x008EFCD0
FMOD_RESULT F_CALLBACK DSPFlange::releaseCallback(FMOD_DSP_STATE *dsp)
{
    DSPFlange *flange = (DSPFlange *)dsp;

    return flange->releaseInternal();
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
// VERIFY: b3fa9b5d manual
// OFFSET: 0x008EFD20
FMOD_RESULT F_CALLBACK DSPFlange::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPFlange *flange = (DSPFlange *)dsp;

    return flange->resetInternal();
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
// VERIFY: 7ddfa3ab auto
// OFFSET: 0x008EFD60
FMOD_RESULT F_CALLBACK DSPFlange::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPFlange *flange = (DSPFlange *)dsp;

    return flange->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
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
// VERIFY: 782cb4e9 manual
// OFFSET: 0x008EFD90
FMOD_RESULT F_CALLBACK DSPFlange::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPFlange *flange = (DSPFlange *)dsp;

    return flange->setParameterInternal(index, value);
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
// VERIFY: a407c4b1 auto
// OFFSET: 0x008EFDD0
FMOD_RESULT F_CALLBACK DSPFlange::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPFlange *flange = (DSPFlange *)dsp;

    return flange->getParameterInternal(index, value, valuestr);
}



#endif //!PLATFORM_PS3_SPU

}

#endif
