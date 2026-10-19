#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_VSTPLUGIN

#include "fmod.h"
#include "fmod_dspi.h"
#include "fmod_dsp_vstplugin.h"
#include "fmod_memory.h"
#include "fmod_os_misc.h"
#include "fmod_systemi.h"


#define BUFFERSIZE 256

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dspvstplugin;

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
        return DSPVSTPlugin::getDescriptionEx();
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
FMOD_DSP_DESCRIPTION_EX *DSPVSTPlugin::getDescriptionEx()
{
    FMOD_memset(&dspvstplugin, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    // name is stamped in later.
    dspvstplugin.create             = DSPVSTPlugin::createCallback;
    dspvstplugin.release            = DSPVSTPlugin::releaseCallback;
    dspvstplugin.reset              = DSPVSTPlugin::resetCallback;
    dspvstplugin.read               = DSPVSTPlugin::readCallback;

    dspvstplugin.setparameter       = DSPVSTPlugin::setParameterCallback;
    dspvstplugin.getparameter       = DSPVSTPlugin::getParameterCallback;
    dspvstplugin.config             = DSPVSTPlugin::configCallback;

    dspvstplugin.mType              = FMOD_DSP_TYPE_VSTPLUGIN;
    dspvstplugin.mCategory          = FMOD_DSP_CATEGORY_FILTER;
    dspvstplugin.mSize              = sizeof(DSPVSTPlugin);

    return &dspvstplugin;
}


float  *DSPVSTPlugin::mDeInterleavedInputBuffer[DSP_MAXLEVELS_MAX]    = { 0 };
float  *DSPVSTPlugin::mDeInterleavedOutputBuffer[DSP_MAXLEVELS_MAX]   = { 0 };
int     DSPVSTPlugin::mDeInterleavedBufferCount    = 0;


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
long DSPVSTPlugin::audioMasterCB(AEffect *effect, long opcode, long index, long value, void *ptr, float opt)
{
    switch(opcode)
    {
        case audioMasterAutomate:
        {
            return 0;
        }
        case audioMasterVersion:
        {
            return 2300;
        }
        case audioMasterCurrentId:
        {
            return 0;
        }
        case audioMasterIdle:
        {
            return 0;
        }
        case audioMasterPinConnected:
        {
            return 0;
        }
    }

    return 0;
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
FMOD_RESULT DSPVSTPlugin::createInternal()
{
    unsigned int    buffersize;
    int             samplerate;

    init();

    /*
        Initialise the plugin
    */
    mSystem->getDSPBufferSize(&buffersize, 0);
    mSystem->getSoftwareFormat(&samplerate, 0, 0, 0, 0, 0);

    mEffect->dispatcher(mEffect, effOpen, 0, 0, 0, 0);
    mEffect->dispatcher(mEffect, effSetSampleRate, 0, 0, 0, (float)samplerate);
    mEffect->dispatcher(mEffect, effSetBlockSize, 0, buffersize, 0, 0);
    mEffect->dispatcher(mEffect, effMainsChanged, 0, 1, 0, 0); // This calls Resume() in the plugin

    /*
        Allocate memory for the deinterleaved buffers
    */
    if (mDeInterleavedBufferCount == 0)
    {
        int count;

        for (count = 0; count < DSP_MAXLEVELS_MAX; count ++)
        {
            mDeInterleavedInputBuffer[count] = (float *)FMOD_Memory_Alloc(sizeof(float) * BUFFERSIZE);
            if (!mDeInterleavedInputBuffer[count])
            {
                return FMOD_ERR_MEMORY;
            }
        }

        for (count = 0; count < DSP_MAXLEVELS_MAX; count ++)
        {
            mDeInterleavedOutputBuffer[count] = (float *)FMOD_Memory_Alloc(sizeof(float) * BUFFERSIZE);
            if (!mDeInterleavedOutputBuffer[count])
            {
                return FMOD_ERR_MEMORY;
            }
        }
    }

    mDeInterleavedBufferCount++;

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
FMOD_RESULT DSPVSTPlugin::releaseInternal()
{
    mEffect->dispatcher(mEffect, effMainsChanged, 0, 0, 0, 0);

    mDeInterleavedBufferCount--;

    if (mDeInterleavedBufferCount == 0)
    {
        int i;

        for (i = 0; i < DSP_MAXLEVELS_MAX ; i++)
        {
            if (mDeInterleavedInputBuffer[i])
            {
                FMOD_Memory_Free(mDeInterleavedInputBuffer[i]);
                mDeInterleavedInputBuffer[i] = 0;
            }
            if (mDeInterleavedOutputBuffer[i])
            {
                FMOD_Memory_Free(mDeInterleavedOutputBuffer[i]);
                mDeInterleavedOutputBuffer[i] = 0;
            }
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
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPVSTPlugin::resetInternal()
{
    mEffect->dispatcher(mEffect, effMainsChanged, 0, 0, 0, 0);   // suspend
    mEffect->dispatcher(mEffect, effMainsChanged, 0, 1, 0, 0);   // resume

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
FMOD_RESULT DSPVSTPlugin::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    unsigned int    remaining = length;
    int             i, j;
    bool            downmix = false;

	float *in = inbuffer;

    if (!inbuffer)
    {
        return FMOD_OK;
    }

    /*
        Downmix to mono if thats what the vst plugin wants
    */
    if (inchannels > 1 && mEffect->numInputs == 1)
    {
        downmix = true;
    }
    else if (inchannels != mEffect->numInputs)
    {
        FMOD_memcpy(outbuffer, inbuffer, length * outchannels * sizeof(float));
        return FMOD_ERR_FORMAT;
    }
    else if (outchannels != mEffect->numOutputs)
    {
        FMOD_memcpy(outbuffer, inbuffer, length * outchannels * sizeof(float));
        return FMOD_ERR_FORMAT;
    }

    /*
        Clear the output buffer
    */
    for (i = 0; i < outchannels; i++)
    {
        FMOD_memset(mDeInterleavedOutputBuffer[i], 0, sizeof(float) * BUFFERSIZE);
    }

    while(remaining > 0)
    {
        int size = remaining;

        if (size > BUFFERSIZE)
        {
            size = BUFFERSIZE;
        }

        /*
            If the plugin expects mono, downmix inbuffer to mono
        */
        if (downmix)
        {
            for (i = 0; i < size; i++)
            {
                float data = 0.0f;

                for (j = 0; j < inchannels; j++)
                {
                    data += *inbuffer++;
                }
                
                mDeInterleavedInputBuffer[0][i] = data / inchannels;
            }
        }
        else
        {
            /*
                DeInterleave
            */
            for (i = 0; i < size; i++)
            {
                for (j = 0; j < inchannels; j++)
                {
                    mDeInterleavedInputBuffer[j][i] = *inbuffer++;
                }
            }
        }

        /*
            Process data
        */
        if (mEffect)
        {
            mEffect->dispatcher(mEffect, effStartProcess, 0, 0, 0, 0);
            
            if (mEffect->processReplacing)
            {
                mEffect->processReplacing(mEffect, mDeInterleavedInputBuffer, mDeInterleavedOutputBuffer, size);
            }
            else if (mEffect->process)
            {
                mEffect->process(mEffect, mDeInterleavedInputBuffer, mDeInterleavedOutputBuffer, size);
            }

            mEffect->dispatcher(mEffect, effStopProcess, 0, 0, 0, 0);
        }

        /*
            Interleave back into buffer
        */
        if (downmix)
        {
            /*
                If the input was downmixed, duplicate the resulting mono buffer for each output channel
            */
            for (i = 0; i < size; i++)
            {
                for (j = 0; j < outchannels; j++)
                {
					*outbuffer++ = mDeInterleavedOutputBuffer[0][i];
                }
            }
        }
        else
        {
            for (i = 0; i < size; i++)
            {
                for (j = 0; j < outchannels; j++)
                {
					*outbuffer++ = mDeInterleavedOutputBuffer[j][i];
					in++;
                }
            }
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
FMOD_RESULT DSPVSTPlugin::setParameterInternal(int index, float value)
{
    mSystem->lockDSP();
    {
        mEffect->setParameter(mEffect, index, value);
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
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPVSTPlugin::getParameterInternal(int index, float *value, char *valuestr)
{
    *value = mEffect->getParameter(mEffect, index);

    mEffect->dispatcher(mEffect, effGetParamDisplay, index, 0, valuestr, 0);

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
FMOD_RESULT DSPVSTPlugin::showConfigDialogInternal(void *hwnd, int show)
{
    if (mEffect->flags & effFlagsHasEditor)
    {
        mEffect->dispatcher(mEffect, (show ? effEditOpen : effEditClose), 0, 0, hwnd, 0);
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
    Win32

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK DSPVSTPlugin::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPVSTPlugin *vst = (DSPVSTPlugin *)dsp;

    return vst->createInternal();
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
FMOD_RESULT F_CALLBACK DSPVSTPlugin::releaseCallback(FMOD_DSP_STATE *dsp)
{
    DSPVSTPlugin *vst = (DSPVSTPlugin *)dsp;

    return vst->releaseInternal();
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
FMOD_RESULT F_CALLBACK DSPVSTPlugin::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPVSTPlugin *vst = (DSPVSTPlugin *)dsp;

    return vst->resetInternal();
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
FMOD_RESULT F_CALLBACK DSPVSTPlugin::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPVSTPlugin *vst = (DSPVSTPlugin *)dsp;

    return vst->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
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
FMOD_RESULT F_CALLBACK DSPVSTPlugin::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPVSTPlugin *vst = (DSPVSTPlugin *)dsp;

    return vst->setParameterInternal(index, value);
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
FMOD_RESULT F_CALLBACK DSPVSTPlugin::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPVSTPlugin *vst = (DSPVSTPlugin *)dsp;

    return vst->getParameterInternal(index, value, valuestr);
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
FMOD_RESULT F_CALLBACK DSPVSTPlugin::configCallback(FMOD_DSP_STATE *dsp, void *hwnd, int show)
{
    DSPVSTPlugin *vst = (DSPVSTPlugin *)dsp;

    return vst->showConfigDialogInternal(hwnd, show);
}
}

#endif
