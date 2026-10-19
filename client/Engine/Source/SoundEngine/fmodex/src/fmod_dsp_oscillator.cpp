#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OSCILLATOR

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_oscillator.h"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_systemi_spu.h"
    #include <spu_intrinsics.h>
#else
    #include "fmod_systemi.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#ifdef PLATFORM_PS3
extern unsigned int _binary_spu_fmod_dsp_oscillator_pic_start[];
#endif

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dsposcillator;

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
        return DSPOscillator::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */

#ifndef PLATFORM_PS3_SPU

FMOD_DSP_PARAMETERDESC dsposcillator_param[2] =
{
    { 0.0f, 5.0f,       0.0f, "Oscillator type",   "", "Select a waveform type" },
    { 1.0f, 22000.0f, 220.0f, "Frequency",       "hz", "Playback frequency of tone, for example music note A above middle C is 440.0." }
};

#endif

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
// VERIFY: 8473d14d auto
// OFFSET: 0x008F52A0
FMOD_DSP_DESCRIPTION_EX *DSPOscillator::getDescriptionEx()
{
#ifndef PLATFORM_PS3_SPU
    FMOD_memset(&dsposcillator, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dsposcillator.name, "FMOD Oscillator");
    dsposcillator.version       = 0x00010100;
    dsposcillator.channels      = 1;
    dsposcillator.create        = DSPOscillator::createCallback;
    dsposcillator.release       = DSPOscillator::releaseCallback;

    #ifdef PLATFORM_PS3
    dsposcillator.read          = (FMOD_DSP_READCALLBACK)_binary_spu_fmod_dsp_oscillator_pic_start;    /* SPU PIC entry address */
    #else
    dsposcillator.read          = DSPOscillator::readCallback;
    #endif

    dsposcillator.numparameters = sizeof(dsposcillator_param) / sizeof(dsposcillator_param[0]);
    dsposcillator.paramdesc     = dsposcillator_param;
    dsposcillator.setparameter  = DSPOscillator::setParameterCallback;
    dsposcillator.getparameter  = DSPOscillator::getParameterCallback;

    dsposcillator.mType         = FMOD_DSP_TYPE_OSCILLATOR;
    dsposcillator.mSize         = sizeof(DSPOscillator);
    dsposcillator.mCategory     = FMOD_DSP_CATEGORY_FILTER;


#else
    dsposcillator.read          = DSPOscillator::readCallback;                  /* We only care about read function on SPU */
#endif
    return &dsposcillator;
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
// VERIFY: 9ff1a8f8 auto
// OFFSET: 0x008EAC00
FMOD_RESULT DSPOscillator::createInternal()
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
FMOD_RESULT DSPOscillator::releaseInternal()
{
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
// VERIFY: b121352c manual
// OFFSET: 0x008F4920
FMOD_RESULT DSPOscillator::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    unsigned int count;

    switch (mType)
    {
        case 0:     // sine
        {
            #ifdef PLATFORM_PS3_SPU

            vector float *out = (vector float *)outbuffer;

            for (count = 0; count < length; count+=4)
            {
                vector float position;

                float position0 = mPosition;
                float position1 = position0 + (mRate * (FMOD_PI * 2));
                float position2 = position1 + (mRate * (FMOD_PI * 2));
                float position3 = position2 + (mRate * (FMOD_PI * 2));

                if (position1 >= (FMOD_PI * 2.0f))
                {
                    position1 -= (FMOD_PI * 2.0f);
                }
                if (position2 >= (FMOD_PI * 2.0f))
                {
                    position2 -= (FMOD_PI * 2.0f);
                }
                if (position3 >= (FMOD_PI * 2.0f))
                {
                    position3 -= (FMOD_PI * 2.0f);
                }

                position = spu_insert(position0, position, 0);
                position = spu_insert(position1, position, 1);
                position = spu_insert(position2, position, 2);
                position = spu_insert(position3, position, 3);

                *out = sinf4(position);
                out++;

                mPosition = position3 + (mRate * (FMOD_PI * 2));
                if (mPosition >= (FMOD_PI * 2.0f))
                {
                    mPosition -= (FMOD_PI * 2.0f);
                }
            }

            #else
            
            for (count = 0; count < length; count++)
            {
                outbuffer[count] = FMOD_SIN(mPosition);

                mPosition += (mRate * (FMOD_PI * 2));
                if (mPosition >= (FMOD_PI * 2.0f))
                {
                    mPosition -= (FMOD_PI * 2.0f);
                }
            }

            #endif
            break;
        }
        case 1:     // square
        {
            for (count = 0; count < length; count++)
            {
                outbuffer[count] = 1.0f * (float)mDirection;
                mPosition += mRate;
                if (mPosition >= 1.0f)
                {
                    mPosition -= 1.0f;
                    mDirection = -mDirection;
                }
            }
            break;
        }
        case 2:     // saw up
        {
            for (count = 0; count < length; count++)
            {
                outbuffer[count] = (mPosition * 2.0f) - 1.0f;
                mPosition += mRate;
                if (mPosition >= 1.0f)
                {
                    mPosition -= 1.0f;
                }
            }
            break;
        }
        case 3:     // saw down
        {
            for (count = 0; count < length; count++)
            {
                outbuffer[count] = (mPosition * -2.0f) + 1.0f;;

                mPosition += mRate;
                if (mPosition >= 1.0f)
                {
                    mPosition -= 1.0f;
                }
            }
            break;
        }
        case 4:     // triangle
        {
            for (count = 0; count < length; count++)
            {
                outbuffer[count] = mPosition;

                mPosition += (mRate * (float)mDirection * 2.0f);
                if (mPosition > 1.0f || mPosition < -1.0f)
                {
                    mDirection = -mDirection;
                    mPosition += (mRate * (float)mDirection * 2.0f);
                }
            }
            break;
        }
        case 5:     // noise
        {
            for (count = 0; count < length; count++)
            {
                outbuffer[count] = ((float)(FMOD_RAND()%32768) / 16384.0f) - 1.0f;
            }
            break;
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
// VERIFY: e16d4a24 auto
// OFFSET: 0x008F5090
FMOD_RESULT DSPOscillator::setParameterInternal(int index, float value)
{
    FMOD_RESULT result;
    int outputrate;

    switch (index)
    {
        case FMOD_DSP_OSCILLATOR_TYPE:
        {
            mType = (int)value;
            break;
        }
        case FMOD_DSP_OSCILLATOR_RATE:
        {
            mRateHz = value;
            break;
        }
    }

    mDirection = 1;
    result = mSystem->getSoftwareFormat(&outputrate, 0, 0, 0, 0, 0 );
    if (result != FMOD_OK)
    {
        return result;
    }

    mRate = mRateHz / (float)outputrate;

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
// VERIFY: 4df20866 auto
// OFFSET: 0x008F50F0
FMOD_RESULT DSPOscillator::getParameterInternal(int index, float *value, char *valuestr)
{   
    switch (index)
    {
        case FMOD_DSP_OSCILLATOR_TYPE:
        {
            *value = (float)mType;
            switch (mType)
            {
                case 0: { FMOD_strcpy(valuestr, "sine");     break; }
                case 1: { FMOD_strcpy(valuestr, "square");   break; }
                case 2: { FMOD_strcpy(valuestr, "saw up");   break; }
                case 3: { FMOD_strcpy(valuestr, "saw down"); break; }
                case 4: { FMOD_strcpy(valuestr, "triangle"); break; }
                case 5: { FMOD_strcpy(valuestr, "noise");    break; }
            }
            break;
        }
        case FMOD_DSP_OSCILLATOR_RATE:
        {
            *value = mRateHz;
            sprintf(valuestr, "%.02f", mRateHz);
            break;
        }
    }

    return FMOD_OK;
}


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
// VERIFY: 8969ce7d manual
// OFFSET: 0x008EB200
FMOD_RESULT F_CALLBACK DSPOscillator::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPOscillator *osc = (DSPOscillator *)dsp;

    return osc->createInternal();
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
FMOD_RESULT F_CALLBACK DSPOscillator::releaseCallback(FMOD_DSP_STATE *dsp)
{
    DSPOscillator *osc = (DSPOscillator *)dsp;

    return osc->releaseInternal();
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
// VERIFY: a80010f7 auto
// OFFSET: 0x008F5200
FMOD_RESULT F_CALLBACK DSPOscillator::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPOscillator *osc = (DSPOscillator *)dsp;

    return osc->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
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
// VERIFY: 3a0dcf44 manual
// OFFSET: 0x008F5230
FMOD_RESULT F_CALLBACK DSPOscillator::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPOscillator *osc = (DSPOscillator *)dsp;

    return osc->setParameterInternal(index, value);
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
// VERIFY: ab077c3c auto
// OFFSET: 0x008F5270
FMOD_RESULT F_CALLBACK DSPOscillator::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPOscillator *osc = (DSPOscillator *)dsp;

    return osc->getParameterInternal(index, value, valuestr);
}

#endif //!PLATFORM_PS3_SPU

}

#endif
