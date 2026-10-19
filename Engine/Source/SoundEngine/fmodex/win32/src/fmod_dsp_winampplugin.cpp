#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_WINAMPPLUGIN

#include "fmod.h"
#include "fmod_dsp_winampplugin.h"
#include "fmod_systemi.h"

#define BUFFERSIZE 256

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dspwinampplugin;

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
        return DSPWinampPlugin::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
FMOD_DSP_DESCRIPTION_EX *DSPWinampPlugin::getDescriptionEx()
{
    FMOD_memset(&dspwinampplugin, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    // name and version is stamped in later.
    dspwinampplugin.create             = DSPWinampPlugin::createCallback;
    dspwinampplugin.release            = DSPWinampPlugin::releaseCallback;
    dspwinampplugin.reset              = DSPWinampPlugin::resetCallback;
    dspwinampplugin.read               = DSPWinampPlugin::readCallback;

    dspwinampplugin.setparameter       = DSPWinampPlugin::setParameterCallback;
    dspwinampplugin.getparameter       = DSPWinampPlugin::getParameterCallback;
    dspwinampplugin.config             = DSPWinampPlugin::configCallback;

    dspwinampplugin.mType              = FMOD_DSP_TYPE_WINAMPPLUGIN;
    dspwinampplugin.mCategory          = FMOD_DSP_CATEGORY_FILTER;
    dspwinampplugin.mSize              = sizeof(DSPWinampPlugin);

    return &dspwinampplugin;
}


short *DSPWinampPlugin::mEffectBuffer       = NULL;
int    DSPWinampPlugin::mEffectBufferCount  = 0;


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPWinampPlugin::createInternal()
{
    int result;

    if (mEffectBufferCount == 0)
    {
        mEffectBuffer = (short *)FMOD_Memory_Alloc(sizeof(short) * BUFFERSIZE);
        if (!mEffectBuffer)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    mEffectBufferCount++;

    result = mEffect->Init(mEffect);

    if (result)
    {
        return FMOD_ERR_PLUGIN;
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
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPWinampPlugin::releaseInternal()
{
    mEffectBufferCount--;

    if (mEffectBufferCount == 0)
    {
        FMOD_Memory_Free(mEffectBuffer);
        mEffectBuffer = NULL;
    }

    mEffect->Quit(mEffect);

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
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPWinampPlugin::resetInternal()
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
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPWinampPlugin::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{ 
    int count;
    int rate;
    int remaining = length * outchannels;

    mSystem->getSoftwareFormat(&rate, 0, 0, 0, 0, 0);

    while(remaining > 0)
    {
        int size = remaining;

        if (size > BUFFERSIZE)
        {
            size = BUFFERSIZE;
        }

        /*
            Convert from float to short
        */
        for (count = 0; count < size; count++)
        {
            float val = *inbuffer++;

            mEffectBuffer[count] = (short)(val < -1.0f ? -32767 : val > 1.0f ? 32767 : val * 32767.0f);

        }

        mEffect->ModifySamples(mEffect, mEffectBuffer, size / outchannels, 16, outchannels, rate);

        /*
            Convert back to floating point data
        */
        for (count = 0; count < size; count++)
        {
            *outbuffer++ = (float)mEffectBuffer[count] / 32767.0f;
        }

        remaining -= size;
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
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPWinampPlugin::setParameterInternal(int index, float value)
{
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPWinampPlugin::getParameterInternal(int index, float *value, char *valuestr)
{
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPWinampPlugin::showConfigDialogInternal(void *hwnd, int show)
{
    mEffect->hwndParent = hwnd;

    mEffect->Config(mEffect);

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
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK DSPWinampPlugin::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPWinampPlugin *winamp = (DSPWinampPlugin *)dsp;

    return winamp->createInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK DSPWinampPlugin::releaseCallback(FMOD_DSP_STATE *dsp)
{
    DSPWinampPlugin *winamp = (DSPWinampPlugin *)dsp;

    return winamp->releaseInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK DSPWinampPlugin::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPWinampPlugin *winamp = (DSPWinampPlugin *)dsp;

    return winamp->resetInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK DSPWinampPlugin::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPWinampPlugin *winamp = (DSPWinampPlugin *)dsp;

    return winamp->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK DSPWinampPlugin::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPWinampPlugin *winamp = (DSPWinampPlugin *)dsp;

    return winamp->setParameterInternal(index, value);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK DSPWinampPlugin::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPWinampPlugin *winamp = (DSPWinampPlugin *)dsp;

    return winamp->getParameterInternal(index, value, valuestr);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK DSPWinampPlugin::configCallback(FMOD_DSP_STATE *dsp, void *hwnd, int show)
{
    DSPWinampPlugin *winamp = (DSPWinampPlugin *)dsp;

    return winamp->showConfigDialogInternal(hwnd, show);
}
}

#endif