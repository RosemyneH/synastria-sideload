#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_ECHO

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_echo.h"
#include "fmod_systemi.h"

#include <stdio.h>

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dspecho_desc;

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
        return DSPEcho::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


FMOD_DSP_PARAMETERDESC dspecho_param[5] =
{
    { 1.0f,  5000.0f,   500.0f, "Delay",        "ms",       "Echo delay in ms.  10  to 5000.  Default = 500." },
    { 0.0f,     1.0f,     0.5f, "Decay",        "%",        "Echo decay per delay.  0 to 1.  1.0 = No decay, 0.0 = total decay.  Default = 0.5." },
    { 0.0f,    16.0f,     0.0f, "Max channels", "channels", "Maximum channels supported.  0 to 16.  0 = same as fmod's default output polyphony, 1 = mono, 2 = stereo etc.  Default = 0.  It is suggested to leave at 0!" },
    { 0.0f,     1.0f,     1.0f, "Drymix",       "%",        "Volume of original signal to pass to output.  0.0 to 1.0. Default = 1.0." },
    { 0.0f,     1.0f,     1.0f, "Wetmix",       "%",        "Volume of echo delay signal to pass to output.  0.0 to 1.0. Default = 1.0." },
};

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
// VERIFY: 99467b23 auto
// OFFSET: 0x008F0910
FMOD_DSP_DESCRIPTION_EX *DSPEcho::getDescriptionEx()
{
    FMOD_memset(&dspecho_desc, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dspecho_desc.name, "FMOD Echo");
    dspecho_desc.version       = 0x00010100;
    dspecho_desc.create        = DSPEcho::createCallback;
    dspecho_desc.release       = DSPEcho::releaseCallback;
    dspecho_desc.reset         = DSPEcho::resetCallback;
    dspecho_desc.read          = DSPEcho::readCallback;

    dspecho_desc.numparameters = sizeof(dspecho_param) / sizeof(dspecho_param[0]);
    dspecho_desc.paramdesc     = dspecho_param;
    dspecho_desc.setparameter  = DSPEcho::setParameterCallback;
    dspecho_desc.getparameter  = DSPEcho::getParameterCallback;

    dspecho_desc.mType         = FMOD_DSP_TYPE_ECHO;
    dspecho_desc.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dspecho_desc.mSize         = sizeof(DSPEcho);

    return &dspecho_desc;
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
// VERIFY: 81af4d9d auto
// OFFSET: 0x008EFEB0
FMOD_RESULT DSPEcho::createInternal()
{
    int count;

    init();

    mChannels = 0;

    for (count = 0; count < mDescription.numparameters; count++)
    {
        FMOD_RESULT result;

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
FMOD_RESULT DSPEcho::releaseInternal()
{
    if (mEchoBuffer)
    {
        FMOD_Memory_Free(mEchoBuffer);
        mEchoBuffer = 0;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPEcho::resetInternal()
{
    mEchoPosition = 0;

    if (mEchoBuffer)
    {
        FMOD_memset(mEchoBuffer, 0, mEchoBufferLengthBytes);
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 9a45ab9d manual
// OFFSET: 0x008EFF00
FMOD_RESULT DSPEcho::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    unsigned int count;

    if (inchannels > mChannels || !mEchoBuffer)
    {
        FMOD_memcpy(outbuffer, inbuffer, length * outchannels * sizeof(float));
        return FMOD_OK;
    }

    if (inchannels == 1)
    {
        unsigned int len;
        float *out, *in;

        in  = inbuffer;
        out = outbuffer;

        len = length;
        while (len)
        {
            unsigned int len2 = len;
            float *echobuff;

            echobuff = mEchoBuffer + (mEchoPosition * 1);

            if (mEchoPosition + len > mEchoLength)
            {
                len2 = mEchoLength - mEchoPosition;
            }

            for (count = 0; count < len2; count++)
            {
                float in0 = in[0];

                out[0]      = (in0 * mDryMix) + (echobuff[0] * mWetMix);
                echobuff[0] = (in0          ) + (echobuff[0] * mDecayRatio);

                in++;
                out++;
                echobuff++;
	        }

		    mEchoPosition += len2;
		    if (mEchoPosition >= mEchoLength)
            {
                mEchoPosition = 0;
            }
            len -= len2;
        }
    }
    else if (inchannels == 2)
    {
        unsigned int len;
        float *out, *in;

        in  = inbuffer;
        out = outbuffer;

        len = length;
        while (len)
        {
            unsigned int len2 = len;
            float *echobuff;

            echobuff = mEchoBuffer + (mEchoPosition * 2);

            if (mEchoPosition + len > mEchoLength)
            {
                len2 = mEchoLength - mEchoPosition;
            }

            for (count = 0; count < len2; count++)
            {
                float in0 = in[0];
                float in1 = in[1];

                out[0] = (in0 * mDryMix) + (echobuff[0] * mWetMix);
                out[1] = (in1 * mDryMix) + (echobuff[1] * mWetMix);
                echobuff[0] = in0 + (echobuff[0] * mDecayRatio);
                echobuff[1] = in1 + (echobuff[1] * mDecayRatio);

                in+=2;
                out+=2;
                echobuff+=2;
	        }

		    mEchoPosition += len2;
		    if (mEchoPosition >= mEchoLength)
            {
                mEchoPosition = 0;
            }
            len -= len2;
        }
    }
    else
    {
        unsigned int len;
        float *out, *in;

        in  = inbuffer;
        out = outbuffer;

        len = length;
        while (len)
        {
            int count2;
            unsigned int len2 = len;
            float *echobuff;

            echobuff = mEchoBuffer + (mEchoPosition * inchannels);

            if (mEchoPosition + len > mEchoLength)
            {
                len2 = mEchoLength - mEchoPosition;
            }

            for (count = 0; count < len2; count++)
            {
                for (count2 = 0; count2 < inchannels; count2++)
                {
                    float in0 = in[count2];

                    out[count2]      = (in0 * mDryMix) + (echobuff[count2] * mWetMix);
                    echobuff[count2] = (in0          ) + (echobuff[count2] * mDecayRatio);
                }

                in+=inchannels;
                out+=inchannels;
                echobuff+=inchannels;
	        }

		    mEchoPosition += len2;
		    if (mEchoPosition >= mEchoLength)
            {
                mEchoPosition = 0;
            }
            len -= len2;
        }
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: ce8882df manual
// OFFSET: 0x008F0530
FMOD_RESULT DSPEcho::setParameterInternal(int index, float value)
{
    float olddelay    = mDelay;
    int   oldchannels = mChannels;

    mSystem->getSoftwareFormat(&mOutputRate, 0, &mChannels, 0, 0, 0);

    mSystem->lockDSP();

    switch (index)
    {
        case FMOD_DSP_ECHO_DELAY:
        {
            mDelay = value;
            break;
        }
        case FMOD_DSP_ECHO_DECAYRATIO:
        {
            mDecayRatio = value;
            break;
        }
        case FMOD_DSP_ECHO_MAXCHANNELS:
        {
            mMaxChannels = (int)value;
            if (mMaxChannels)
            {
                mChannels = mMaxChannels;
            }
            break;
        }
        case FMOD_DSP_ECHO_DRYMIX:
        {
            mDryMix = value;
            break;
        }
        case FMOD_DSP_ECHO_WETMIX:
        {
            mWetMix = value;
            break;
        }
    }

    if (mDelay != olddelay || mChannels != oldchannels || !mEchoBuffer)
    {
        mEchoLength = (int)((float)mOutputRate * mDelay) / 1000;

        if (mEchoBuffer)
        {
            FMOD_Memory_Free(mEchoBuffer);
        }

        mEchoBufferLengthBytes = mEchoLength;
        mEchoBufferLengthBytes *= mChannels;
        mEchoBufferLengthBytes *= sizeof(float);

        mEchoBuffer = (float *)FMOD_Memory_Calloc(mEchoBufferLengthBytes);
        if (!mEchoBuffer)
        {
            mSystem->unlockDSP();
            return FMOD_ERR_MEMORY;
        }

        resetInternal();
    }

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
// VERIFY: df8aaa8a auto
// OFFSET: 0x008F06D0
FMOD_RESULT DSPEcho::getParameterInternal(int index, float *value, char *valuestr)
{
    switch (index)
    {
        case FMOD_DSP_ECHO_DELAY:
        {
            *value = mDelay;
            sprintf(valuestr, "%.02f", mDelay);
            break;
        }
        case FMOD_DSP_ECHO_DECAYRATIO:
        {
            *value = mDecayRatio;
            sprintf(valuestr, "%.1f", mDecayRatio * 100.0f);
            break;
        }
        case FMOD_DSP_ECHO_MAXCHANNELS:
        {
            *value = (float)mMaxChannels;
            sprintf(valuestr, "%d", mMaxChannels);
            break;
        }
        case FMOD_DSP_ECHO_DRYMIX:
        {
            *value = mDryMix;
            sprintf(valuestr, "%.1f", mDryMix * 100.0f);
            break;
        }
        case FMOD_DSP_ECHO_WETMIX:
        {
            *value = mWetMix;
            sprintf(valuestr, "%.1f", mWetMix * 100.0f);
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
// VERIFY: bd95d5a1 manual
// OFFSET: 0x008F07B0
FMOD_RESULT F_CALLBACK DSPEcho::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPEcho *echo = (DSPEcho *)dsp;

    return echo->createInternal();
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
// VERIFY: 5cf90fbf manual
// OFFSET: 0x008F07E0
FMOD_RESULT F_CALLBACK DSPEcho::releaseCallback(FMOD_DSP_STATE *dsp)
{
    DSPEcho *echo = (DSPEcho *)dsp;

    return echo->releaseInternal();
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
// VERIFY: 478d6ddb manual
// OFFSET: 0x008F0830
FMOD_RESULT F_CALLBACK DSPEcho::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPEcho *echo = (DSPEcho *)dsp;

    return echo->resetInternal();
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
// VERIFY: 389787ac auto
// OFFSET: 0x008F0870
FMOD_RESULT F_CALLBACK DSPEcho::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPEcho *echo = (DSPEcho *)dsp;

    return echo->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
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
// VERIFY: 49258adc manual
// OFFSET: 0x008F08A0
FMOD_RESULT F_CALLBACK DSPEcho::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPEcho *echo = (DSPEcho *)dsp;

    return echo->setParameterInternal(index, value);
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
// VERIFY: de75c5bb auto
// OFFSET: 0x008F08E0
FMOD_RESULT F_CALLBACK DSPEcho::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPEcho *echo = (DSPEcho *)dsp;

    return echo->getParameterInternal(index, value, valuestr);
}




}

#endif
