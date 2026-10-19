#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_CHORUS

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_chorus.h"
#include "fmod_systemi.h"

#include <stdio.h>

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dspchorus;

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
        return DSPChorus::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


FMOD_DSP_PARAMETERDESC dspchorus_param[8] =
{
    {  0.0f,   1.0f,  0.50f, "Dry mix",         "",   "Volume of original signal to pass to output.  0.0 to 1.0. Default = 0.5." },
    {  0.0f,   1.0f,  0.50f, "Wet mix tap 1",   "",   "Volume of 1st chorus tap.  0.0 to 1.0.  Default = 0.5." },
    {  0.0f,   1.0f,  0.50f, "Wet mix tap 2",   "",   "Volume of 2nd chorus tap. This tap is 90 degrees out of phase of the first tap.  0.0 to 1.0.  Default = 0.5." },
    {  0.0f,   1.0f,  0.50f, "Wet mix tap 3",   "",   "Volume of 3rd chorus tap. This tap is 90 degrees out of phase of the second tap.  0.0 to 1.0.  Default = 0.5." },
    {  0.0f, 100.0f, 40.00f, "Delay",           "ms", "Chorus delay in ms.  0.1 to 100.0.  Default = 40.0 ms." },
    {  0.0f,  20.0f,  0.80f, "Rate",            "hz", "Chorus modulation rate in hz.  0.0 to 20.0.  Default = 0.8 hz." },
    {  0.0f,   1.0f,  0.03f, "Depth",           "",   "Chorus modulation depth.  0.0 to 1.0.  Default = 0.03." },
    {  0.0f,   1.0f,  0.00f, "Feedback",        "",   "Chorus feedback.  Controls how much of the wet signal gets fed back into the chorus buffer.  0.0 to 1.0.  Default = 0.0." }
};


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
// VERIFY: 1c6fd531 auto
// OFFSET: 0x008EC3A0
FMOD_DSP_DESCRIPTION_EX *DSPChorus::getDescriptionEx()
{
    FMOD_memset(&dspchorus, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dspchorus.name, "FMOD Chorus");
    dspchorus.version       = 0x00010100;
    dspchorus.create        = DSPChorus::createCallback;
    dspchorus.release       = DSPChorus::releaseCallback;
    dspchorus.reset         = DSPChorus::resetCallback;
    dspchorus.read          = DSPChorus::readCallback;

    dspchorus.numparameters = sizeof(dspchorus_param) / sizeof(dspchorus_param[0]);
    dspchorus.paramdesc     = dspchorus_param;
    dspchorus.setparameter  = DSPChorus::setParameterCallback;
    dspchorus.getparameter  = DSPChorus::getParameterCallback;

    dspchorus.mType         = FMOD_DSP_TYPE_CHORUS;
    dspchorus.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dspchorus.mSize         = sizeof(DSPChorus);

    return &dspchorus;
}

#ifdef DSP_CHORUS_USECOSTAB

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
FMOD_INLINE const float DSPChorus::cosine(float x)
{
    int y;

    x *= DSP_CHORUS_TABLERANGE;
    y = (int)x;
    if (y < 0)
    {
        y = -y; 
    }

    y &= DSP_CHORUS_TABLEMASK;
    switch (y >> DSP_CHORUS_COSTABBITS)
    {
        case 0 : return  mCosTab[y]; 
        case 1 : return -mCosTab[(DSP_CHORUS_COSTABSIZE - 1) - (y - (DSP_CHORUS_COSTABSIZE * 1))]; 
        case 2 : return -mCosTab[                              (y - (DSP_CHORUS_COSTABSIZE * 2))]; 
        case 3 : return  mCosTab[(DSP_CHORUS_COSTABSIZE - 1) - (y - (DSP_CHORUS_COSTABSIZE * 3))]; 
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
FMOD_INLINE const float DSPChorus::sine(float x)
{
    return cosine(x - 0.25f);
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
// VERIFY: 6f28e6b4 manual
// OFFSET: 0x008EB9B0
FMOD_RESULT DSPChorus::createInternal()
{
    FMOD_RESULT  result;
    int          channels, count;

    init();

#ifdef DSP_CHORUS_USECOSTAB
    {
        int count;

        for (count = 0; count < DSP_CHORUS_COSTABSIZE; count++)
        {
            mCosTab[count] = (float)FMOD_COS(FMOD_PI_2 * (float)count / (float)DSP_CHORUS_COSTABSIZE);
        }
    }
#endif
    
    result = mSystem->getSoftwareFormat(&mOutputRate, 0, &channels, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    mChorusBufferLengthBytes = (int)((float)mOutputRate * DSP_CHORUS_MAXBUFFERLENGTHMS) / 1000;
    mChorusBufferLengthBytes *= channels;
    mChorusBufferLengthBytes *= sizeof(signed short);
    mChorusBufferLengthBytes += 1024;

    mChorusBuffer = (signed short *)FMOD_Memory_Calloc(mChorusBufferLengthBytes);
    if (!mChorusBuffer)
    {
        return FMOD_ERR_MEMORY;
    }

    mChorusTick = 0;
    
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
    
	[SEE_ALSO]
]
*/
FMOD_RESULT DSPChorus::releaseInternal()
{
    if (mChorusBuffer)
    {
        FMOD_Memory_Free(mChorusBuffer);
        mChorusBuffer = 0;
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
FMOD_RESULT DSPChorus::resetInternal()
{
    mChorusBufferPosition = 0;

    if (mChorusBuffer)
    {
        FMOD_memset(mChorusBuffer, 0, mChorusBufferLengthBytes);
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
// VERIFY: cc338952 manual
// OFFSET: 0x008EBAC0
FMOD_RESULT DSPChorus::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{   
    unsigned int  count;
    int      halfchorusbufferlength = mChorusBufferLength / 2;
    float    halfdepth = mDepth * 0.5f;
    
	for (count=0; count < length; count++) 
    {
		unsigned int p1[3], p2[3];
		float frac[3];
        int count2;

		p1[0] = (mChorusBufferPosition + (unsigned int)mChorusPosition[0]) % mChorusBufferLength;
		p1[1] = (mChorusBufferPosition + (unsigned int)mChorusPosition[1]) % mChorusBufferLength;
		p1[2] = (mChorusBufferPosition + (unsigned int)mChorusPosition[2]) % mChorusBufferLength;
		p2[0] = p1[0] + 1;  /* the first sample of the buffer has been  duplicated to the end so it wont click. */
		p2[1] = p1[1] + 1;  /* the first sample of the buffer has been  duplicated to the end so it wont click. */
		p2[2] = p1[2] + 1;  /* the first sample of the buffer has been  duplicated to the end so it wont click. */

		frac[0] = mChorusPosition[0] - (int)mChorusPosition[0];
		frac[1] = mChorusPosition[1] - (int)mChorusPosition[1];
		frac[2] = mChorusPosition[2] - (int)mChorusPosition[2];

        for (count2 = 0; count2 < inchannels; count2++)
        {
			int offset = (count * inchannels) + count2;
			float val;
			float chorusval;

			val = inbuffer[offset] * mDryMix * 32768.0f;

			val += ((mChorusBuffer[(p1[0] * inchannels) + count2] * (1.0f - frac[0])) + (mChorusBuffer[(p2[0] * inchannels) + count2] * frac[0])) * mWetMix1;
			val += ((mChorusBuffer[(p1[1] * inchannels) + count2] * (1.0f - frac[1])) + (mChorusBuffer[(p2[1] * inchannels) + count2] * frac[1])) * mWetMix2;
			val += ((mChorusBuffer[(p1[2] * inchannels) + count2] * (1.0f - frac[2])) + (mChorusBuffer[(p2[2] * inchannels) + count2] * frac[2])) * mWetMix3;

			chorusval = (inbuffer[offset] * 32767.0f) + val * mFeedback;

			mChorusBuffer[(mChorusBufferPosition * inchannels) + count2] = chorusval < -32768.0f ? -32768 : chorusval > 32767.0f ? 32767 : (signed short)chorusval;

			outbuffer[offset] = val / 32767.0f;
		}

        if (!mChorusBufferPosition)
        {
            for (count2 = 0; count2 < inchannels; count2++)
            {
				mChorusBuffer[(mChorusBufferLength * inchannels) + count2] = mChorusBuffer[count2];
            }
        }

		mChorusBufferPosition++;
		if (mChorusBufferPosition >= mChorusBufferLength) 
        {
            mChorusBufferPosition = 0;
        }

        #ifdef DSP_CHORUS_USECOSTAB
		mChorusPosition[0] = (1.0f + sine(mChorusTick + 0.00f)) * halfdepth;
		mChorusPosition[1] = (1.0f + sine(mChorusTick + 0.25f)) * halfdepth;
		mChorusPosition[2] = (1.0f + sine(mChorusTick + 0.50f)) * halfdepth;
        #else
		mChorusPosition[0] = (1.0f + sine((mChorusTick + 0.00f) * FMOD_PI2)) * halfdepth;
		mChorusPosition[1] = (1.0f + sine((mChorusTick + 0.25f) * FMOD_PI2)) * halfdepth;
		mChorusPosition[2] = (1.0f + sine((mChorusTick + 0.50f) * FMOD_PI2)) * halfdepth;
        #endif

        mChorusPosition[0] = halfchorusbufferlength + (mChorusPosition[0] * mChorusBufferLength);
        mChorusPosition[1] = halfchorusbufferlength + (mChorusPosition[1] * mChorusBufferLength);
        mChorusPosition[2] = halfchorusbufferlength + (mChorusPosition[2] * mChorusBufferLength);
        
        mChorusTick += mChorusSpeed;
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
// VERIFY: 2dcfbf63 manual
// OFFSET: 0x008EBFF0
FMOD_RESULT DSPChorus::setParameterInternal(int index, float value)
{
    float olddelay;

    olddelay = mDelay;

    mSystem->lockDSP();
   
    switch (index)
    {
        case FMOD_DSP_CHORUS_DRYMIX:
        {
            mDryMix = value;
            break;
        }
        case FMOD_DSP_CHORUS_WETMIX1:
        {
            mWetMix1 = value;
            break;
        }
        case FMOD_DSP_CHORUS_WETMIX2:
        {
            mWetMix2 = value;
            break;
        }
        case FMOD_DSP_CHORUS_WETMIX3:
        {
            mWetMix3 = value;
            break;
        }
        case FMOD_DSP_CHORUS_DELAY:
        {
            mDelay = value;
            break;
        }
        case FMOD_DSP_CHORUS_RATE:
        {
            mRateHz = value;
            break;
        }
        case FMOD_DSP_CHORUS_DEPTH:
        {
            mDepth = value;
            break;
        }
        case FMOD_DSP_CHORUS_FEEDBACK:
        {
            mFeedback = value;
            break;
        }
    }

    if (mDelay != olddelay)
    {
        mChorusBufferLength = (unsigned int)((float)mOutputRate * mDelay / 1000.0f) * 2;
        if (mChorusBufferLength < 4)
        {
            mChorusBufferLength = 4;
        }

        resetInternal();
    }

    mChorusSpeed = mRateHz / (float)mOutputRate;

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
    
	[SEE_ALSO]
]
*/
// VERIFY: dff428b6 auto
// OFFSET: 0x008EC130
FMOD_RESULT DSPChorus::getParameterInternal(int index, float *value, char *valuestr)
{
    switch (index)
    {
        case FMOD_DSP_CHORUS_DRYMIX:
        {
            *value = mDryMix;
            sprintf(valuestr, "%.02f", mDryMix);
            break;
        }
        case FMOD_DSP_CHORUS_WETMIX1:
        {
            *value = mWetMix1;
            sprintf(valuestr, "%.02f", mWetMix1);
            break;
        }
        case FMOD_DSP_CHORUS_WETMIX2:
        {
            *value = mWetMix2;
            sprintf(valuestr, "%.02f", mWetMix2);
            break;
        }
        case FMOD_DSP_CHORUS_WETMIX3:
        {
            *value = mWetMix3;
            sprintf(valuestr, "%.02f", mWetMix3);
            break;
        }
        case FMOD_DSP_CHORUS_DELAY:
        {
            *value = mDelay;
            sprintf(valuestr, "%.02f", mDelay);
            break;
        }
        case FMOD_DSP_CHORUS_RATE:
        {
            *value = mRateHz;
            sprintf(valuestr, "%.02f", mRateHz);
            break;
        }
        case FMOD_DSP_CHORUS_DEPTH:
        {
            *value = mDepth;
            sprintf(valuestr, "%.02f", mDepth);
            break;
        }
        case FMOD_DSP_CHORUS_FEEDBACK:
        {
            *value = mFeedback;
            sprintf(valuestr, "%.02f", mFeedback);
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
    
	[SEE_ALSO]
]
*/
// VERIFY: 82ed0738 manual
// OFFSET: 0x008EC240
FMOD_RESULT F_CALLBACK DSPChorus::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPChorus *chorus = (DSPChorus *)dsp;

    return chorus->createInternal();
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
// VERIFY: 4651ec5a manual
// OFFSET: 0x008EC270
FMOD_RESULT F_CALLBACK DSPChorus::releaseCallback(FMOD_DSP_STATE *dsp)
{
    DSPChorus *chorus = (DSPChorus *)dsp;

    return chorus->releaseInternal();
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
// VERIFY: 609a8ba8 manual
// OFFSET: 0x008EC2C0
FMOD_RESULT F_CALLBACK DSPChorus::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPChorus *chorus = (DSPChorus *)dsp;

    return chorus->resetInternal();
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
// VERIFY: 9145735f auto
// OFFSET: 0x008EC300
FMOD_RESULT F_CALLBACK DSPChorus::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPChorus *chorus = (DSPChorus *)dsp;

    return chorus->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
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
// VERIFY: a0524f3f manual
// OFFSET: 0x008EC330
FMOD_RESULT F_CALLBACK DSPChorus::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPChorus *chorus = (DSPChorus *)dsp;

    return chorus->setParameterInternal(index, value);
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
// VERIFY: 47385c62 auto
// OFFSET: 0x008EC370
FMOD_RESULT F_CALLBACK DSPChorus::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPChorus *chorus = (DSPChorus *)dsp;

    return chorus->getParameterInternal(index, value, valuestr);
}




}

#endif
