#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_3d.h"
#include "fmod_channel_software.h"
#include "fmod_codeci.h"
#ifdef FMOD_SUPPORT_FSB
#include "fmod_codec_fsb.h"
#endif
#ifdef FMOD_SUPPORT_MPEG
#include "fmod_codec_mpeg.h"
#endif
#if defined(FMOD_SUPPORT_WAV) && defined(FMOD_SUPPORT_IMAADPCM)
#include "fmod_codec_wav.h"
#include "fmod_codec_wav_imaadpcm.h"
#endif
#if defined(FMOD_SUPPORT_RAW) && defined(FMOD_SUPPORT_IMAADPCM)
#include "fmod_codec_raw.h"
#endif
#include "fmod_dsp_codec.h"
#include "fmod_dsp_fft.h"
#include "fmod_dsp_resampler.h"
#include "fmod_dsp_wavetable.h"
#include "fmod_dspi.h"
#include "fmod_sample_software.h"
#include "fmod_soundgroupi.h"
#include "fmod_speakermap.h"
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
// VERIFY: bf651e02 manual
// OFFSET: 0x00933EB0
ChannelSoftware::ChannelSoftware()
{
    mDSPWaveTable     = 0;
    mDSPHead          = 0;
    mDSPResampler     = 0;
    mDSPCodec         = 0;
    mDSPCodecStopped  = 0;
    mDSPLowPass       = 0;
    mDSP              = 0;
    mDSPConnection    = 0;
    mDSPReverb        = 0;
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
// VERIFY: 0661c4ba auto
// OFFSET: 0x00934CD0
FMOD_RESULT ChannelSoftware::init(int index, SystemI *system, Output *output, DSPI *dspmixtarget)
{
    FMOD_RESULT             result;
    FMOD_DSP_DESCRIPTION_EX descriptionex;

    ChannelReal::init(index, system, output, dspmixtarget);

    /*
        Create a head unit that things can connect to.
    */
    FMOD_memset(&descriptionex, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));
    FMOD_strcpy(descriptionex.name, "FMOD Channel DSPHead Unit");
    descriptionex.version   = 0x00010100;
    descriptionex.mCategory = FMOD_DSP_CATEGORY_FILTER;
    descriptionex.mFormat   = FMOD_SOUND_FORMAT_PCMFLOAT;

    mDSPHead = &mDSPHeadMemory;

    result = mSystem->createDSP(&descriptionex, &mDSPHead, false);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        Create a lowpass unit
    */
    if (mSystem->mFlags & FMOD_INIT_SOFTWARE_OCCLUSION || mSystem->mFlags & FMOD_INIT_SOFTWARE_HRTF)
    {
        result = mSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS_SIMPLE, &mDSPLowPass);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    /*
        Create a wave table unit
    */
    FMOD_memset(&descriptionex, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));
    FMOD_strcpy(descriptionex.name, "FMOD WaveTable Unit");
    descriptionex.version       = 0x00010100;
    descriptionex.channels      = dspmixtarget->mDescription.channels;
    descriptionex.read          = 0; /* DSPWavetable uses DSPWaveTable::execute not a read callback. */

    descriptionex.setparameter  = DSPWaveTable::setParameterCallback;
    descriptionex.getparameter  = DSPWaveTable::getParameterCallback;
    descriptionex.setposition   = DSPWaveTable::setPositionCallback;

    descriptionex.mCategory     = FMOD_DSP_CATEGORY_WAVETABLE;
    descriptionex.mFormat       = dspmixtarget->mDescription.mFormat;

    mDSPWaveTable = &mDSPWaveTableMemory;

    result = mSystem->createDSP(&descriptionex, (DSPI **)&mDSPWaveTable, false);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mDSPWaveTable->setUserData((void **)this);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mDSPWaveTable->setTargetFrequency((int)dspmixtarget->mDefaultFrequency);
    if (result != FMOD_OK)
    {
        return result;
    }

    mMinFrequency = -mMaxFrequency;

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
// VERIFY: 5267f156 manual
// OFFSET: 0x009328B0
FMOD_RESULT ChannelSoftware::close()
{
    FMOD_RESULT result;

    result = ChannelReal::close();
    if (result != FMOD_OK)
    {
        return result;
    }

    if (mDSPWaveTable)
    {
        mDSPWaveTable->release(false);  /* false = dont free this, as it is not alloced. */
        mDSPWaveTable = 0;
    }

    if (mDSPHead)
    {
        mDSPHead->release(false);       /* false = dont free this, as it is not alloced. */
        mDSPHead= 0;
    }

    if (mDSPResampler)
    {
        mDSPResampler->release();
        mDSPResampler = 0;
    }

    if (mDSPLowPass)
    {
        mDSPLowPass->release();
        mDSPLowPass = 0;
    }

    mDSPCodec        = 0;  /* Don't free, it points to a pool codec. */
    mDSPCodecStopped = 0;

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
// VERIFY: 5319d4c8 manual
// OFFSET: 0x00932940
FMOD_RESULT ChannelSoftware::setupDSPCodec(DSPI *dsp)
{
#ifdef FMOD_SUPPORT_DSPCODEC
    FMOD_RESULT result;
    DSPCodec       *dspcodec;
    Codec          *codec, *soundcodec;
    SampleSoftware *sample;

    sample     = SAFE_CAST(SampleSoftware, mSound);
    dspcodec   = SAFE_CAST(DSPCodec, dsp);
    codec      = SAFE_CAST(Codec, dspcodec->mCodec);
    soundcodec = sample->mCodec ? sample->mCodec : sample->mSubSoundParent->mCodec;

    if (!soundcodec)
    {
        return FMOD_ERR_INTERNAL;
    }

    codec->mPCMBufferLength      = soundcodec->mPCMBufferLength;
    codec->mPCMBufferLengthBytes = soundcodec->mPCMBufferLength * sizeof(short) * dsp->mDescription.channels;

    if (!codec->waveformat)
    {
        return FMOD_ERR_INTERNAL;
    }

    soundcodec->mDescription.getwaveformat(soundcodec, sample->mSubSoundIndex, codec->waveformat);

    codec->mFlags = soundcodec->mFlags;
    dspcodec->mNewPosition = 0xFFFFFFFF;
    dspcodec->mCodec->mFile = &dspcodec->mMemoryFile;

    dspcodec->mMemoryFile.init(sample->mLengthBytes, 0);
    dspcodec->mMemoryFile.mMem = sample->mBuffer;

    /*
        Copy codec specific things from the source sound to the dspcodec used in this channel.
    */
    if (0)
    {
    }
    #if defined(FMOD_SUPPORT_WAV) && defined(FMOD_SUPPORT_IMAADPCM)
    else if (sample->mType == FMOD_SOUND_TYPE_WAV && sample->mFormat == FMOD_SOUND_FORMAT_IMAADPCM)
    {
        CodecWav *srcwav  = (CodecWav *)soundcodec;
        CodecWav *destwav = (CodecWav *)codec;

        destwav->mSamplesPerADPCMBlock = srcwav->mSamplesPerADPCMBlock;
        destwav->mReadBufferLength     = srcwav->mReadBufferLength;
    }
    #endif
    #if defined(FMOD_SUPPORT_WAV) && defined(FMOD_SUPPORT_RAW) && defined(FMOD_SUPPORT_IMAADPCM)
    else if (sample->mType == FMOD_SOUND_TYPE_RAW && sample->mFormat == FMOD_SOUND_FORMAT_IMAADPCM)
    {
        CodecRaw *srcwav  = (CodecRaw *)soundcodec;
        CodecWav *destwav = (CodecWav *)codec;

        destwav->mSamplesPerADPCMBlock = srcwav->mSamplesPerADPCMBlock;
        destwav->mReadBufferLength     = srcwav->mReadBufferLength;
    }
    #endif
    #if defined(FMOD_SUPPORT_FSB)
    else if (sample->mType == FMOD_SOUND_TYPE_FSB)
    {
        CodecFSB  *fsb = (CodecFSB *)soundcodec;

        if (0)
        {
        }
        #if defined(FMOD_SUPPORT_MPEG)
        else if (sample->mFormat == FMOD_SOUND_FORMAT_MPEG)
        {
            CodecMPEG *destmpeg = (CodecMPEG *)codec;

            destmpeg->mPCMFrameLengthBytes = codec->waveformat[0].channels * 2304;
        }
        #endif
        #if defined(FMOD_SUPPORT_IMAADPCM)
        else if (sample->mFormat == FMOD_SOUND_FORMAT_IMAADPCM)
        {
            CodecWav *srcwav  = fsb->mADPCM;
            CodecWav *destwav = (CodecWav *)codec;

            destwav->mSamplesPerADPCMBlock = srcwav->mSamplesPerADPCMBlock;
            destwav->mReadBufferLength     = codec->waveformat[0].channels * 36;
        }
        #endif

        fsb->mDescription.getwaveformat(fsb, sample->mSubSoundIndex, codec->waveformat);
    }
    #endif
    #if defined(FMOD_SUPPORT_MPEG)
    else if (sample->mFormat == FMOD_SOUND_FORMAT_MPEG)
    {
        CodecMPEG *srcmpeg  = (CodecMPEG *)soundcodec;
        CodecMPEG *destmpeg = (CodecMPEG *)codec;

        destmpeg->mPCMFrameLengthBytes = srcmpeg->mPCMFrameLengthBytes;
    }
    #endif

    dsp->mDescription.channels = sample->mChannels;

    result = dsp->setTargetFrequency((int)mParent->mChannelGroup->mDSPHead->mDefaultFrequency);
    if (result != FMOD_OK)
    {
        return result;
    }

    mDSPCodec                   = dspcodec;
    mMinFrequency               = 0;
    mDSPCodec->mLength          = mSound->mLength;
    mDSPCodec->mLoopStart       = mLoopStart;
    mDSPCodec->mLoopLength      = mLoopLength;
    mDSPCodec->mLoopCount       = mLoopCount;
    mDSPCodec->mMode            = mMode;
    mDSPCodec->mDefaultFrequency = mSound->mDefaultFrequency;
#endif

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
// VERIFY: 28a8460f manual
// OFFSET: 0x00933F40
FMOD_RESULT ChannelSoftware::alloc()
{
    FMOD_RESULT result;

    result = ChannelReal::alloc();
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        Standard PCM *wavetable* playback.
    */
    if (!(mMode & FMOD_CREATECOMPRESSEDSAMPLE))
    {
        DSPWaveTable *dspwave = SAFE_CAST(DSPWaveTable, mDSPWaveTable);
        if (!dspwave)
        {
            return FMOD_ERR_INTERNAL;
        }

        mDSPCodec        = 0;
        mDSPCodecStopped = 0;

        /*
            Disconnect existing nodes first.
        */
        result = mDSPHead->disconnectFrom(0);
        if (result != FMOD_OK)
        {
            return result;
        }
        if (mDSPLowPass)
        {
            result = mDSPLowPass->disconnectFrom(0);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        result = mDSPWaveTable->disconnectFrom(0);
        if (result != FMOD_OK)
        {
            return result;
        }

        /*
            Add to parent's channelgroup along with any lowpass.
        */
        if (mDSPLowPass)
        {
            result = mParent->mChannelGroup->mDSPHead->addInputQueued(mDSPHead, false, 0, &mDSPConnection);
            if (result != FMOD_OK)
            {
                return result;
            }
            result = mDSPHead->addInputQueued(mDSPLowPass, false, 0, 0);
            if (result != FMOD_OK)
            {
                return result;
            }
            result = mDSPLowPass->addInputQueued(mDSPWaveTable, false, 0, 0);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        else
        {
            result = mParent->mChannelGroup->mDSPHead->addInputQueued(mDSPHead, false, 0, &mDSPConnection);
            if (result != FMOD_OK)
            {
                return result;
            }

            result = mDSPHead->addInputQueued(mDSPWaveTable, false, 0, 0);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        if (!(mFlags & CHANNELREAL_FLAG_NOREVERB))
        {
            mDSPReverb = mDSPWaveTable;

            result = addToReverbs(mDSPReverb);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        result = setLoopPoints(mSound->mLoopStart, mSound->mLoopLength);
        if (result != FMOD_OK)
        {
            return result;
        }

        mMinFrequency = -mMaxFrequency;

        dspwave->mChannel           = this;
        dspwave->mSound             = mSound;
        dspwave->mPosition.mHi      = 0;
        dspwave->mPosition.mLo      = 0;
        dspwave->mDirection         = DSPWAVETABLE_SPEEDDIR_FORWARDS;

        mDSPHead->setFinished(false, false);
        mDSPHead->setActive(false);
        if (mDSPLowPass)
        {
            mDSPLowPass->setFinished(false, false);
            mDSPLowPass->setActive(false);
        }
        mDSPWaveTable->setFinished(false, false);
        mDSPWaveTable->setActive(false);
    }
    else
    /*
        Advanced - Compressed codec playback with external resampler unit.
    */
    {
#ifdef FMOD_SUPPORT_DSPCODEC
        DSPCodec *dsp;
        bool      finished = true;

        /*
            Wait for the mixer to let go of the codec that was stopped last.
        */
        if (mDSPCodecStopped)
        {
            mDSPCodecStopped->getFinished(&finished);
            if (!finished)
            {
                mSystem->flushDSPConnectionRequests(true);
            }
            else
            {
                mDSPCodecStopped = 0;
            }
        }

        /*
            Disconnect existing nodes first.
        */
        result = mDSPHead->disconnectFrom(0);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (mDSPLowPass)
        {
            result = mDSPLowPass->disconnectFrom(0);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        if (mDSPWaveTable)
        {
            result = mDSPWaveTable->disconnectFrom(0);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        /*
            Grab a codec from the pool based on format.
        */
        result = mSystem->allocDSPCodec(mSound->mFormat, &dsp);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (!finished)
        {
            FMOD_OS_CriticalSection_Enter(mSystem->mDSPCrit);
        }

        result = setupDSPCodec(dsp);

        if (!finished)
        {
            FMOD_OS_CriticalSection_Leave(mSystem->mDSPCrit);
        }

        if (result != FMOD_OK)
        {
            return result;
        }

        if (mDSPLowPass)
        {
            result = mParent->mChannelGroup->mDSPHead->addInputQueued(mDSPHead, false, 0, &mDSPConnection);
            if (result != FMOD_OK)
            {
                return result;
            }
            result = mDSPHead->addInputQueued(mDSPLowPass, false, 0, 0);
            if (result != FMOD_OK)
            {
                return result;
            }
            result = mDSPLowPass->addInputQueued(dsp, false, 0, 0);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        else
        {
            result = mParent->mChannelGroup->mDSPHead->addInputQueued(mDSPHead, false, 0, &mDSPConnection);
            if (result != FMOD_OK)
            {
                return result;
            }
            result = mDSPHead->addInputQueued(dsp, false, 0, 0);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        if (!(mFlags & CHANNELREAL_FLAG_NOREVERB))
        {
            mDSPReverb = dsp;

            result = addToReverbs(mDSPReverb);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        mDSPHead->setFinished(false, false);
        mDSPHead->setActive(false);

        dsp->setFinished(false, false);
        dsp->setActive(true);
#else
        return FMOD_ERR_INTERNAL;
#endif
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
// VERIFY: 0e57346a auto
// OFFSET: 0x00934300
FMOD_RESULT ChannelSoftware::alloc(DSPI *dsp)
{
    FMOD_RESULT result;

    result = ChannelReal::alloc();
    if (result != FMOD_OK)
    {
        return result;
    }

    mDSPCodec = 0;

    {
        FMOD_DSP_DESCRIPTION_EX descriptionex;

        FMOD_memset(&descriptionex, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));
        FMOD_strcpy(descriptionex.name, "FMOD Resampler Unit");
        descriptionex.version       = 0x00010100;
        descriptionex.channels      = 0;
        descriptionex.mCategory     = FMOD_DSP_CATEGORY_RESAMPLER;

        result = mSystem->createDSP(&descriptionex, (DSPI **)&mDSPResampler, true);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = mDSPResampler->setUserData((void **)this);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = mDSPResampler->setTargetFrequency((int)mParent->mChannelGroup->mDSPHead->mDefaultFrequency);
        if (result != FMOD_OK)
        {
            return result;
        }

        mMinFrequency = 0;
    }


    /*
        Connect the mixer to the resampler and the resampler to the reader
    */
    result = mDSPHead->disconnectFrom(0);
    if (result != FMOD_OK)
    {
        return result;
    }
    if (mDSPLowPass)
    {
        result = mDSPLowPass->disconnectFrom(0);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    result = mDSPWaveTable->disconnectFrom(0);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mDSPHead->addInputQueued(mDSPResampler, false, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mDSPResampler->addInputQueued(dsp, false, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mParent->mChannelGroup->mDSPHead->addInputQueued(mDSPHead, false, 0, &mDSPConnection);
    if (result != FMOD_OK)
    {
        return result;
    }

    mDSPReverb = mDSPResampler;

    result = addToReverbs(mDSPReverb);
    if (result != FMOD_OK)
    {
        return result;
    }

    mDSPResampler->mLength     = mLength;
    mDSPResampler->mLoopStart  = mLoopStart;
    mDSPResampler->mLoopLength = mLoopLength;
    mDSPResampler->mLoopCount  = mLoopCount;
    mDSPResampler->mMode       = mMode;

    if (!mDSPWaveTable)
    {
        return FMOD_ERR_INTERNAL;
    }

    mDSPWaveTable->mSound = 0;

    mDSPHead->setFinished(false, false);
    mDSPHead->setActive(false);

    mDSPResampler->setFinished(false, false);
    mDSPResampler->setActive(false);

    dsp->setFinished(false, false);
    dsp->setActive(false);

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
// VERIFY: 5c85866b auto
// OFFSET: 0x00932B70
FMOD_RESULT ChannelSoftware::start()
{
    if (!(mFlags & CHANNELREAL_FLAG_PAUSED))
    {
        mDSPHead->setActive(true);
    }

    if (mSound)
    {
        mDSPWaveTable->setActive(true);
    }
    if (mDSPResampler)
    {
        mDSPResampler->setActive(true);
    }
    if (mDSPLowPass)
    {
        mDSPLowPass->setActive(true);
    }
#ifdef FMOD_SUPPORT_DSPCODEC
    if (mDSPCodec)
    {
        mDSPCodec->setActive(true);
    }
#endif
    if (mDSP)
    {
        mDSP->setActive(true);
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
// VERIFY: c2b67d65 auto
// OFFSET: 0x00932BE0
FMOD_RESULT ChannelSoftware::stop(bool ignoreenddelay, bool resetstate, bool force)
{
    FMOD_RESULT result;
    int count;

    if (mFlags & CHANNELREAL_FLAG_ENDDELAY && !ignoreenddelay)
    {
        return FMOD_OK;
    }

    if (mDSPHead)
    {
        mDSPHead->disconnectAll(false, true);
        mDSPHead->setFinished(true, force);
        mDSPHead->setActive(false);
    }

#ifdef FMOD_SUPPORT_DSPCODEC
    if (mDSPCodec)
    {
        mDSPCodec->disconnectAll(false, true);      /* Just outputs, there are no inputs. */
        mDSPCodec->setFinished(true, force);
        mDSPCodec->setActive(false);
        mDSPCodec->freeFromPool();

        mDSPCodecStopped = mDSPCodec;
        mDSPCodec = 0;
    }
#endif

    if (mDSPResampler)
    {
        mDSPResampler->setFinished(true, force);
        mDSPResampler->setActive(false);
        mDSPResampler->release();
        mDSPResampler = 0;
    }

    if (mDSPWaveTable)
    {
        mDSPWaveTable->disconnectAll(false, true);      /* Just outputs, there are no inputs. */
        mDSPWaveTable->setFinished(true, force);
        mDSPWaveTable->setActive(false);
        mDSPWaveTable->mFlags &= ~FMOD_DSP_FLAG_VISITED;
    }

    if (mDSP)
    {
        DSPI *prev;
        int numoutputs;

        result = mDSP->getNumOutputs(&numoutputs);
        if (result != FMOD_OK)
        {
            return result;
        }

        for(count = 0; count < numoutputs; count++)
        {
            result = mDSP->getOutput(count, &prev);
            if (result == FMOD_OK)
            {
                result = prev->disconnectFrom(mDSP);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
        }
    }

    mDSPReverb = 0;

    ChannelReal::stop(ignoreenddelay, true);

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
// VERIFY: 52d72b0e auto
// OFFSET: 0x00933F20
FMOD_RESULT ChannelSoftware::stop(bool ignoreenddelay, bool resetstate)
{
    return stop(ignoreenddelay, resetstate, false);
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
// VERIFY: ba04da45 auto
// OFFSET: 0x00932D70
FMOD_RESULT ChannelSoftware::setPaused(bool paused)
{
    mDSPHead->setActive(!paused);

    if (mDSPReverb)
    {
        mDSPReverb->setActive(!paused);
    }

	return ChannelReal::setPaused(paused);
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
// VERIFY: 6643abb7 manual
// OFFSET: 0x00934520
FMOD_RESULT ChannelSoftware::setReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop)
{
#ifdef FMOD_SUPPORT_REVERB
    FMOD_RESULT result;
    DSPConnectionI *connection;
    float lindirect;

    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    lindirect = FMOD_POW(10.0f, prop->Direct / 2000.0f); // Convert to linear gain

    if (lindirect != mParent->mReverbDryVolume)
    {
        mParent->mReverbDryVolume = lindirect;
        updateDirectMix(mParent->mVolume);
    }

    /*
        Set main reverb's channel properties and update the reverb mix
    */
    result = mSystem->mReverbGlobal.setChanProperties(mParent->mIndex, prop);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (mSystem->mReverbGlobal.mDSP)
    {
        mSystem->mReverbGlobal.getChanProperties(mParent->mIndex, 0, &connection);
        if (!connection)
        {
            if      (mDSPCodec)     mDSPReverb = mDSPCodec;
            else if (mDSPResampler) mDSPReverb = mDSPResampler;
            else                    mDSPReverb = mDSPWaveTable;

            addToReverbs(mDSPReverb);
        }
        result = updateReverbMix(&mSystem->mReverbGlobal, mParent->mVolume);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

#ifdef FMOD_SUPPORT_MULTIREVERB
    /*
        Also for 3D master reverb
    */
    result = mSystem->mReverb3D.setChanProperties(mParent->mIndex, prop);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (mSystem->mReverb3D.mDSP)
    {
        mSystem->mReverb3D.getChanProperties(mParent->mIndex, 0, &connection);
        if (!connection)
        {
            if      (mDSPCodec)     mDSPReverb = mDSPCodec;
            else if (mDSPResampler) mDSPReverb = mDSPResampler;
            else                    mDSPReverb = mDSPWaveTable;

            addToReverbs(mDSPReverb);
        }
        result = updateReverbMix(&mSystem->mReverb3D, mParent->mVolume);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    /*
        Also for physicals in 3D reverb list
    */
    ReverbI *reverb_c;
    for (reverb_c = SAFE_CAST(ReverbI, mSystem->mReverb3DHead.getNext()); reverb_c != &mSystem->mReverb3DHead; reverb_c = SAFE_CAST(ReverbI, reverb_c->getNext()))
    {
        if (reverb_c->getMode() == FMOD_REVERB_PHYSICAL)
        {
            reverb_c->setChanProperties(mParent->mIndex, prop);

            if (reverb_c->mDSP)
            {
                reverb_c->getChanProperties(mParent->mIndex, 0, &connection);
                if (!connection)
                {
                    if      (mDSPCodec)     mDSPReverb = mDSPCodec;
                    else if (mDSPResampler) mDSPReverb = mDSPResampler;
                    else                    mDSPReverb = mDSPWaveTable;

                    addToReverbs(mDSPReverb);
                }
                result = updateReverbMix(reverb_c, mParent->mVolume);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
        }
    }
#endif

    return FMOD_OK;
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
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
// VERIFY: 80b87148 auto
// OFFSET: 0x00932DC0
FMOD_RESULT ChannelSoftware::updateDirectMix(float volume)
{
    float gainlin;

    /*
       Calculate level based on current state
    */
    gainlin  = volume;
    gainlin *= mParent->mFadeVolume;
    gainlin *= mParent->mReverbDryVolume;
    gainlin *= ((1.0f - mParent->m3DPanLevel) + (mParent->mVolume3D * mParent->m3DPanLevel));
    gainlin *= mParent->mChannelGroup->mRealVolume;
    gainlin *= ((1.0f - mParent->m3DPanLevel) + (mParent->mConeVolume3D * mParent->m3DPanLevel));
    if (mSound && mSound->mSoundGroup)
    {
        gainlin *= mSound->mSoundGroup->mVolume;
    }

    float transmission = mParent->mChannelGroup->mRealDirectOcclusionVolume * mParent->mDirectOcclusionVolume;

    if (mDSPLowPass)
    {
        float hrtf_cutoff = 22050.0f;

        if (mSystem->mFlags & FMOD_INIT_SOFTWARE_HRTF)
        {
            float hrtf_min_angle = mSystem->mAdvancedSettings.HRTFMinAngle / 2.0f;
            float hrtf_max_angle = mSystem->mAdvancedSettings.HRTFMaxAngle / 2.0f;
            float abs_angle = (mAngleToListener <= 180.0f) ? mAngleToListener : 360.0f-mAngleToListener;

            if (abs_angle <= hrtf_min_angle)
            {
                hrtf_cutoff = 22050.0f;        /* Max cutoff frequency - i.e. no filter. */
            }
            else if (abs_angle >= hrtf_max_angle)
            {
                hrtf_cutoff = (float)mSystem->mAdvancedSettings.HRTFFreq;        /* Min cutoff frequency - i.e. maximum filter */
            }
            else
            {
                /*
                    Linear interpolation between min and max frequency with linear angle
                */
                float filter_strength = (abs_angle - hrtf_min_angle) / (hrtf_max_angle - hrtf_min_angle);
                float range = 22050.0f - mSystem->mAdvancedSettings.HRTFFreq;

                hrtf_cutoff = mSystem->mAdvancedSettings.HRTFFreq + range * (1.0f - filter_strength);
            }
        }

        hrtf_cutoff = (((1.0f - mParent->m3DPanLevel) * 22050.0f) + (hrtf_cutoff * mParent->m3DPanLevel));

        if (transmission < 1.0f || hrtf_cutoff < 22050.0f)
        {
            float cutoff;

            mDSPLowPass->setBypass(false);

            cutoff = 22050.0f * transmission * transmission;

            /*
                Try to override the occlusion
            */
            if (hrtf_cutoff < cutoff)
            {
                cutoff = hrtf_cutoff;
            }

            mDSPLowPass->setParameter(FMOD_DSP_LOWPASS_SIMPLE_CUTOFF, cutoff);
        }
        else
        {
            mDSPLowPass->setBypass(true);
        }
    }
    else
    {
        gainlin *= transmission;
    }

    return mDSPConnection->setMix(gainlin);
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
// VERIFY: e62e45e9 manual
// OFFSET: 0x00932F90
FMOD_RESULT ChannelSoftware::updateReverbMix(ReverbI* reverb, float volume)
{
#ifdef FMOD_SUPPORT_REVERB
    FMOD_RESULT result;
    FMOD_REVERB_CHANNELPROPERTIES props;
    DSPConnectionI *connection;
    float gainlin = 0.0f;
    bool morphing = false;

    if (!reverb)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!reverb->mDSP)
    {
        return FMOD_OK;
    }

    /*
        Get properties
    */
    result = reverb->getChanProperties(mParent->mIndex, &props, &connection);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (!connection)
    {
       return FMOD_OK;
    }

    result = mSystem->get3DReverbActive(&morphing);
    if (result != FMOD_OK)
    {
        return result;
    }

    float transmission = (1.0f - mParent->mReverbOcclusion) * mParent->mChannelGroup->mRealReverbOcclusionVolume;

    /*
        Main reverb
    */
    if (reverb == &mSystem->mReverbGlobal)
    {
        if (!(mMode & FMOD_3D))
        {
            gainlin  = (float)FMOD_POW(10.0f, props.Room / 2000.0f);
            gainlin *= mParent->mChannelGroup->mRealVolume;

            return connection->setMix(gainlin * volume);
        }

        if (morphing)
        {
            return connection->setMix(0.0f);
        }

        gainlin  = (float)FMOD_POW(10.0f, props.Room / 2000.0f); // Convert to linear gain
        gainlin *= transmission;
        gainlin *= mParent->mFadeVolume;
        gainlin *= mParent->mVolume3D;
        gainlin *= mParent->mChannelGroup->mRealVolume;
    }
    /*
        Main 3D reverb
    */
    else if (reverb == &mSystem->mReverb3D)
    {
        if (!(mMode & FMOD_3D))
        {
            return connection->setMix(0.0f);
        }

        gainlin  = (float)FMOD_POW(10.0f, props.Room / 2000.0f); // Convert to linear gain
        gainlin *= transmission;
        gainlin *= mParent->mFadeVolume;
        gainlin *= mParent->mVolume3D;
        gainlin *= mParent->mChannelGroup->mRealVolume;
    }
    /*
        Physical 3D reverbs
    */
    else
    {
        if (mMode & FMOD_2D)
        {
            return connection->setMix(0.0f);
        }

        /*
            Find listener and source components
        */
        float source_gain;
        reverb->getPresenceGain(mParent->mIndex, &source_gain);

        /*
            Take the maximum of source presence and listener presence components
        */
        float listener_gain = reverb->getGain();
        gainlin = (listener_gain > source_gain) ? listener_gain : source_gain;

        /*
            Apply other gains
        */
        gainlin *= (float)FMOD_POW(10.0f, props.Room / 2000.0f); /* Convert to linear gain  */
        gainlin *= transmission;
        gainlin *= mParent->mFadeVolume;
        gainlin *= mParent->mVolume3D;
        gainlin *= mParent->mChannelGroup->mRealVolume;
    }

    if (mSound && mSound->mSoundGroup)
    {
        gainlin *= mSound->mSoundGroup->mVolume;
    }

    /*
        Find connection and set channel's reverb input level as appropriate
    */
    return connection->setMix(gainlin * volume);
#else
    return FMOD_OK;
#endif
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
// VERIFY: 2e091aff auto
// OFFSET: 0x00933140
FMOD_RESULT ChannelSoftware::getReverbProperties(FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    if (!prop)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return mSystem->mReverbGlobal.getChanProperties(mParent->mIndex, prop);
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
// VERIFY: 44b6f13c auto
// OFFSET: 0x00933170
FMOD_RESULT ChannelSoftware::set3DOcclusion(float directOcclusion, float reverbOcclusion)
{
    FMOD_RESULT    result;

    if (mSubChannelIndex > 0)
    {
        return FMOD_OK;
    }

    /*
        Modify Room property to be reverbVolume in mB
    */
    mParent->mReverbOcclusion = reverbOcclusion;

    /*
        Set direct occlusion factors
    */
    mParent->mDirectOcclusion       = directOcclusion;
    mParent->mDirectOcclusionVolume = 1.0f - directOcclusion;

    /*
        Update direct and listener reverb mix levels
    */
    result = updateDirectMix(mParent->mVolume);
    if (result != FMOD_OK)
    {
        return result;
    }

#ifdef FMOD_SUPPORT_MULTIREVERB
    ReverbI *reverb_c;
    for (reverb_c = SAFE_CAST(ReverbI, mSystem->mReverb3DHead.getNext()); reverb_c != &mSystem->mReverb3DHead; reverb_c = SAFE_CAST(ReverbI, reverb_c->getNext()))
    {
        if (reverb_c->getMode() == FMOD_REVERB_PHYSICAL)
        {
            result = updateReverbMix(reverb_c, mParent->mVolume);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
    }
#endif

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
// VERIFY: e07dea55 manual
// OFFSET: 0x00933210
FMOD_RESULT ChannelSoftware::setVolume(float volume)
{
    FMOD_RESULT    result;

    if (mSubChannelIndex > 0)
    {
        return FMOD_OK;
    }

    /*
        Update direct and listener reverb mix levels
    */
    result = updateDirectMix(volume);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (!(mFlags & CHANNELREAL_FLAG_NOREVERB))
    {
        result = updateReverbMix(&mSystem->mReverbGlobal, volume);
        if (result != FMOD_OK)
        {
            return result;

        }

        #ifdef FMOD_SUPPORT_MULTIREVERB
        ReverbI *reverb_c;

        result = updateReverbMix(&mSystem->mReverb3D, volume);
        if (result != FMOD_OK)
        {
            return result;
        }

        for (reverb_c = SAFE_CAST(ReverbI, mSystem->mReverb3DHead.getNext());
             reverb_c != &mSystem->mReverb3DHead;
             reverb_c = SAFE_CAST(ReverbI, reverb_c->getNext()))
        {
            if(reverb_c->getMode() == FMOD_REVERB_PHYSICAL)
            {
                result = updateReverbMix(reverb_c, volume);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
        }
        #endif
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
// VERIFY: 3f2b1690 manual
// OFFSET: 0x009332D0
FMOD_RESULT ChannelSoftware::setFrequency(float frequency)
{
    if (mDSPResampler || mDSPCodec)
    {
        DSPResampler *dspresampler;

#ifdef FMOD_SUPPORT_DSPCODEC
        if (mDSPCodec)
        {
            dspresampler = SAFE_CAST(DSPResampler, mDSPCodec);
        }
        else
#endif
        {
            dspresampler = SAFE_CAST(DSPResampler, mDSPResampler);
        }
        if (!dspresampler)
        {
            return FMOD_ERR_INVALID_PARAM;
        }

        frequency *= ((mParent->mPitch3D * mParent->m3DPanLevel) + (1.0f - mParent->m3DPanLevel));
        frequency *= mParent->mChannelGroup->mRealPitch;

        if (frequency > mMaxFrequency)
        {
            frequency = mMaxFrequency;
        }
        if (frequency < mMinFrequency)
        {
            frequency = mMinFrequency;
        }

        return dspresampler->setFrequency(frequency);
    }
    else if (mDSPWaveTable)
    {
        frequency *= ((mParent->mPitch3D * mParent->m3DPanLevel) + (1.0f - mParent->m3DPanLevel));
        frequency *= mParent->mChannelGroup->mRealPitch;

        if (frequency > mMaxFrequency)
        {
            frequency = mMaxFrequency;
        }
        if (frequency < mMinFrequency)
        {
            frequency = mMinFrequency;
        }

        return mDSPWaveTable->setFrequency(frequency);
    }

    return FMOD_ERR_INVALID_PARAM;
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
// VERIFY: 61dee4ae auto
// OFFSET: 0x009333D0
FMOD_RESULT ChannelSoftware::setPan(float lrpan, float fbpan)
{
    int numchannels;
    float pan = (lrpan + 1.0f) / 2.0f;
    float l,r;
    unsigned int channelmask = 0;

    if (mSound)
    {
        numchannels = mSound->mChannels;
        channelmask = mSound->mDefaultChannelMask;
    }
    else if (mDSP)
    {
        numchannels = mDSP->mDescription.channels;
    }
    else
    {
        return FMOD_ERR_INVALID_HANDLE;
    }

    if (numchannels == 1)
    {
        l = FMOD_SQRT(1.0f - pan);
        r = FMOD_SQRT(pan);

        return setSpeakerMix(l, r, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        /*
            Stereo source panning is 0 = 1.0:0.0.  0.5 = 1.0:1.0.  1.0 = 0:1.0.
        */
        if (pan <= 0.5f)
        {
            l = 1.0f;
            r = pan * 2.0f;
        }
        else
        {
            l = (1.0f - pan) * 2.0f;
            r = 1.0f;
        }

        if (numchannels == 2 && !(channelmask & SPEAKER_ALLMONO))
        {
            return setSpeakerMix(l, r, 0, 0, 0, 0, 0, 0);
        }
        else
        {
           return setSpeakerMix(l, r, 1.0f, 1.0f, l, r, l, r);
        }
    }
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
// VERIFY: ce03b657 manual
// OFFSET: 0x009334F0
FMOD_RESULT ChannelSoftware::setSpeakerMix(float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright)
{
    FMOD_RESULT         result;
    int                 numinputlevels;
    float               levels[DSP_MAXLEVELS_OUT * DSP_MAXLEVELS_IN];
    int                 numchannels;
    int                 count, count2;
    FMOD_SPEAKERMAPTYPE speakermap = FMOD_SPEAKERMAPTYPE_DEFAULT;

    if (mSubChannelIndex > 0)
    {
        return FMOD_OK;
    }

    if (mSound)
    {
        numchannels = mSound->mChannels;

        if (mSound->mDefaultChannelMask & SPEAKER_ALLMONO)
        {
            speakermap = FMOD_SPEAKERMAPTYPE_ALLMONO;
        }
        else if (mSound->mDefaultChannelMask & SPEAKER_ALLSTEREO)
        {
            speakermap = FMOD_SPEAKERMAPTYPE_ALLSTEREO;
        }
    }
    else if (mDSP)
    {
        numchannels = mDSP->mDescription.channels;
    }
    else
    {
        return FMOD_ERR_INVALID_HANDLE;
    }

    result = DSPI::calculateSpeakerLevels(frontleft, frontright, center, lfe, backleft, backright, sideleft, sideright, mSystem->mSpeakerMode, numchannels, speakermap, &levels[0], &numinputlevels);
    if (result != FMOD_OK)
	{
        return result;
	}

    if (mParent->mFlags & CHANNELI_FLAG_USEDINPUTMIX)
    {
        for (count = 0; count < mSystem->mMaxOutputChannels; count++)
        {
            for (count2 = 0; count2 < numinputlevels; count2++)
            {
                levels[(count * numinputlevels) + count2] *= mParent->mInputMix[count2];
            }
        }
    }

    return mDSPConnection->setLevels(&levels[0], numinputlevels);
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
// VERIFY: c4cdfe12 manual
// OFFSET: 0x00933640
FMOD_RESULT ChannelSoftware::setSpeakerLevels(int speaker, float *levels, int numlevels)
{
    FMOD_RESULT     result;
    float           clevels[DSP_MAXLEVELS_OUT][DSP_MAXLEVELS_IN];
    int             count;

    if (mSubChannelIndex > 0)
    {
        return FMOD_OK;
    }

    result = mDSPConnection->getLevels(&clevels[0][0], DSP_MAXLEVELS_IN);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < numlevels; count++)
    {
        clevels[speaker][count] = levels[count] * mParent->mInputMix[count];
    }

    return mDSPConnection->setLevels(&clevels[0][0], DSP_MAXLEVELS_IN);
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
// VERIFY: 17c56eb6 manual
// OFFSET: 0x00934760
FMOD_RESULT ChannelSoftware::setPosition(unsigned int position, FMOD_TIMEUNIT postype)
{
    unsigned int      pcm = 0, endpoint;
    int               channels;
    FMOD_SOUND_FORMAT format;
    float             frequency;

    if (mSubChannelIndex > 0)
    {
        return FMOD_OK;
    }

    if (postype != FMOD_TIMEUNIT_MS && postype != FMOD_TIMEUNIT_PCM && postype != FMOD_TIMEUNIT_PCMBYTES)
    {
        return FMOD_ERR_FORMAT;
    }
#ifdef FMOD_SUPPORT_DSPCODEC
    else if (mDSPCodec)
    {
        channels    = mDSPCodec->mDescription.channels;
        format      = FMOD_SOUND_FORMAT_PCMFLOAT;
        frequency   = mDSPCodec->mDefaultFrequency;
    }
#endif
    else if (mSound)
    {
        channels    = mSound->mChannels;
        format      = mSound->mFormat;
        frequency   = mSound->mDefaultFrequency;
    }
    else if (mDSPResampler)
    {
        channels    = mDSPResampler->mDescription.channels;
        format      = FMOD_SOUND_FORMAT_PCMFLOAT;
        frequency   = mDSPResampler->mDefaultFrequency;
    }
    else
    {
        return FMOD_ERR_INVALID_HANDLE;
    }

    if (postype == FMOD_TIMEUNIT_PCM)
    {
        pcm = position;
    }
    else if (postype == FMOD_TIMEUNIT_PCMBYTES)
    {
        SoundI::getSamplesFromBytes(position, &pcm, channels, format);
    }
    else if (postype == FMOD_TIMEUNIT_MS)
    {
        pcm = (unsigned int)((float)position * frequency / 1000.0f);
    }

    if (mSound)
    {
        if (mMode & FMOD_LOOP_OFF)
        {
            endpoint = mSound->mLength - 1;
        }
        else
        {
            endpoint = mLoopStart + mLoopLength - 1;
        }
    }
    else
    {
        endpoint = (unsigned int)-1;
    }

    if (pcm > endpoint)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    /*
        Recurse through channel head downwards calling setposition to all dsp units.
    */
    if (0)
    {
    }
#ifdef FMOD_SUPPORT_DSPCODEC
    else if (mDSPCodec)
    {
        return mDSPCodec->setPosition(pcm, false);
    }
#endif
    else if (mDSPWaveTable)
    {
        return mDSPWaveTable->setPosition(pcm, false);
    }
    else if (mDSPResampler)
    {
        return mDSPResampler->setPosition(pcm, true);
    }

    return mDSPHead->setPosition(pcm, true);  /* Channel based sound is always wavetable, dspcodec, or resampler.  If this is called they must have done a dsp connection and it will flush. */
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
// VERIFY: 3883f8f8 manual
// OFFSET: 0x00934A00
FMOD_RESULT ChannelSoftware::getPosition(unsigned int *position, FMOD_TIMEUNIT postype)
{
    int channels;
    FMOD_SOUND_FORMAT format;
    float frequency;

    if (!position)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (postype != FMOD_TIMEUNIT_MS && postype != FMOD_TIMEUNIT_PCM && postype != FMOD_TIMEUNIT_PCMBYTES)
    {
        return FMOD_ERR_FORMAT;
    }

#ifdef FMOD_SUPPORT_DSPCODEC
    if (mDSPCodec)
    {
        channels    = mDSPCodec->mDescription.channels;
        format      = FMOD_SOUND_FORMAT_PCMFLOAT;
        frequency   = mSound->mDefaultFrequency;

        mPosition = mDSPCodec->DSPResampler::mPosition.mHi;
    }
    else
#endif
    if (mSound)
    {
        if (!mDSPWaveTable)
        {
            return FMOD_ERR_INVALID_PARAM;
        }

        channels    = mSound->mChannels;
        format      = mSound->mFormat;
        frequency   = mSound->mDefaultFrequency;

        mPosition = mDSPWaveTable->mPosition.mHi;
    }
    else if (mDSPResampler)
    {
        channels    = mDSPResampler->mDescription.channels;
        format      = FMOD_SOUND_FORMAT_PCMFLOAT;
        frequency   = mDSPResampler->mDefaultFrequency;
    }
    else
    {
        return FMOD_ERR_INVALID_HANDLE;
    }

    if (postype == FMOD_TIMEUNIT_PCM)
    {
        *position = mPosition;
    }
    else if (postype == FMOD_TIMEUNIT_PCMBYTES)
    {
        SoundI::getBytesFromSamples(mPosition, position, channels, format);
    }
    else
    {
        *position = (unsigned int)((float)mPosition / frequency * 1000.0f);
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
// VERIFY: cba638fe auto
// OFFSET: 0x00933740
FMOD_RESULT ChannelSoftware::setLoopPoints(unsigned int loopstart, unsigned int looplength)
{
    FMOD_RESULT result;

    result = ChannelReal::setLoopPoints(loopstart, looplength);
    if (result != FMOD_OK)
    {
        return result;
    }

#ifdef FMOD_SUPPORT_DSPCODEC
    if (mDSPCodec)
    {
        mDSPCodec->mLoopStart  = mLoopStart;
        mDSPCodec->mLoopLength = mLoopLength;
    }
#endif

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
// VERIFY: 010b68a8 auto
// OFFSET: 0x00933790
FMOD_RESULT ChannelSoftware::setLoopCount(int loopcount)
{
    FMOD_RESULT result;

    result = ChannelReal::setLoopCount(loopcount);
    if (result != FMOD_OK)
    {
        return result;
    }

#ifdef FMOD_SUPPORT_DSPCODEC
    if (mDSPCodec)
    {
        mDSPCodec->mLoopCount = mLoopCount;
    }
#endif

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
// VERIFY: a0827641 auto
// OFFSET: 0x009337C0
FMOD_RESULT ChannelSoftware::setMode(FMOD_MODE mode)
{
    FMOD_RESULT result;

    result = ChannelReal::setMode(mode);
    if (result != FMOD_OK)
    {
        return result;
    }

#ifdef FMOD_SUPPORT_DSPCODEC
    if (mDSPCodec)
    {
        mDSPCodec->mMode = mMode;
    }
#endif

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
// VERIFY: bd38a747 manual
// OFFSET: 0x009337F0
FMOD_RESULT ChannelSoftware::isPlaying(bool *isplaying, bool includethreadlatency)
{
    if (!isplaying)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (mFlags & CHANNELREAL_FLAG_ALLOCATED)
    {
        *isplaying = true;
        return FMOD_OK;
    }

#ifdef FMOD_SUPPORT_DSPCODEC
    if (mDSPCodec)
    {
        mDSPCodec->getFinished(isplaying);
        *isplaying = !*isplaying;
    }
    else if (mDSPCodecStopped && includethreadlatency)
    {
        mDSPCodecStopped->getFinished(isplaying);
        *isplaying = !*isplaying;
        if (!*isplaying)
        {
            mDSPCodecStopped = 0;
        }
    }
    else
#endif
    if (mDSPResampler)
    {
        int numinputs, count, numfinished = 0;

        *isplaying = false;

        if (mDSPResampler->getNumInputs(&numinputs) == FMOD_OK)
        {
            for (count = 0; count < numinputs; count++)
            {
                DSPI *inputdsp;
                bool  finished;

                if (mDSPResampler->getInput(count, &inputdsp) != FMOD_OK)
                {
                    break;
                }

                inputdsp->getFinished(&finished);
                if (finished)
                {
                    numfinished++;
                }
            }

            if (count >= numinputs && numfinished != numinputs)
            {
                *isplaying = true;
            }
        }
    }
    else if (mSound || mDSP)
    {
        mDSPWaveTable->getFinished(isplaying);
        *isplaying = !*isplaying;
    }
    else
    {
        *isplaying = false;
    }

    if (*isplaying)
    {
        return FMOD_OK;
    }

    if (mDelayEnd)
    {
        mFlags |= CHANNELREAL_FLAG_ENDDELAY;
        *isplaying = true;
        return FMOD_OK;
    }

    mFlags &= ~(CHANNELREAL_FLAG_ALLOCATED | CHANNELREAL_FLAG_PLAYING);

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Retrieves the spectrum from the currently playing output signal.

	[PARAMETERS]
    'spectrumarray'     Pointer to an array of floats to receive spectrum data.  Data range is 0-1.  Decibels = 10.0f * (float)log10(val) * 2.0f;
    'windowsize'        Number of PCM samples to analyze.  The resulting spectrum placed in the spectrumarray parameter will be HALF the size of this value (ie 1024 will place 512 floats in spectrumarray).  Must be a power of 2. (ie 128/256/512 etc).  Min = 128.  Max = 16384.
    'channeloffset'     Channel to analyze.  If the signal is multichannel (such as a stereo output), then this value represents which channel to analyze.  On a stereo signal 0 = left, 1 = right.
    'windowtype'        Pre-FFT window method.  This filters the PCM data before entering the spectrum analyzer to reduce transient frequency error for more accurate results.  See FMOD_DSP_FFT_WINDOW for different types of fft window techniques possible and for a more detailed explanation.

	[RETURN_VALUE]

	[REMARKS]
    The larger the windowsize, the more CPU the FFT will take.  Choose the right value to trade off between accuracy / speed.<br>
    The larger the windowsize, the more 'lag' the spectrum will seem to inherit.  This is because the window size stretches the analysis back in time to what was already played.  For example if the window size happened to be 44100 and the output rate was 44100 it would be analyzing the past second of data, and giving you the average spectrum over that time period.<br>
    If you are not displaying the result in dB, then the data may seem smaller than it should be.  To display it you may want to normalize the data - that is, find the maximum value in the resulting spectrum, and scale all values in the array by 1 / max.  (ie if the max was 0.5f, then it would become 1).<br>
    To get the spectrum for both channels of a stereo signal, call this function twice, once with channeloffset = 0, and again with channeloffset = 1.  Then add the spectrums together and divide by 2 to get the average spectrum for both channels.<br>

    [PLATFORMS]

	[SEE_ALSO]
    FMOD_DSP_FFT_WINDOW
]
*/
// VERIFY: 512591c2 auto
// OFFSET: 0x00933970
FMOD_RESULT ChannelSoftware::getSpectrum(float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
{
#ifdef FMOD_SUPPORT_GETSPECTRUM
    FMOD_RESULT     result;
    DSPFilter      *dsphead;
    float          *buffer;
    unsigned int    position, length, bufferlength;
    int             numchannels, windowsize;
    static DSPFFT   fft;

    dsphead = (DSPFilter *)mDSPHead;
    if (!dsphead)
    {
        return FMOD_ERR_INITIALIZATION;
    }

    windowsize = numvalues * 2;

    if (windowsize != (1 <<  7) &&
        windowsize != (1 <<  8) &&
        windowsize != (1 <<  9) &&
        windowsize != (1 << 10) &&
        windowsize != (1 << 11) &&
        windowsize != (1 << 12) &&
        windowsize != (1 << 13) &&
        windowsize != (1 << 14))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (mSound)
    {
        numchannels = mSound->mChannels;
    }
    else if (mDSP)
    {
        numchannels = mDSP->mDescription.channels;
    }
    else
    {
        return FMOD_ERR_INVALID_HANDLE;
    }

    if (channeloffset >= numchannels)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = dsphead->startBuffering(16384);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = dsphead->getHistoryBuffer(&buffer, &position, &length);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (windowsize > (int)length)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    mSystem->getDSPBufferSize(&bufferlength, 0);

    position -= windowsize;
    if ((int)position < 0)
    {
        position += length;
    }

    return fft.getSpectrum(buffer, position, length, spectrumarray, windowsize, channeloffset, numchannels, windowtype);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif // FMOD_SUPPORT_GETSPECTRUM
}


/*
[
	[DESCRIPTION]
    Retrieves the spectrum from the currently playing output signal.

	[PARAMETERS]
    'spectrumarray'     Pointer to an array of floats to receive spectrum data.  Data range is 0-1.  Decibels = 10.0f * (float)log10(val) * 2.0f;
    'windowsize'        Number of PCM samples to analyze.  The resulting spectrum placed in the spectrumarray parameter will be HALF the size of this value (ie 1024 will place 512 floats in spectrumarray).  Must be a power of 2. (ie 128/256/512 etc).  Min = 128.  Max = 16384.
    'channeloffset'     Channel to analyze.  If the signal is multichannel (such as a stereo output), then this value represents which channel to analyze.  On a stereo signal 0 = left, 1 = right.
    'windowtype'        Pre-FFT window method.  This filters the PCM data before entering the spectrum analyzer to reduce transient frequency error for more accurate results.  See FMOD_DSP_FFT_WINDOW for different types of fft window techniques possible and for a more detailed explanation.

	[RETURN_VALUE]

	[REMARKS]
    The larger the windowsize, the more CPU the FFT will take.  Choose the right value to trade off between accuracy / speed.<br>
    The larger the windowsize, the more 'lag' the spectrum will seem to inherit.  This is because the window size stretches the analysis back in time to what was already played.  For example if the window size happened to be 44100 and the output rate was 44100 it would be analyzing the past second of data, and giving you the average spectrum over that time period.<br>
    If you are not displaying the result in dB, then the data may seem smaller than it should be.  To display it you may want to normalize the data - that is, find the maximum value in the resulting spectrum, and scale all values in the array by 1 / max.  (ie if the max was 0.5f, then it would become 1).<br>
    To get the spectrum for both channels of a stereo signal, call this function twice, once with channeloffset = 0, and again with channeloffset = 1.  Then add the spectrums together and divide by 2 to get the average spectrum for both channels.<br>

    [PLATFORMS]

	[SEE_ALSO]
    FMOD_DSP_FFT_WINDOW
]
*/
// VERIFY: f1b1f7a1 manual
// OFFSET: 0x00933AB0
FMOD_RESULT ChannelSoftware::getWaveData(float *wavearray, int numvalues, int channeloffset)
{
    FMOD_RESULT     result;
    DSPFilter      *dsphead;
    float          *buffer;
    unsigned int    position, length;
    int             numchannels, count;

    dsphead = (DSPFilter *)mDSPHead;
    if (!dsphead)
    {
        return FMOD_ERR_INITIALIZATION;
    }

    if (mSound)
    {
        numchannels = mSound->mChannels;
    }
    else if (mDSP)
    {
        numchannels = mDSP->mDescription.channels;
    }
    else
    {
        return FMOD_ERR_INVALID_HANDLE;
    }

    if (channeloffset >= numchannels)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = dsphead->startBuffering(16384);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = dsphead->getHistoryBuffer(&buffer, &position, &length);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (numvalues > (int)length)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    position -= numvalues;
    if ((int)position < 0)
    {
        position += length;
    }

    for (count = 0; count < numvalues; count++)
    {
        wavearray[count] = buffer[position*numchannels+channeloffset];
        position++;
        if (position >= length)
        {
            position = 0;
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
// VERIFY: 46b9799a auto
// OFFSET: 0x00933C40
FMOD_RESULT ChannelSoftware::getDSPHead(DSPI **dsp)
{
    *dsp = mDSPHead;

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
// VERIFY: d309c00b manual
// OFFSET: 0x00933C60
FMOD_RESULT ChannelSoftware::addToReverbs(DSPI *dsptarget)
{
#ifdef FMOD_SUPPORT_REVERB
    FMOD_RESULT                   result;
    DSPConnectionI               *connection;
    FMOD_REVERB_CHANNELPROPERTIES props = { 0, 0, 0, 0, 0, 0.0f, 0, 0.25f, 1.5f, 1.0f, 0, 1.0f, 0, 0.0f, 0.0f, 0.0f, 1.0f, 0 };

    if (!dsptarget)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    /*
        Add an input to standard reverb
    */
    if (mSystem->mReverbGlobal.mDSP)
    {
        result = mSystem->mReverbGlobal.mDSP->addInputQueued(dsptarget, false, 0, &connection);
        if (result != FMOD_OK)
        {
            return result;
        }

        mSystem->mReverbGlobal.setChanProperties(mParent->mIndex, &props, connection);
        mSystem->mReverbGlobal.setPresenceGain(mParent->mIndex, 1.0f);
    }

#ifdef FMOD_SUPPORT_MULTIREVERB
    /*
        Add an input to 3d reverb
    */
    if (mSystem->mReverb3D.mDSP)
    {
        result = mSystem->mReverb3D.mDSP->addInputQueued(dsptarget, false, 0, &connection);
        if (result != FMOD_OK)
        {
            return result;
        }

        mSystem->mReverb3D.setChanProperties(mParent->mIndex, &props, connection);
        mSystem->mReverb3D.setPresenceGain(mParent->mIndex, 1.0f);
    }

    /*
        Traverse reverb list and add an input to each one that has a DSP
    */
    ReverbI *reverb_c = SAFE_CAST(ReverbI, mSystem->mReverb3DHead.getNext());
    while (reverb_c != &(mSystem->mReverb3DHead))
    {
        if (reverb_c->mDSP)
        {
            result = reverb_c->mDSP->addInputQueued(dsptarget, false, 0, &connection);
            if (result != FMOD_OK)
            {
                return result;
            }

            reverb_c->setChanProperties(mParent->mIndex, &props, connection);
            reverb_c->setPresenceGain(mParent->mIndex, 1.0f);
        }
        reverb_c = SAFE_CAST(ReverbI, reverb_c->getNext());
    }
#endif

#endif
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
// VERIFY: 2f109c1b manual
// OFFSET: 0x00933E00
FMOD_RESULT ChannelSoftware::moveChannelGroup(ChannelGroupI *oldchannelgroup, ChannelGroupI *newchannelgroup)
{
    FMOD_RESULT result;

    if (oldchannelgroup == newchannelgroup)
    {
        return FMOD_OK;
    }

    if (mDSPLowPass)
    {
        /*
            1. disconnect from previous channel group's head.
        */
        if (oldchannelgroup && oldchannelgroup->mDSPMixTarget)
        {
            result = oldchannelgroup->mDSPMixTarget->disconnectFrom(mDSPHead);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        /*
            2. reconnect to DSP group's head.
        */
        result = newchannelgroup->mDSPMixTarget->addInputQueued(mDSPHead, false, mDSPConnection, &mDSPConnection);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    else
    {
        if (oldchannelgroup && oldchannelgroup->mDSPMixTarget)
        {
            result = oldchannelgroup->mDSPMixTarget->disconnectFrom(mDSPHead);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        result = newchannelgroup->mDSPMixTarget->addInputQueued(mDSPHead, false, mDSPConnection, &mDSPConnection);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    return FMOD_OK;
}


}

#endif
