#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_DSPCODEC

#include "fmod_channel_software.h"
#ifdef FMOD_SUPPORT_MPEG
#include "fmod_codec_mpeg.h"
#endif
#ifdef FMOD_SUPPORT_XMA
#include "fmod_codec_xma.h"
#endif
#ifdef FMOD_SUPPORT_IMAADPCM
#include "fmod_codec_wav_imaadpcm.h"
#endif
#include "fmod_codeci.h"
#include "fmod_dspi.h"
#include "fmod_dsp_codec.h"
#include "fmod_dsp_codecpool.h"
#include "fmod_pluginfactory.h"
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

// VERIFY: 977542f8 manual
// OFFSET: 0x008E86B0
FMOD_RESULT DSPCodecPool::init(FMOD_DSP_CATEGORY category, int resamplerpcmblocksize, int numdspcodecs)
{
    int count;
    FMOD_DSP_DESCRIPTION_EX descriptionex;

    if (!mSystem->mSoftware)
    {
        return FMOD_ERR_NEEDSSOFTWARE;
    }

    if (numdspcodecs >= FMOD_DSP_CODECPOOL_MAXCODECS)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    mNumDSPCodecs = numdspcodecs;

    mPool = (DSPCodec **)FMOD_Memory_Calloc(sizeof(DSPCodec *) * mNumDSPCodecs);
    if (!mPool)
    {
        return FMOD_ERR_MEMORY;
    }

    FMOD_memcpy(&descriptionex, DSPCodec::getDescriptionEx(), sizeof(FMOD_DSP_DESCRIPTION_EX));

    for (count = 0; count < mNumDSPCodecs; count++)
    {
        FMOD_RESULT           result;
        DSPCodec              *dspcodec;

        descriptionex.channels = 2;                       // Maximum possible stereo? This is for memory allocation.
        descriptionex.mFormat = FMOD_SOUND_FORMAT_PCM16;
        descriptionex.mCategory = category;
        descriptionex.mResamplerBlockLength = resamplerpcmblocksize;

        if (0)
        {
        }
        #ifdef FMOD_SUPPORT_MPEG
        else if (category == FMOD_DSP_CATEGORY_DSPCODECMPEG)
        {
            descriptionex.mSize = sizeof(DSPCodecMPEG);
        }
        #endif
        #ifdef FMOD_SUPPORT_XMA
        else if (category == FMOD_DSP_CATEGORY_DSPCODECXMA)
        {
            descriptionex.mSize = sizeof(DSPCodecXMA);
        }
        #endif
        #ifdef FMOD_SUPPORT_IMAADPCM
        else if (category == FMOD_DSP_CATEGORY_DSPCODECADPCM)
        {
            descriptionex.mSize = sizeof(DSPCodecADPCM);
        }
        #endif
        #ifdef FMOD_SUPPORT_RAW
        else if (category == FMOD_DSP_CATEGORY_DSPCODECRAW)
        {
            descriptionex.mResamplerBlockLength = 256;
            descriptionex.mSize = sizeof(DSPCodecRaw);
        }
        #endif
        else
        {
            return FMOD_ERR_FORMAT;
        }

        result = mSystem->createDSP(&descriptionex, (DSPI **)&dspcodec, true);
        if (result != FMOD_OK)
        {
            return result;
        }

        dspcodec->mFlags = 0;

        if (0)
        {
        }
        #ifdef FMOD_SUPPORT_MPEG
        else if (category == FMOD_DSP_CATEGORY_DSPCODECMPEG)
        {
            DSPCodecMPEG *dspcodecmpeg = SAFE_CAST(DSPCodecMPEG, dspcodec);
            CodecMPEG    *mpeg = &dspcodecmpeg->mCodecMemory;
            FMOD_CODEC_DESCRIPTION_EX *desc;

            result = mSystem->mPluginFactory->getCodec(mSystem->mMPEGPluginHandle, &desc);
            if (result != FMOD_OK)
            {
                return result;
            }

            FMOD_memcpy(&mpeg->mDescription, desc, sizeof(FMOD_CODEC_DESCRIPTION_EX));
            mpeg->mDescription.getwaveformat = &Codec::defaultGetWaveFormat;

            dspcodec->mCodec = mpeg;
            dspcodec->mWaveFormat.format = FMOD_SOUND_FORMAT_MPEG;

            mpeg->mMemoryBlock = &dspcodecmpeg->mCodecMemoryBlock;
        }
        #endif
        #ifdef FMOD_SUPPORT_IMAADPCM
        else if (category == FMOD_DSP_CATEGORY_DSPCODECADPCM)
        {
            DSPCodecADPCM *dspcodecadpcm = SAFE_CAST(DSPCodecADPCM, dspcodec);
            CodecWav      *wav = &dspcodecadpcm->mCodecMemory;
            FMOD_CODEC_DESCRIPTION_EX *desc;

            result = mSystem->mPluginFactory->getCodec(mSystem->mWAVPluginHandle, &desc);
            if (result != FMOD_OK)
            {
                return result;
            }

            FMOD_memcpy(&wav->mDescription, desc, sizeof(FMOD_CODEC_DESCRIPTION_EX));
            wav->mDescription.getwaveformat = &Codec::defaultGetWaveFormat;

            dspcodec->mCodec = wav;
            dspcodec->mWaveFormat.format = FMOD_SOUND_FORMAT_IMAADPCM;
        }
        #endif
        #ifdef FMOD_SUPPORT_RAW
        else
        {
            DSPCodecRaw *dspcodecraw = SAFE_CAST(DSPCodecRaw, dspcodec);
            CodecRaw    *raw = &dspcodecraw->mCodecMemory;

            FMOD_memcpy(&raw->mDescription, CodecRaw::getDescriptionEx(), sizeof(FMOD_CODEC_DESCRIPTION_EX));
            raw->mDescription.getwaveformat = &Codec::defaultGetWaveFormat;

            dspcodec->mCodec = raw;
            dspcodec->mWaveFormat.format = FMOD_SOUND_FORMAT_PCM16;
        }
        #endif

        dspcodec->mCodec->mFile           = &dspcodec->mMemoryFile;
        dspcodec->mCodec->waveformat      = &dspcodec->mWaveFormat;
        dspcodec->mCodec->mSrcDataOffset  = 0;                          /* Raw data is going to be placed right at the start. */
        dspcodec->mCodec->mFlags         |= FMOD_CODEC_ACCURATELENGTH;
        dspcodec->mPool                   = this;
        dspcodec->mPoolIndex              = count;

        dspcodec->setFinished(true, true);                              /* Start off finished so that it can be allocated. */

        mPool[count] = dspcodec;
        mAllocated[count] = false;
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
// VERIFY: ec0072cf auto
// OFFSET: 0x008E8510
FMOD_RESULT DSPCodecPool::close()
{
    if (mPool)
    {
        int count;

        for (count = 0; count < mNumDSPCodecs; count++)
        {
            DSPCodec *dspcodec = mPool[count];

            dspcodec->mCodec->mFile = 0;             /* Stop it trying to free our static memory file. */
            dspcodec->mCodec->mReadBuffer = 0;       /* Stop it trying to free the global read buffer multiple times if it happens to be pointing to it. */
            dspcodec->mCodec->mWaveFormatMemory = 0; /* Stop it trying to free the sound's wave format if it happens to be pointing to it. */

            if (dspcodec->mCodec->mPCMBufferMemory)
            {
                FMOD_Memory_Free(dspcodec->mCodec->mPCMBufferMemory);
            }

            mPool[count]->release();
        }

        FMOD_Memory_Free(mPool);
        mPool = 0;
    }

    if (mReadBuffer)
    {
        FMOD_Memory_Free(mReadBuffer);
        mReadBuffer = 0;
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
// VERIFY: d6c6ec42 manual
// OFFSET: 0x008E85F0
FMOD_RESULT DSPCodecPool::alloc(DSPCodec **dspcodec)
{
    int count;

    for (count = 0; count < mNumDSPCodecs; count++)
    {
        bool finished;
        
        mPool[count]->getFinished(&finished);

        if (!mAllocated[count] && finished)
        {
            mAllocated[count] = true;    
            *dspcodec = mPool[count];
            return FMOD_OK;
        }
    }

    return FMOD_ERR_CHANNEL_ALLOC;
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
// VERIFY: 8cca9a7c manual
// OFFSET: 0x008E8660
FMOD_RESULT DSPCodecPool::areAnyFree()
{
    int count;

    for (count = 0; count < mNumDSPCodecs; count++)
    {
        bool finished;

        mPool[count]->getFinished(&finished);

        if (!mAllocated[count] && finished)
        {
            return FMOD_OK;
        }
    }

    return FMOD_ERR_CHANNEL_ALLOC;
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
