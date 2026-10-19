#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_DSPCODEC

#include "fmod.h"
#include "fmod_channel_software.h"
#include "fmod_codeci.h"
#include "fmod_dspi.h"
#include "fmod_dsp_codec.h"
#include "fmod_sample_software.h"
#include "fmod_soundi.h"

#include "fmod_systemi.h"

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dspcodec;

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
        return DSPCodec::getDescriptionEx();
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
    
	[SEE_ALSO]
]
*/
// VERIFY: b362e9e0 auto
// OFFSET: 0x00927000
FMOD_DSP_DESCRIPTION_EX *DSPCodec::getDescriptionEx()
{
    FMOD_memset(&dspcodec, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dspcodec.name, "FMOD DSP Codec");
    dspcodec.version       = 0x00010100;
    dspcodec.create        = DSPCodec::createCallback;
    dspcodec.release       = DSPCodec::releaseCallback;
    dspcodec.reset         = DSPCodec::resetCallback;
    dspcodec.read          = DSPCodec::readCallback;
    dspcodec.setposition   = DSPCodec::setPositionCallback;

    dspcodec.numparameters = 0;
    dspcodec.paramdesc     = 0;
    dspcodec.setparameter  = DSPCodec::setParameterCallback;
    dspcodec.getparameter  = DSPCodec::getParameterCallback;

    dspcodec.mType         = (FMOD_DSP_TYPE)FMOD_DSP_TYPE_CODECREADER;
    dspcodec.mSize         = sizeof(DSPCodec);

    return &dspcodec;
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
FMOD_RESULT DSPCodec::createInternal()
{
    init();

    mNewPosition = 0xFFFFFFFF;

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
// VERIFY: f5c3e87d auto
// OFFSET: 0x00926C20
FMOD_RESULT DSPCodec::release(bool freethis)
{
    FMOD_RESULT result;

    result = DSPResampler::release(false);

    if (freethis)
    {
        FMOD_Memory_FreeType(this, mWaveFormat.format == FMOD_SOUND_FORMAT_XMA ? FMOD_MEMORY_XBOX360_PHYSICAL : FMOD_MEMORY_NORMAL);
    }

    return result;
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
FMOD_RESULT DSPCodec::releaseInternal()
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
    
	[SEE_ALSO]
]
*/
FMOD_RESULT DSPCodec::resetInternal()
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
    
	[SEE_ALSO]
]
*/
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
// VERIFY: a533aa87 auto
// OFFSET: 0x00926CD0
FMOD_RESULT DSPCodec::readInternal(signed short *inbuffer, signed short *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    FMOD_RESULT result;
    unsigned int size = length;
    unsigned int bytespersample;
    int retries = 0;

    if (mNewPosition != 0xFFFFFFFF)
    {
        mPosition = mNewPosition;
        mCodec->reset();
        mCodec->setPosition(0, mPosition, FMOD_TIMEUNIT_PCM);
        mNewPosition = 0xFFFFFFFF;
    }

    SoundI::getBytesFromSamples(1, &bytespersample, mDescription.channels, mDescription.mFormat);

    while (size)
    {
        unsigned int endpoint, toread;
        unsigned int read;

        if (mMode & FMOD_LOOP_NORMAL && mLoopCount)
        {
            endpoint = mLoopStart + mLoopLength - 1;
        }
        else
        {
            if (mCodec->mFlags & FMOD_CODEC_ACCURATELENGTH)
            {
                endpoint = mLength - 1;
            }
            else
            {
                endpoint = (unsigned int)-1;
            }
        }

        toread = size;
        if (mPosition > endpoint)
        {
            toread = 0;
        }
        else if (mPosition + toread > endpoint)
        {
            toread = (endpoint - mPosition) + 1;
        }

        result = mCodec->read(outbuffer, toread * bytespersample, &read);
        if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
        {
            return result;
        }

        read /= bytespersample;

        mPosition += read;
        size      -= read;
        outbuffer += (read * inchannels);

        if (!read)
        {
            if (retries > 255)
            {
                result = FMOD_ERR_FILE_EOF;
            }
            retries++;
        }

        if (result == FMOD_ERR_FILE_EOF || mPosition > endpoint)
        {
            if (mMode & FMOD_LOOP_NORMAL && mLoopCount && retries <= 255)
            {
                mPosition = mLoopStart;
                mCodec->setPosition(0, mLoopStart, FMOD_TIMEUNIT_PCM);

                if (mLoopCount > 0)
                {
                    mLoopCount--;
                }
            }
            else
            {
                FMOD_memset(outbuffer, 0, size * bytespersample);
                return FMOD_ERR_FILE_EOF;
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
    
	[SEE_ALSO]
]
*/
FMOD_RESULT DSPCodec::setPositionInternal(unsigned int position)
{
    mNewPosition = position;

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
FMOD_RESULT DSPCodec::setParameterInternal(int index, float value)
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
    
	[SEE_ALSO]
]
*/
FMOD_RESULT DSPCodec::getParameterInternal(int index, float *value, char *valuestr)
{
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
    
	[SEE_ALSO]
]
*/
// VERIFY: bbe3914f manual
// OFFSET: 0x00926C70
FMOD_RESULT F_CALLBACK DSPCodec::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPCodec *dspcodec = (DSPCodec *)dsp;

    return dspcodec->createInternal();
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
FMOD_RESULT F_CALLBACK DSPCodec::releaseCallback(FMOD_DSP_STATE *dsp)
{
    DSPCodec *dspcodec = (DSPCodec *)dsp;

    return dspcodec->releaseInternal();
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
FMOD_RESULT F_CALLBACK DSPCodec::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPCodec *dspcodec = (DSPCodec *)dsp;

    return dspcodec->resetInternal();
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
// VERIFY: da7b32bc auto
// OFFSET: 0x00926FD0
FMOD_RESULT F_CALLBACK DSPCodec::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPCodec *dspcodec = (DSPCodec *)dsp;

    return dspcodec->readInternal((signed short *)inbuffer, (signed short *)outbuffer, length, inchannels, outchannels);
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
// VERIFY: 03a5e511 manual
// OFFSET: 0x00926CA0
FMOD_RESULT F_CALLBACK DSPCodec::setPositionCallback(FMOD_DSP_STATE *dsp, unsigned int pos)
{
    DSPCodec *dspcodec = (DSPCodec *)dsp;

    return dspcodec->setPositionInternal(pos);
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
FMOD_RESULT F_CALLBACK DSPCodec::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPCodec *dspcodec = (DSPCodec *)dsp;

    return dspcodec->setParameterInternal(index, value);
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
FMOD_RESULT F_CALLBACK DSPCodec::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPCodec *dspcodec = (DSPCodec *)dsp;

    return dspcodec->getParameterInternal(index, value, valuestr);
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
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/


}

#endif
