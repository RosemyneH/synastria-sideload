#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_channel_software.h"
#include "fmod_channelpool.h"
#include "fmod_dsp_resampler.h"
#include "fmod_memory.h"
#include "fmod_output_software.h"
#include "fmod_sample_software.h"
#include "fmod_string.h"
#include "fmod_systemi.h"

namespace FMOD
{

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
// VERIFY: fae0110e manual
// OFFSET: 0x0091AA80
OutputSoftware::OutputSoftware()
{
    FMOD_memset(&mDescription, 0, sizeof(FMOD_OUTPUT_DESCRIPTION_EX));

    mDescription.name    = "FMOD Software Output";
    mDescription.version = 0x00010100;
    mDescription.polling = false;

    /*
        Private members
    */
    mDescription.getsamplemaxchannels = &OutputSoftware::getSampleMaxChannelsCallback;
    mDescription.mType                = FMOD_OUTPUTTYPE_SOFTWARE;
    mDescription.mSize                = sizeof(OutputSoftware);

    mChannel = 0;
    mChannelPool = 0;
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
// VERIFY: 02e9062c manual
// OFFSET: 0x0091A930
FMOD_RESULT OutputSoftware::init(int maxchannels)
{
    FMOD_RESULT     result;

    if (!mSystem)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    /*
        Create Software channels
    */
    if (maxchannels)
    {
        int count;

        mChannelPool = mChannelPool3D = FMOD_Object_Alloc(ChannelPool);
        if (!mChannelPool)
        {
            return FMOD_ERR_MEMORY;
        }

        result = mChannelPool->init(mSystem, this, maxchannels);
        if (result != FMOD_OK)
        {
            return result;
        }

        mChannel = (ChannelSoftware *)FMOD_Memory_Calloc(sizeof(ChannelSoftware) * maxchannels);
        if (!mChannel)
        {
            return FMOD_ERR_MEMORY;
        }

        for (count = 0; count < maxchannels; count++)
        {
            new (&mChannel[count]) ChannelSoftware;

            mChannelPool->setChannel(count, &mChannel[count], mSystem->mChannelGroup->mDSPHead);
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
// VERIFY: 268d5542 auto
// OFFSET: 0x0091AA20
FMOD_RESULT OutputSoftware::release()
{
    if (mChannelPool)
    {    
        mChannelPool->release();
        mChannelPool = 0;
    }
    if (mChannel)
    {
        FMOD_Memory_Free(mChannel);
        mChannel = 0;
    }

    return Output::release();
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
// VERIFY: ad9c6d57 manual
// OFFSET: 0x0091AB00
FMOD_RESULT OutputSoftware::createSample(FMOD_MODE mode, FMOD_CODEC_WAVEFORMAT *waveformat, Sample **sample)
{
    FMOD_RESULT      result;
	int              bits = 0;
    SampleSoftware  *newsample;
    unsigned int     overflowbytes = 0;
    bool             allocated = false;

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputSoftware::createSample", "lengthpcm %d, lengthbytes %d, channels %d, format %d, mode %08x\n", waveformat ? waveformat->lengthpcm : 0, waveformat ? waveformat->lengthbytes : 0, waveformat ? waveformat->channels : 0, waveformat ? waveformat->format : FMOD_SOUND_FORMAT_NONE, mode));

    if (!sample)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (waveformat)
    {
        result = SoundI::getBitsFromFormat(waveformat->format, &bits);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (!bits && waveformat->format != FMOD_SOUND_FORMAT_NONE
            #ifdef FMOD_SUPPORT_DSPCODEC
                #ifdef FMOD_SUPPORT_IMAADPCM
                && waveformat->format != FMOD_SOUND_FORMAT_IMAADPCM
                #endif
                #ifdef FMOD_SUPPORT_XMA
                && waveformat->format != FMOD_SOUND_FORMAT_XMA
                #endif
                #ifdef FMOD_SUPPORT_MPEG
                && waveformat->format != FMOD_SOUND_FORMAT_MPEG
                #endif
            #endif
        )
        {
            return FMOD_ERR_FORMAT;
        }
    }

    if (*sample == 0)
    {
        newsample = FMOD_Object_Calloc(SampleSoftware);
        if (!newsample)
        {
            return FMOD_ERR_MEMORY;
        }
        allocated = true;
    }
    else
    {
        newsample = SAFE_CAST(SampleSoftware, *sample);
    }

    if (!waveformat)
    {
        *sample = newsample;
        return FMOD_OK;
    }

    newsample->mFormat = waveformat->format;

    if (waveformat->format == FMOD_SOUND_FORMAT_IMAADPCM || waveformat->format == FMOD_SOUND_FORMAT_XMA || waveformat->format == FMOD_SOUND_FORMAT_MPEG)
    {
        newsample->mLengthBytes = waveformat->lengthbytes;
        newsample->mLoopPointDataEnd = 0;
        overflowbytes = 0;
    }
    else
    {
        result = SoundI::getBytesFromSamples(waveformat->lengthpcm, &newsample->mLengthBytes, waveformat->channels, waveformat->format);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = SoundI::getBytesFromSamples(FMOD_DSP_RESAMPLER_OVERFLOWLENGTH, &overflowbytes, waveformat->channels, waveformat->format);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (overflowbytes <= 8)
        {
            newsample->mLoopPointDataEnd = newsample->mLoopPointDataEndMemory;
        }
        else
        {
            newsample->mLoopPointDataEnd = (char *)FMOD_Memory_Calloc(overflowbytes);
            if (!newsample->mLoopPointDataEnd)
            {
                return FMOD_ERR_MEMORY;
            }
        }
    }

    if (mode & FMOD_OPENMEMORY_POINT)
    {
        newsample->mBufferMemory = 0;
        newsample->mBuffer = 0;
    }
    else
    {
        newsample->mBufferMemory = FMOD_Memory_Calloc(newsample->mLengthBytes + (overflowbytes * 2) + 16);
        if (!newsample->mBufferMemory)
        {
            if (allocated)
            {
                FMOD_Memory_Free(newsample);
            }
            return FMOD_ERR_MEMORY;
        }

        newsample->mBuffer = (char *)FMOD_ALIGNPOINTER((FMOD_UINT_NATIVE)newsample->mBufferMemory + overflowbytes, 16);
    }

    newsample->mFormat = waveformat->format;
    newsample->mLength = waveformat->lengthpcm;

    *sample = newsample;

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputSoftware::createSample", "done\n"));

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
// INLINED
int OutputSoftware::getSampleMaxChannels(FMOD_MODE mode, FMOD_SOUND_FORMAT format)
{
    return 16;       /* 2D and 3D voices can be panned without needing to split them up.  Allow up to 16 channel wide. */
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
// VERIFY: 31f8b339 auto
// OFFSET: 0x0091AA70
int F_CALLBACK OutputSoftware::getSampleMaxChannelsCallback(FMOD_OUTPUT_STATE *output, FMOD_MODE mode, FMOD_SOUND_FORMAT format)
{
    OutputSoftware *outputsoftware = (OutputSoftware *)output;

    return outputsoftware->getSampleMaxChannels(mode, format);
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


}

#endif
