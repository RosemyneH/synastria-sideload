#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL

#include "fmod_3d.h"
#include "fmod_channel_openal.h"
#include "fmod_debug.h"
#ifdef FMOD_SUPPORT_FSB
#   include "fmod_codec_fsb.h"
#endif
#ifdef FMOD_SUPPORT_MPEG
#   include "fmod_codec_mpeg.h"
#endif
#if defined(FMOD_SUPPORT_WAV) && defined(FMOD_SUPPORT_IMAADPCM)
#   include "fmod_codec_wav.h"
#   include "fmod_codec_wav_imaadpcm.h"
#endif
#include "fmod_dsp_codec.h"
#include "fmod_dsp_fft.h"
#include "fmod_dsp_resampler.h"
#include "fmod_dsp_wavetable.h"
#include "fmod_dspi.h"
#include "fmod_sample_openal.h"
#include "fmod_systemi.h"

#include "fmod_output_openal.h"

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
// VERIFY: d1dc045c manual
// OFFSET: 0x00946FD0
ChannelOpenAL::ChannelOpenAL()
{
    mDSPWaveTable     = NULL;
    mDSPHead          = NULL;
    mDSPResampler     = NULL;
    mDSPCodec         = NULL;
    mDSPLowPass       = NULL;
    mDSP              = NULL;
    mDSPConnection    = NULL;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: eb7e5ba8 manual
// OFFSET: 0x009478C0
FMOD_RESULT ChannelOpenAL::init(int index, SystemI *system, Output *output, DSPI *dspmixtarget)
{
    FMOD_RESULT              result;
    FMOD_DSP_DESCRIPTION_EX  descriptionex;

    ChannelReal::init(index, system, output, dspmixtarget);

    /*
        Create a head unit that things can connect to
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
        Make sure nothing else can connect to mDSPHead
    */
	mDSPHead->mDescription.mCategory = FMOD_DSP_CATEGORY_SOUNDCARD;

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
    descriptionex.channels      = 1;
    descriptionex.mCategory     = FMOD_DSP_CATEGORY_WAVETABLE;
    descriptionex.mFormat       = FMOD_SOUND_FORMAT_PCMFLOAT;

    descriptionex.read          = NULL;                                  /* DSPWavetable uses DSPWaveTable::execute not a read callback. */
    descriptionex.setparameter  = DSPWaveTable::setParameterCallback;
    descriptionex.getparameter  = DSPWaveTable::getParameterCallback;
    descriptionex.setposition   = DSPWaveTable::setPositionCallback;

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
    result = mDSPWaveTable->setTargetFrequency(44100);
    if (result != FMOD_OK)
    {
        return result;
    }

    mMinFrequency = -mMaxFrequency;

    // Clear OpenAL sources assiociated with this channel
	mNumSources = 0;
	FMOD_memset(mSources, 0, 16 * sizeof(ALuint));

    // OutputOpenAL that this channel belongs to
    mOutputOAL = (OutputOpenAL *)output;

    // Allocate memory for final stream buffer
	mBuffer = (short *)FMOD_Memory_Calloc(mOutputOAL->mBufferLength * sizeof(short));

    // Allocate memory for temporary deinterleave stream buffer
	mTempBuffer = (float *)FMOD_Memory_Calloc(mOutputOAL->mBufferLength * sizeof(float));

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 622da562 manual
// OFFSET: 0x009451A0
FMOD_RESULT ChannelOpenAL::close()
{
    FMOD_RESULT result;

    result = ChannelReal::close();
    if (result != FMOD_OK)
    {
        return result;
    }

	if (mTempBuffer)
    {
        FMOD_Memory_Free(mTempBuffer);
        mTempBuffer = NULL;
    }

    if (mBuffer)
    {
        FMOD_Memory_Free(mBuffer);
        mBuffer = NULL;
    }

    if (mDSPWaveTable)
    {
        mDSPWaveTable->release(false);  /* false = dont free this, as it is not alloced. */
        mDSPWaveTable = NULL;
    }

    if (mDSPHead)
    {
        mDSPHead->release(false);       /* false = dont free this, as it is not alloced. */
        mDSPHead= NULL;
    }

    if (mDSPResampler)
    {
        mDSPResampler->release();
        mDSPResampler = NULL;
    }

    mDSPCodec = NULL;                   /* Don't free, it points to a pool codec. */

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
// VERIFY: a944ad46 manual
// OFFSET: 0x00945260
FMOD_RESULT ChannelOpenAL::setupDSPCodec(DSPI *dsp)
{
#ifdef FMOD_SUPPORT_DSPCODEC
    FMOD_RESULT     result;
    DSPCodec       *dspcodec;
    Codec          *codec, *soundcodec;
    SampleOpenAL   *sample;

    sample     = SAFE_CAST(SampleOpenAL, mSound);
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

    soundcodec->mDescription.getwaveformat(soundcodec, mSound->mSubSoundIndex, codec->waveformat);

    codec->mFlags = soundcodec->mFlags;

    dspcodec->mCodec->mFile = &dspcodec->mMemoryFile;
    dspcodec->mMemoryFile.init(mSound->mLengthBytes, 0);
    dspcodec->mMemoryFile.mMem = sample->mBuffer;

    if (0)
    {
    }
    #if defined(FMOD_SUPPORT_WAV) && defined(FMOD_SUPPORT_IMAADPCM)
    else if (mSound->mType == FMOD_SOUND_TYPE_WAV && mSound->mFormat == FMOD_SOUND_FORMAT_IMAADPCM)
    {
        CodecWav *srcwav  = (CodecWav *)soundcodec;
        CodecWav *destwav = (CodecWav *)codec;

        destwav->mSamplesPerADPCMBlock = srcwav->mSamplesPerADPCMBlock;
        destwav->mReadBufferLength     = srcwav->mReadBufferLength;
    }
    #endif
    #if defined(FMOD_SUPPORT_WAV) && defined(FMOD_SUPPORT_RAW) && defined(FMOD_SUPPORT_IMAADPCM)
    else if (mSound->mType == FMOD_SOUND_TYPE_RAW && mSound->mFormat == FMOD_SOUND_FORMAT_IMAADPCM)
    {
        CodecRaw *srcwav  = (CodecRaw *)soundcodec;
        CodecWav *destwav = (CodecWav *)codec;

        destwav->mSamplesPerADPCMBlock = srcwav->mSamplesPerADPCMBlock;
        destwav->mReadBufferLength     = srcwav->mReadBufferLength;
    }
    #endif
    #if defined(FMOD_SUPPORT_FSB)
    else if (mSound->mType == FMOD_SOUND_TYPE_FSB)
    {
        CodecFSB  *fsb = (CodecFSB *)soundcodec;

        if (0)
        {
        }
        #if defined(FMOD_SUPPORT_MPEG)
        else if (mSound->mFormat == FMOD_SOUND_FORMAT_MPEG)
        {
            CodecMPEG *destmpeg = (CodecMPEG *)codec;

            destmpeg->mPCMFrameLengthBytes = codec->waveformat[0].channels * 2304;
        }
        #endif
        #if defined(FMOD_SUPPORT_IMAADPCM)
        else if (mSound->mFormat == FMOD_SOUND_FORMAT_IMAADPCM)
        {
            CodecWav *srcwav  = fsb->mADPCM;
            CodecWav *destwav = (CodecWav *)codec;

            destwav->mSamplesPerADPCMBlock = srcwav->mSamplesPerADPCMBlock;
            destwav->mReadBufferLength     = codec->waveformat[0].channels * 36;
        }
        #endif

        fsb->mDescription.getwaveformat(fsb, mSound->mSubSoundIndex, codec->waveformat);
    }
    #endif
    #if defined(FMOD_SUPPORT_MPEG)
    else if (mSound->mFormat == FMOD_SOUND_FORMAT_MPEG)
    {
        CodecMPEG *srcmpeg  = (CodecMPEG *)soundcodec;
        CodecMPEG *destmpeg = (CodecMPEG *)codec;

        destmpeg->mPCMFrameLengthBytes = srcmpeg->mPCMFrameLengthBytes;
    }
    #endif

    dsp->mDescription.channels = mSound->mChannels;

    result = dsp->setTargetFrequency((int)mParent->mChannelGroup->mDSPHead->mDefaultFrequency);
    if (result != FMOD_OK)
    {
        return result;
    }

    mDSPCodec                    = dspcodec;
    mMinFrequency                = 0;
    mDSPCodec->mLength           = mSound->mLength;
    mDSPCodec->mLoopStart        = mLoopStart;
    mDSPCodec->mLoopLength       = mLoopLength;
    mDSPCodec->mLoopCount        = mLoopCount;
    mDSPCodec->mMode             = mMode;
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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 34314566 manual
// OFFSET: 0x00945480
FMOD_RESULT ChannelOpenAL::alloc()
{
    FMOD_RESULT result;
    int         requiredSources;

    if (!(mMode & FMOD_CREATECOMPRESSEDSAMPLE))
    {
        DSPWaveTable *dspwave = SAFE_CAST(DSPWaveTable, mDSPWaveTable);
        if (!dspwave)
        {
            return FMOD_ERR_INTERNAL;
        }

        mDSPCodec = 0;

        result = mDSPHead->disconnectFromInternal(0);
        if (result != FMOD_OK)
        {
            return result;
        }
        if (mDSPLowPass)
        {
            result = mDSPLowPass->disconnectFromInternal(0);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        result = mDSPWaveTable->disconnectFromInternal(0);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (mDSPLowPass)
        {
            result = mDSPHead->addInputInternal(mDSPLowPass, false);
            if (result != FMOD_OK)
            {
                return result;
            }
            result = mDSPLowPass->addInputInternal(mDSPWaveTable, false);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        else
        {
            result = mDSPHead->addInputInternal(mDSPWaveTable, false);
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

        dspwave->mChannel      = this;
        dspwave->mSound        = mSound;
        dspwave->mPosition.mHi = 0;
        dspwave->mPosition.mLo = 0;
        dspwave->mDirection    = DSPWAVETABLE_SPEEDDIR_FORWARDS;

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
    {
#ifdef FMOD_SUPPORT_DSPCODEC
        DSPCodec       *dsp;
        bool            dspfinished = true;

        if (mDSPCodec)
        {
            mDSPCodec->getFinished(&dspfinished);
            if (!dspfinished)
            {
                mSystem->flushDSPConnectionRequests(true);
            }
        }

        result = mDSPHead->disconnectFromInternal(0);
        if (result != FMOD_OK)
        {
            return result;
        }
        if (mDSPLowPass)
        {
            result = mDSPLowPass->disconnectFromInternal(0);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        if (mDSPWaveTable)
        {
            result = mDSPWaveTable->disconnectFromInternal(0);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        result = mSystem->allocDSPCodec(mSound->mFormat, &dsp);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (!dspfinished)
        {
            FMOD_OS_CriticalSection_Enter(mSystem->mDSPCrit);
        }
        result = setupDSPCodec(dsp);
        if (!dspfinished)
        {
            FMOD_OS_CriticalSection_Leave(mSystem->mDSPCrit);
        }
        if (result != FMOD_OK)
        {
            return result;
        }

        if (mDSPLowPass)
        {
            result = mDSPHead->addInputInternal(mDSPLowPass, false);
            if (result != FMOD_OK)
            {
                return result;
            }
            result = mDSPLowPass->addInputInternal(dsp, false);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        else
        {
            result = mDSPHead->addInputInternal(dsp, false);
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

    switch (mSystem->mSpeakerMode)
    {
        case FMOD_SPEAKERMODE_MONO    : requiredSources = 1;
                                        break;
        case FMOD_SPEAKERMODE_QUAD    : requiredSources = 4;
                                        break;
        case FMOD_SPEAKERMODE_5POINT1 : requiredSources = 6;
                                        break;
        case FMOD_SPEAKERMODE_7POINT1 : requiredSources = 8;
                                        break;
        default                       : requiredSources = 2;
                                        break;
    }

    if (mSound->mChannels < requiredSources)
    {
        requiredSources = mSound->mChannels;
    }

    for (int i = 0; i < mOutputOAL->mNumSources; i++)
	{
        if (!mOutputOAL->mSources[i].used)
		{
            mSources[mNumSources] = (SourceOpenAL*)&mOutputOAL->mSources[i];
			mSources[mNumSources]->used = true;

            if (++mNumSources == requiredSources)
            {
				break;
            }
		}
	}

	setupChannel();

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

        return dspresampler->setFrequency(mSound->mDefaultFrequency);
    }
    else
    {
        DSPWaveTable *dspwave;

        dspwave = SAFE_CAST(DSPWaveTable, mDSPWaveTable);
        if (!dspwave)
        {
            return FMOD_ERR_INVALID_PARAM;
        }

        return dspwave->setFrequency(mSound->mDefaultFrequency);
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
// VERIFY: 4327641d manual
// OFFSET: 0x00947040
FMOD_RESULT ChannelOpenAL::alloc(DSPI *dsp)
{
    FMOD_RESULT result;
    int         requiredSources;

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

    result = mDSPHead->disconnectFromInternal(0);
    if (result != FMOD_OK)
    {
        return result;
    }
    if (mDSPLowPass)
    {
        result = mDSPLowPass->disconnectFromInternal(0);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    result = mDSPWaveTable->disconnectFromInternal(0);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mDSPHead->addInputInternal(mDSPResampler, false);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = mDSPResampler->addInputInternal(dsp, false);
    if (result != FMOD_OK)
    {
        return result;
    }

    {
        DSPResampler *resampler = SAFE_CAST(DSPResampler, mDSPResampler);

        resampler->mLength     = mLength;
        resampler->mLoopStart  = mLoopStart;
        resampler->mLoopLength = mLoopLength;
        resampler->mLoopCount  = mLoopCount;
        resampler->mMode       = mMode;

        DSPWaveTable *dspwave = SAFE_CAST(DSPWaveTable, mDSPWaveTable);
        if (!dspwave)
        {
            return FMOD_ERR_INTERNAL;
        }
        dspwave->mSound = 0;
    }

    mDSPHead->setFinished(false, false);
    mDSPHead->setActive(false);
    mDSPResampler->setFinished(false, false);
    mDSPResampler->setActive(false);
    dsp->setFinished(false, false);
    dsp->setActive(false);

    switch (mSystem->mSpeakerMode)
    {
        case FMOD_SPEAKERMODE_MONO    : requiredSources = 1;
                                        break;
        case FMOD_SPEAKERMODE_QUAD    : requiredSources = 4;
                                        break;
        case FMOD_SPEAKERMODE_5POINT1 : requiredSources = 6;
                                        break;
        case FMOD_SPEAKERMODE_7POINT1 : requiredSources = 8;
                                        break;
        default                       : requiredSources = 2;
                                        break;
    }

    if (mSound->mChannels < requiredSources)
    {
        requiredSources = mSound->mChannels;
    }

    for (int i = 0; i < mOutputOAL->mNumSources; i++)
	{
        if (!mOutputOAL->mSources[i].used)
		{
            mSources[mNumSources] = (SourceOpenAL*)&mOutputOAL->mSources[i];
			mSources[mNumSources]->used = true;

            if (++mNumSources == requiredSources)
            {
				break;
            }
		}
	}

	setupChannel();

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

        return dspresampler->setFrequency(mSound->mDefaultFrequency);
    }
    else
    {
        DSPWaveTable *dspwave;

        dspwave = SAFE_CAST(DSPWaveTable, mDSPWaveTable);
        if (!dspwave)
        {
            return FMOD_ERR_INVALID_PARAM;
        }

        return dspwave->setFrequency(mSound->mDefaultFrequency);
    }
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: bb077191 manual
// OFFSET: 0x00945890
FMOD_RESULT ChannelOpenAL::start()
{
    SampleOpenAL *sampleopenal;

    sampleopenal = SAFE_CAST(SampleOpenAL, mSound);
    if (!sampleopenal)
    {
    	return FMOD_ERR_INVALID_PARAM;
    }
    
    // Tell the mixer to fill up all buffers with initial data
    mInitialFill = true;

    for (int count = 0; count < mNumSources; count++)
	{
		/*
			Setup the OpenAL source
		*/
		if (sampleopenal->mMode & FMOD_3D)
		{
			mOutputOAL->mOALFnTable.alSourcei(mSources[count]->sid, AL_SOURCE_RELATIVE, AL_FALSE);
		}
		else
		{
			/*
				OpenAL doesn't have 2D sounds, so set up some 3D properties to fudge it
			*/
			mOutputOAL->mOALFnTable.alSourcei(mSources[count]->sid, AL_SOURCE_RELATIVE, AL_TRUE);
			mOutputOAL->mOALFnTable.alSourcef(mSources[count]->sid, AL_REFERENCE_DISTANCE, 1.0f);
			mOutputOAL->mOALFnTable.alSourcef(mSources[count]->sid, AL_MAX_DISTANCE, 1.0f);
			switch (sampleopenal->mChannels)
			{
				case 0x01: // Mono
					mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.0f, 0.0f, 0.0f);
					break;
				case 0x02: // Stereo
					switch (count)
					{
						case 0x00:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, -0.5f, 0.0f, -0.866f);
							break;
						case 0x01:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.5f, 0.0f, -0.866f);
							break;
					}
					break;
				case 0x04: // Quad
					switch (count)
					{
						case 0x00:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, -0.5f, 0.0f, -0.866f);
							break;
						case 0x01:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.5f, 0.0f, -0.866f);
							break;
						case 0x02:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, -0.5f, 0.0f, 0.866f);
							break;
						case 0x03:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.5f, 0.0f, 0.866f);
							break;
					}
					break;
				case 0x06: // 5.1
					switch (count)
					{
						case 0x00:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, -0.5f, 0.0f, -0.866f);
							break;
						case 0x01:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.5f, 0.0f, -0.866f);
							break;
						case 0x02:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.0f, 0.0f, -1.0f);
							break;
						case 0x03:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.0f, 0.0f, 0.0f);
							break;
						case 0x04:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, -0.5f, 0.0f, 0.866f);
							break;
						case 0x05:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.5f, 0.0f, 0.866f);
							break;
					}
					break;
				case 0x08: // 7.1
					switch (count)
					{
						case 0x00:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, -0.5f, 0.0f, -0.866f);
							break;
						case 0x01:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.5f, 0.0f, -0.866f);
							break;
						case 0x02:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.0f, 0.0f, -1.0f);
							break;
						case 0x03:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.0f, 0.0f, 0.0f);
							break;
						case 0x04:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, -0.5f, 0.0f, 0.866f);
							break;
						case 0x05:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.5f, 0.0f, 0.866f);
							break;
						case 0x06:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, -1.0f, 0.0f, 0.0f);
							break;
						case 0x07:
							mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 1.0f, 0.0f, 0.0f);
							break;
					}
					break;
				default:
					mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, 0.0f, 0.0f, 0.0f);
					break;
			}
		}
		mOutputOAL->mOALFnTable.alSourcei(mSources[count]->sid, AL_LOOPING, AL_FALSE);
		mOutputOAL->mOALFnTable.alSourcef(mSources[count]->sid, AL_PITCH, 1.0f);
	}

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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 2476c3a8 manual
// OFFSET: 0x00945CD0
FMOD_RESULT ChannelOpenAL::stop(bool ignoreenddelay, bool resetstate, bool force)
{
    FMOD_RESULT result;
    int count;
	int i;

    if (mFlags & CHANNELREAL_FLAG_ENDDELAY && !ignoreenddelay)
    {
        return FMOD_OK;
    }

    if (mDSPHead)
    {
        mDSPHead->setFinished(true, true);
        mDSPHead->setActive(false);
        mDSPHead->disconnectAll(false, true);
    }

#ifdef FMOD_SUPPORT_DSPCODEC
    if (mDSPCodec)
    {
        mDSPCodec->setFinished(true, true);

        result = mDSPCodec->disconnectFromInternal(0);
        if (result != FMOD_OK)
        {
            return result;
        }

        mDSPCodec->setActive(false);
        mDSPCodec->freeFromPool();
        mDSPCodec = 0;
    }
#endif

    if (mDSPResampler)
    {
        mDSPResampler->setFinished(true, true);
        mDSPResampler->setActive(false);
        mDSPResampler->release();
        mDSPResampler = 0;
    }

    if (mDSPWaveTable)
    {
        mDSPWaveTable->setFinished(true, true);
        mDSPWaveTable->setActive(false);
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
                result = prev->disconnectFromInternal(mDSP);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
        }
    }

    FMOD_OS_CriticalSection_Enter(mOutputOAL->mSystem->mDSPCrit);

    for(i = 0; i < mNumSources; i++)
	{
		mOutputOAL->mOALFnTable.alSourceStop(mSources[i]->sid);
		mOutputOAL->mOALFnTable.alSourcei(mSources[i]->sid, AL_BUFFER, 0);
		mSources[i]->used = false;
		mSources[i] = NULL;
	}
	mNumSources = 0;

    FMOD_OS_CriticalSection_Leave(mOutputOAL->mSystem->mDSPCrit);

    ChannelReal::stop(ignoreenddelay, true);

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Pause or unpause the channel

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 7d9628c8 manual
// OFFSET: 0x00945ED0
FMOD_RESULT ChannelOpenAL::setPaused(bool paused)
{
    FMOD_RESULT   result         = FMOD_OK;
    ALuint        sourceIDs[16]  = {0};

    // Pause or unpause the DSP
    result = mDSPHead->setActive(!paused);
    if (result != FMOD_OK)
    {
        return result;
    }
	
    // Get the IDs of all the sources used by this channel
	for (int i = 0; i < mNumSources; i++)
	{
		sourceIDs[i] = mSources[i]->sid;
	} 
    
    /*
        To avoid spamming the hardware only pause when required (i.e. when not already paused)
    */
    // If we want to pause...
    if (paused)
    {
        // ...and we were not already paused
        if (!(mFlags & CHANNELREAL_FLAG_PAUSED))
        {
            mOutputOAL->mOALFnTable.alSourcePausev(mNumSources, sourceIDs);
        }
    }
    // If we want to unpause
    else 
    {
	    // ... and we were already paused
        if (mFlags & CHANNELREAL_FLAG_PAUSED)
        {
            mOutputOAL->mOALFnTable.alSourcePlayv(mNumSources, sourceIDs);
        }
    }

	return(ChannelReal::setPaused(paused));
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 294bb944 auto
// OFFSET: 0x00945FA0
FMOD_RESULT ChannelOpenAL::setVolume(float volume)
{
    int count;

    if (mOutputOAL->mReverbVersion == REVERB_VERSION_NONE)
    {
		volume = mParent->mChannelGroup->mRealDirectOcclusionVolume * mParent->mDirectOcclusionVolume * volume;
	}
    volume *= mParent->mChannelGroup->mRealVolume;
    if (mMode & FMOD_3D)
    {
        if (mMode & (FMOD_3D_LINEARROLLOFF | FMOD_3D_CUSTOMROLLOFF) || mSystem->mRolloffCallback)
        {
            volume *= mParent->mVolume3D;
        }

        volume *= mParent->mConeVolume3D;
    }
    if (mSound && mSound->mSoundGroup)
    {
        volume *= mSound->mSoundGroup->mVolume;
    }
	for (count = 0; count < mNumSources; count++)
	{
		mOutputOAL->mOALFnTable.alSourcef(mSources[count]->sid, AL_GAIN, volume);
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
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelOpenAL::setSpeakerLevels(int speaker, float *levels, int numlevels)
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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 634a5f1a auto
// OFFSET: 0x00946090
FMOD_RESULT ChannelOpenAL::setFrequency(float frequency)
{
    float basefrequency = mSound->mDefaultFrequency;
	int count;
    ALfloat pitch = 0.0f;

    frequency *= mParent->mChannelGroup->mRealPitch;

    /*
        Convert frequency to pitch multiplier
    */       
    pitch = frequency/basefrequency;
 
    /*
        This needs to be calculated properly
    */
	for (count = 0; count < mNumSources; count++)
	{
		mOutputOAL->mOALFnTable.alSourcef(mSources[count]->sid, AL_PITCH, pitch);
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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 1ff35466 auto
// OFFSET: 0x00946100
FMOD_RESULT ChannelOpenAL::setPan(float pan, float fbpan)
{
    int count;

    if (mSound->mMode & FMOD_3D)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    /*
        Use 3D position, left speaker is at -30 degrees (sin -30, 0, -cos -30), right speaker is at +30 degrees (sin 30, 0, -cos 30)
    */
	for (count = 0; count < mNumSources; count++)
	{
		mOutputOAL->mOALFnTable.alSource3f(mSources[count]->sid, AL_POSITION, FMOD_SIN(30.0f * pan * FMOD_PI / 180.0f), 0.0f, -FMOD_COS(30.0f * pan * FMOD_PI / 180.0f));
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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 99fad7ba manual
// OFFSET: 0x00947350
FMOD_RESULT ChannelOpenAL::setPosition(unsigned int position, FMOD_TIMEUNIT postype)
{
    unsigned int      pcm, endpoint;
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
    else 
    {
        return mDSPHead->setPosition(pcm, true);  /* Channel based sound is always wavetable, dspcodec, or resampler.  If this is called they must have done a dsp connection and it will flush. */
    }    
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: d05b1e3a manual
// OFFSET: 0x009475F0
FMOD_RESULT ChannelOpenAL::getPosition(unsigned int *position, FMOD_TIMEUNIT postype)
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
        DSPResampler *dspcodec = SAFE_CAST(DSPResampler, mDSPCodec);
        if (!dspcodec)
        {
            return FMOD_ERR_INVALID_PARAM;
        }

        channels    = mDSPCodec->mDescription.channels;
        format      = FMOD_SOUND_FORMAT_PCMFLOAT;
        frequency   = mSound->mDefaultFrequency;

        mPosition = dspcodec->mPosition.mHi;
    }    
    else
#endif
    if (mSound)
    {
        DSPWaveTable *dspwave = SAFE_CAST(DSPWaveTable, mDSPWaveTable);
        if (!dspwave)
        {
            return FMOD_ERR_INVALID_PARAM;
        }

        channels    = mSound->mChannels;
        format      = mSound->mFormat;
        frequency   = mSound->mDefaultFrequency;
    
        mPosition = dspwave->mPosition.mHi;       
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
    else if (postype == FMOD_TIMEUNIT_MS)
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
// VERIFY: be55d107 auto
// OFFSET: 0x00946190
FMOD_RESULT ChannelOpenAL::setLoopPoints(unsigned int loopstart, unsigned int looplength)
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
// VERIFY: 1c6e6002 auto
// OFFSET: 0x009461E0
FMOD_RESULT ChannelOpenAL::setLoopCount(int loopcount)
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
// VERIFY: aa34ae22 auto
// OFFSET: 0x00946210
FMOD_RESULT ChannelOpenAL::setMode(FMOD_MODE mode)
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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: c91a1387 manual
// OFFSET: 0x00946240
FMOD_RESULT ChannelOpenAL::set3DAttributes()
{
    FMOD_RESULT   result;
    int  count;
    int           numlisteners;
    ALfloat       mindistance;
    ALfloat       maxdistance;
    ALfloat       originalposition[] = { 0.0, 0.0, 0.0 };
    ALfloat       position[] = { 0.0, 0.0, 0.0 };
    ALfloat       velocity[] = { 0.0, 0.0, 0.0 };

    if (!(mSound->mMode & FMOD_3D))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!mOutputOAL)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = mSystem->get3DNumListeners(&numlisteners);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (numlisteners == 1)
    {
        position[0] = mParent->mPosition3D.x;
        position[1] = mParent->mPosition3D.y;
        position[2] = mParent->mPosition3D.z;
    }
    else
    {
        position[0] = 0;
        position[1] = 0;      
        position[2] = mParent->mDistance;
    }

    if (numlisteners == 1)
    {
        velocity[0] = mParent->mVelocity3D.x;
        velocity[1] = mParent->mVelocity3D.y;
        velocity[2] = mParent->mVelocity3D.z;
    }
    else
    {
        velocity[0] = 0;
        velocity[1] = 0;
        velocity[2] = 0;
    }

    if (!(mSystem->mFlags & FMOD_INIT_3D_RIGHTHANDED))
    {
        position[2] *= -1;
        velocity[2] *= -1;
    }

    if (mMode & (FMOD_3D_LINEARROLLOFF | FMOD_3D_CUSTOMROLLOFF) || mSystem->mRolloffCallback)
    {
        FMOD_VECTOR diff;
        float       distance;
        FMOD_VECTOR pos;

        mindistance = 1.0f;
        maxdistance = 1.0f;

        pos.x = position[0];
        pos.y = position[1];
        pos.z = position[2];

        FMOD_Vector_Subtract(&pos, &mSystem->mListener[0].mPosition, &diff);

        distance = FMOD_Vector_GetLengthFast(&diff);
        
        if (distance > 0.0f)
        {
            FMOD_Vector_Scale(&diff, 1.0f / distance, &diff);
        }
        else
        {
            FMOD_Vector_Set(&diff, 0, 0, 0);
        }

        FMOD_Vector_Add(&mSystem->mListener[0].mPosition, &diff, &pos);

        position[0] = pos.x;
        position[1] = pos.y;
        position[2] = pos.z;

        setVolume(mParent->mVolume);
    }
    else
    {
    	mindistance = mParent->mMinDistance;
	    maxdistance = mParent->mMaxDistance;

        if (mParent->mConeOutsideAngle < 360.0f || mParent->mConeInsideAngle < 360.0f)    /* Because the first part of this if statement calls setvolume all the time, we'll just call it here if cones are used. */
        {
            setVolume(mParent->mVolume);
        }
    }

	originalposition[0] = position[0];
	originalposition[1] = position[1];
	originalposition[2] = position[2];

    /*
        Set the source properties
    */
	for (count = 0; count < mNumSources; count++)
	{
		//mParent->mSpread  L R FC SW BL BR
		//make a table for each channel count and source index fraction:
		//stereo -> -0.50f, 0.50f
		//quad   -> -0.50f, 0.50f, 0.0f, 0.0f
		//5.1    -> -0.25f, 0.25f, 0.0f, 0.0f, -0.50f, 0.50f
		//7.1    -> -0.17f, 0.17f, 0.0f, 0.0f, -0.50f, 0.50f, -0.33f, 0.33f
		static float spreadAdjust[9][8]=
		{
			{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f},
			{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f},
			{-0.50f, 0.50f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f},
			{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f},
			{-0.50f, 0.50f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f},
			{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f},
			{-0.25f, 0.25f, 0.00f, 0.00f,-0.50f, 0.50f, 0.00f, 0.00f},
			{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f},
			{-0.17f, 0.17f, 0.00f, 0.00f,-0.50f, 0.50f,-0.33f, 0.33f}
		};

		if (mParent->mSpread != 0.0f)
		{
			FMOD_VECTOR temp;

			temp.x = originalposition[0];
			temp.y = originalposition[1];
			temp.z = originalposition[2];
			FMOD_Vector_AxisRotate(&temp,&mSystem->mListener[0].mPosition,&mSystem->mListener[0].mUp,-spreadAdjust[mNumSources][count] * mParent->mSpread * FMOD_PI / 180.0f);
	        position[0] = temp.x;
		    position[1] = temp.y;
			position[2] = temp.z;
		}

		mOutputOAL->mOALFnTable.alSourcefv(mSources[count]->sid, AL_POSITION, position);
		mOutputOAL->mOALFnTable.alSourcefv(mSources[count]->sid, AL_VELOCITY, velocity);
		mOutputOAL->mOALFnTable.alSourcef(mSources[count]->sid, AL_REFERENCE_DISTANCE, mindistance);
		mOutputOAL->mOALFnTable.alSourcef(mSources[count]->sid, AL_MAX_DISTANCE, maxdistance);
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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: ffa8ca1a auto
// OFFSET: 0x00946720
FMOD_RESULT ChannelOpenAL::set3DMinMaxDistance()
{
    int count;

    if (!(mSound->mMode & FMOD_3D))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!mOutputOAL)
    {
        return FMOD_ERR_INVALID_PARAM;
    }
    
	for (count = 0; count < mNumSources; count++)
	{
	    mOutputOAL->mOALFnTable.alSourcef(mSources[count]->sid, AL_REFERENCE_DISTANCE, mParent->mMinDistance);
		mOutputOAL->mOALFnTable.alSourcef(mSources[count]->sid, AL_MAX_DISTANCE, mParent->mMaxDistance);
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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: a6777747 auto
// OFFSET: 0x009467C0
FMOD_RESULT ChannelOpenAL::set3DOcclusion(float directOcclusion, float reverbOcclusion)
{
    if (mOutputOAL->mReverbVersion != REVERB_VERSION_NONE)
    {
        float directTransmission = (1.0f - directOcclusion) *
                                   mParent->mChannelGroup->mRealDirectOcclusionVolume;

        if (directTransmission < 0.00001f)
        {
            directTransmission = 0.00001f;
        }
       
        int directGain = int(100.0f * 20.0f * FMOD_LOG10(directTransmission));

        float reverbTransmission = (1.0f - reverbOcclusion) *
                                   mParent->mChannelGroup->mRealReverbOcclusionVolume;
        if (reverbTransmission < 0.00001f)
        {
            reverbTransmission = 0.00001f;
        }
        int reverbGain = int(100.0f * 20.0f * FMOD_LOG10(reverbTransmission));

        FMOD_REVERB_CHANNELPROPERTIES prop;	
        
        getReverbProperties(&prop);	

        prop.Direct   = directGain >> 2;    // quarter occlusion for low frequencies
        prop.DirectHF = directGain;         // full occlusion for high frequencies
        prop.Room     = reverbGain;
        
        return setReverbProperties(&prop);
    }
    else
    {
        return setVolume(mParent->mVolume);
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
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelOpenAL::setReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop)
{    
    if (mMode & FMOD_2D)
    {
        return FMOD_ERR_NEEDS3D;
    }

    return FMOD_ERR_UNSUPPORTED;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT ChannelOpenAL::getReverbProperties(FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    if (mMode & FMOD_2D)
    {
        return FMOD_ERR_NEEDS3D;
    }

    return FMOD_ERR_UNSUPPORTED;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 8745da6e manual
// OFFSET: 0x009468C0
FMOD_RESULT ChannelOpenAL::isPlaying(bool *isplaying, bool includethreadlatency)
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
    else
#endif
    if (mDSPResampler)
    {
        int numinputs, count, numfinished = 0;

        if (mDSPResampler->getNumInputs(&numinputs) != FMOD_OK)
        {
            *isplaying = false;
        }
        else
        {
            for (count = 0; count < numinputs; count++)
            {
                DSPI *input;
                bool  finished;

                if (mDSPResampler->getInput(count, &input) != FMOD_OK)
                {
                    break;
                }

                input->getFinished(&finished);
                if (finished)
                {
                    numfinished++;
                }
            }

            *isplaying = (count == numinputs && numfinished != numinputs);
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
    }
    else
    {
        mFlags &= ~(CHANNELREAL_FLAG_ALLOCATED | CHANNELREAL_FLAG_PLAYING);
    }

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
// VERIFY: b13a7f0e auto
// OFFSET: 0x009469E0
FMOD_RESULT ChannelOpenAL::getSpectrum(float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
{
#ifdef FMOD_SUPPORT_GETSPECTRUM
    FMOD_RESULT result;

#ifdef FMOD_STATICFORPLUGINS

    result = FMOD_ERR_UNSUPPORTED;

#else

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

    result = mSystem->getDSPBufferSize(&bufferlength, 0);
    
    position -= windowsize;
    if ((int)position < 0)
    {
        position += length;
    }

//    mUpdateTimeStamp.stampIn();

    result = fft.getSpectrum(buffer, position, length, spectrumarray, windowsize, channeloffset, numchannels, windowtype);

//    mUpdateTimeStamp.stampOut(95);
#endif
    return result;
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
// VERIFY: 404a8c49 manual
// OFFSET: 0x00946B20
FMOD_RESULT ChannelOpenAL::getWaveData(float *wavearray, int numvalues, int channeloffset)
{
    FMOD_RESULT result;

#ifdef FMOD_STATICFORPLUGINS

    result = FMOD_ERR_UNSUPPORTED;

#else

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

#endif
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
FMOD_RESULT ChannelOpenAL::setupChannel()
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
// VERIFY: 3f40420f manual
// OFFSET: 0x00946CB0
FMOD_RESULT ChannelOpenAL::updateChannel()
{
	FMOD_SPEAKERMODE  channelsToSpeakermode[]  = { FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_MONO, FMOD_SPEAKERMODE_STEREO, FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_QUAD, FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_5POINT1, FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_7POINT1, FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_RAW };
	FMOD_RESULT       result                   = FMOD_OK;
    ALint	          processedBuffers         = 0;
    ALuint	          sourceIDs[16]            = {0};

    // This channel doesn't have any sources or it's paused
    // NOTE: If channel started paused, this prevents incrementing the play position until playing
    if (!(mNumSources > 0 && mNumSources < 16) || (mFlags & CHANNELREAL_FLAG_PAUSED))
	{
        return FMOD_OK;
    }

    /*
        Determine how many buffers needs updating
    */
    // Initial fill will update all buffers
    processedBuffers = mOutputOAL->mNumBuffers;
    if (!mInitialFill)
    {
        // Find the minimum number of buffers that all sources for this channel have processed
        for (int i = 0; i < mNumSources; i++)
	    {
		    ALint processed = 0;
		    mOutputOAL->mOALFnTable.alGetSourcei(mSources[i]->sid, AL_BUFFERS_PROCESSED, &processed);
            processedBuffers = min(processed, processedBuffers);
	    }
    }

	/*
		Update the stream buffers that have already played
	*/
	for (int bufferIndex = 0; bufferIndex < processedBuffers; bufferIndex++)
	{
		int           outputChannels   = mNumSources;
		unsigned int  outputSamples    = mOutputOAL->mBufferLength;
		float        *interleavedData  = NULL;

		/*
			Fill buffer with data
		*/
		mSystem->mDSPActive = true;
        mSystem->mDSPTempBuffIndex = 0;

        result = mDSPHead->execute(0, &interleavedData, &outputSamples, outputChannels, &outputChannels, channelsToSpeakermode[outputChannels]);
		if (result != FMOD_OK)
		{
			return result;
		}

		mSystem->mDSPActive = false;

        mDSPHead->resetVisited();

		/*
			Update all sources on this channel
		*/
		for (int sourceIndex = 0; sourceIndex < mNumSources; sourceIndex++)
		{
			if (interleavedData)
			{
				/*
					Deinterleave data
				*/
				for (unsigned int i = 0, j = sourceIndex; i < outputSamples; i++, j += outputChannels)
				{
					mTempBuffer[i] = interleavedData[j];
				}

				/*
					Convert data from internal FMOD float to PCM16 for the OpenAL sources
				*/
				result = DSPI::convert(mBuffer, mTempBuffer, FMOD_SOUND_FORMAT_PCM16, mDSPHead->mDescription.mFormat, outputSamples, 1, 1, 1.0f);
				if (result != FMOD_OK)
				{
					return result;
				}
			}
			else
			{
				/*
					Fill buffer with silence
				*/
				FMOD_memset(mBuffer, 0, mOutputOAL->mBufferLength * sizeof(short));
			}

			/*
				Update source queue
			*/
		    ALuint buffer = 0;
            if (mInitialFill)
            {
	        	// Initial fill will not have any buffers queued, so get the buffer manually
                buffer = mSources[sourceIndex]->bid[bufferIndex];
            }
            else
            {
                // Unqueue the used buffer to be refilled
                mOutputOAL->mOALFnTable.alSourceUnqueueBuffers(mSources[sourceIndex]->sid, 1, &buffer);
            }
			mOutputOAL->mOALFnTable.alBufferData(buffer, AL_FORMAT_MONO16, mBuffer, mOutputOAL->mBufferLength * sizeof(short), 44100);
			mOutputOAL->mOALFnTable.alSourceQueueBuffers(mSources[sourceIndex]->sid, 1, &buffer);
			sourceIDs[sourceIndex] = mSources[sourceIndex]->sid;
		}
	}
    
	/*
		Check for starvation, all buffers may have finished before getting new data (or this could be initial fill)
	*/
	if (processedBuffers == (ALint)mOutputOAL->mNumBuffers)
	{
		mOutputOAL->mOALFnTable.alSourcePlayv(mNumSources, sourceIDs);
	}

    mInitialFill = false;
	return result;
}

}

#endif /* FMOD_SUPPORT_OPENAL */
