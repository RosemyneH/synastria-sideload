#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_downmix.h"
#include "fmod_dsp_filter.h"
#include "fmod_dsp_soundcard.h"
#include "fmod_soundi.h"
#include "fmod_systemi.h"

namespace FMOD
{


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
// VERIFY: ab8a4448 manual
// OFFSET: 0x00929C20
FMOD_RESULT DSPSoundCard::alloc(FMOD_DSP_DESCRIPTION_EX *description)
{
    FMOD_RESULT result;

    result = DSPI::alloc(description);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (description->mFormat == FMOD_SOUND_FORMAT_PCMFLOAT)
    {
        mConvertBuffer = 0;
    }
    else
    {
        mConvertBufferMemory = FMOD_Memory_Calloc((mSystem->mDSPBlockSize * description->channels * sizeof(float)) + 16);
        if (!mConvertBufferMemory)
        {
            return FMOD_ERR_MEMORY;
        }
        mConvertBuffer = (float *)FMOD_ALIGNPOINTER(mConvertBufferMemory, 16);
    }

    updateTreeLevel(0); /* Soundcard units always start at level 0 */

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
// VERIFY: 0273aeed manual
// OFFSET: 0x00929D00
FMOD_RESULT DSPSoundCard::execute(void *inbuffer, void **outbuffer, unsigned int *length, int inchannels, int *outchannels, FMOD_SPEAKERMODE speakermode)
{
    FMOD_RESULT result;
    float      *readbuffer;

    if (!mConvertBuffer)
    {
        result = DSPFilter::execute((float *)inbuffer, (float **)outbuffer, length, inchannels, outchannels, speakermode);
        if (result != FMOD_OK)
        {
            return result;
        }
        return FMOD_OK;
    }

    readbuffer = 0;

    result = DSPFilter::execute(mConvertBuffer, &readbuffer, length, inchannels, outchannels, speakermode);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = convert(*outbuffer, readbuffer, mDescription.mFormat, FMOD_SOUND_FORMAT_PCMFLOAT, *length * *outchannels, 1, 1, 1.0f);
    if (result != FMOD_OK)
    {
        return result;
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
// VERIFY: 0e6ea6fb auto
// OFFSET: 0x00929CB0
FMOD_RESULT DSPSoundCard::release(bool freethis)
{
    if (mConvertBufferMemory)
    {
        FMOD_Memory_Free(mConvertBufferMemory);
        mConvertBuffer       = 0;
        mConvertBufferMemory = 0;
    }

    return DSPFilter::release(freethis);
}

}

#endif
