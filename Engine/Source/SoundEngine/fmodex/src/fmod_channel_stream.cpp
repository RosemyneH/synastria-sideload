#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_STREAMING

#include "fmod_async.h"
#include "fmod_channel_stream.h"
#include "fmod_codeci.h"
#include "fmod_localcriticalsection.h"
#include "fmod_soundi.h"
#include "fmod_sound_stream.h"
#include "fmod_sound_sample.h"
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
// VERIFY: a7b844bf manual
// OFFSET: 0x0091EF10
ChannelStream::ChannelStream()
{
    mSamplesPlayed = mSamplesWritten = 0;
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
FMOD_RESULT ChannelStream::setRealChannel(ChannelReal *realchan)
{
    mRealChannel[0] = realchan;
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
// VERIFY: 3ed980d0 auto
// OFFSET: 0x0091E5C0
FMOD_RESULT ChannelStream::set2DFreqVolumePanFor3D()
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->set2DFreqVolumePanFor3D();
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
// VERIFY: aa7765f0 auto
// OFFSET: 0x0091E5F0
FMOD_RESULT ChannelStream::moveChannelGroup(ChannelGroupI *oldchannelgroup, ChannelGroupI *newchannelgroup)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->moveChannelGroup(oldchannelgroup, newchannelgroup);
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
// VERIFY: 2ce6069d manual
// OFFSET: 0x0091E630
FMOD_RESULT ChannelStream::alloc()
{
    FMOD_RESULT result;
    Stream *stream = SAFE_CAST(Stream, mSound);
    int count;

    mFlags &= ~(CHANNELREAL_FLAG_STOPPED | CHANNELREAL_FLAG_ENDDELAY);

    mSystem         = stream->mSystem;
    mFinished       = false;
    mLastPCM        = 0;
    mDecodeOffset   = 0;

    if (stream->mSubSoundParent)
    {
        mPosition = ((Stream *)stream->mSubSoundParent)->mInitialPosition;
    }
    else
    {
        mPosition = stream->mInitialPosition;
    }

    mSamplesPlayed  = 0;
    mSamplesWritten = 0;

    mMinFrequency = mRealChannel[0]->mMinFrequency;
    if (mMinFrequency < 100.0f)
    {
        mMinFrequency = 100.0f;
    }
    mMaxFrequency = mRealChannel[0]->mMaxFrequency;

    for (count = 0; count < mNumRealChannels; count++)
    {
        Sample *sample = stream->mSample;

        if (sample->mNumSubSamples)
        {
            sample = sample->mSubSample[count];
        }

        sample->mSubSoundParent = stream;

        mRealChannel[count]->mSound            = sample;
        mRealChannel[count]->mSubChannelIndex  = count;
        mRealChannel[count]->mDSP              = 0;
        mRealChannel[count]->mMode             = sample->mMode;
        mRealChannel[count]->mLoopStart        = sample->mLoopStart;
        mRealChannel[count]->mLoopLength       = sample->mLoopLength;
        mRealChannel[count]->mLoopCount        = -1;
        mRealChannel[count]->mLength           = sample->mLength;
        mRealChannel[count]->mDelayStart       = 0;
        mRealChannel[count]->mDelayEnd         = 0;
        mRealChannel[count]->mParent           = mParent;

        result = mRealChannel[count]->alloc();
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    FMOD_OS_CriticalSection_Enter(mSystem->mStreamListCrit);
    {
        mStreamNode.setData(this);
        mStreamNode.addBefore(&mSystem->mStreamListChannelHead);
    }
    FMOD_OS_CriticalSection_Leave(mSystem->mStreamListCrit);

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
// VERIFY: 73435730 auto
// OFFSET: 0x0091E7D0
FMOD_RESULT ChannelStream::start()
{
    FMOD_RESULT result;
    int count;

    if (!mRealChannel[0])
    {
        return FMOD_ERR_INVALID_HANDLE;
    }

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->start();
        if (result != FMOD_OK)
        {
            return result;
        }

        mRealChannel[count]->mFlags &= ~CHANNELREAL_FLAG_STOPPED;
        mRealChannel[count]->mFlags &= ~CHANNELREAL_FLAG_ALLOCATED;
        mRealChannel[count]->mFlags &= ~CHANNELREAL_FLAG_ENDDELAY;
        mRealChannel[count]->mFlags |=  CHANNELREAL_FLAG_PLAYING;
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
// VERIFY: 613ff4cc auto
// OFFSET: 0x0091E830
FMOD_RESULT ChannelStream::update(int delta)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->update(delta);
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
// VERIFY: 41292d28 manual
// OFFSET: 0x0091EF50
FMOD_RESULT ChannelStream::updateStream()
{
    FMOD_RESULT result = FMOD_OK;
    Sample *sample;
    Stream *stream;
    unsigned int pcm = 0;
    int delta;
	LocalCriticalSection crit(mSystem->mStreamRealchanCrit, true);

    stream = SAFE_CAST(Stream, mSound);
    if (!stream)
    {
        return FMOD_OK;
    }

    sample = stream->mSample;

    if (stream->mOpenState != FMOD_OPENSTATE_READY)
    {
        return FMOD_ERR_NOTREADY;
    }

    stream->mFlags |= FMOD_SOUND_FLAG_PLAYED;

    if (mFinished)
    {
        stream->mFlags |= FMOD_SOUND_FLAG_FINISHED;
        return FMOD_OK;
    }

    if (mRealChannel[0])
    {
        result = mRealChannel[0]->updateStream();
        if (result != FMOD_OK)
        {
            return result;
        }

        result = mRealChannel[0]->getPosition(&pcm, FMOD_TIMEUNIT_PCM);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    crit.leave();

    while ((mSamplesPlayed > mSamplesWritten && mSamplesPlayed - mSamplesWritten >= (unsigned int)stream->mBlockSize) ||
           (mSamplesPlayed < mSamplesWritten && mSamplesWritten - mSamplesPlayed >= (unsigned int)stream->mBlockSize))
    {
        unsigned int len, endpoint;

        crit.enter();
        {
            len = stream->mBlockSize;
            if (mDecodeOffset > sample->mLength)
            {
                len = 0;
            }
            else if (mDecodeOffset + len > sample->mLength)
            {
                len = sample->mLength - mDecodeOffset;
            }
        }
        crit.leave();

        result = stream->fill(mDecodeOffset, len);

        crit.enter();

		if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
		{
			int count;

			for (count = 0; count < mNumRealChannels; count++)
			{
				if (mRealChannel[count])
				{
					mRealChannel[count]->setPaused(true);
				}
			}

			stream->mOpenState = FMOD_OPENSTATE_ERROR;
			if (stream->mAsyncData)
			{
				stream->mAsyncData->mResult = result;
			}

			mFinished = true;
			return result;
		}

		stream->mFlags |= FMOD_SOUND_FLAG_WANTSTOFLUSH;

		mDecodeOffset += len;
		if (mDecodeOffset >= sample->mLength)
		{
			mDecodeOffset -= sample->mLength;
		}

		mSamplesWritten += len;

		if (stream->mLength < mLoopStart + mLoopLength)
		{
			mLoopLength = stream->mLength - mLoopStart;
		}

		if (mMode & FMOD_LOOP_NORMAL && mLoopCount)
		{
			endpoint = mLoopStart + mLoopLength - 1;
		}
		else
		{
			endpoint = stream->mLength - 1;
		}

        if (mPosition > endpoint)
        {
            len = 0;
        }
        else if (mPosition + len > endpoint)
        {
            len = endpoint - mPosition + 1;
        }

    	mPosition += len;

		if (mPosition > endpoint)
		{
			if ((mMode & FMOD_LOOP_NORMAL && mLoopCount) || stream->mLength == (unsigned int)-1)
			{
				mPosition = mLoopStart;
				if (mLoopCount > 0)
				{
					mLoopCount--;
				}
			}
			else if (stream->mFlags & FMOD_SOUND_FLAG_FINISHED)
			{
				int count;

				mPosition = stream->mLength;

				for (count = 0; count < mNumRealChannels; count++)
				{
					if (mRealChannel[count])
					{
						mRealChannel[count]->setPaused(true);
					}
				}

				mFinished = true;
                break;
			}
		}

		crit.leave();
    }

    delta = pcm - mLastPCM;
    if (delta < 0)
    {
        delta += sample->mLength;
        if (delta < 0)
        {
            delta = 0;
        }
    }

    mSamplesPlayed += delta;
    mLastPCM = pcm;

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
// VERIFY: 2ce93c67 auto
// OFFSET: 0x0091E870
FMOD_RESULT ChannelStream::setMode(FMOD_MODE mode)
{
    FMOD_RESULT result;
    int count;

    result = ChannelReal::setMode(mode);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mSound->setMode(mode);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->setMode(mode & ~(FMOD_LOOP_OFF | FMOD_LOOP_NORMAL | FMOD_LOOP_BIDI));
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
// VERIFY: 2df34548 auto
// OFFSET: 0x0091E8E0
FMOD_RESULT ChannelStream::stop(bool ignoreenddelay, bool resetstate, bool force)
{
    FMOD_RESULT result = FMOD_OK;
    FMOD_UINT_NATIVE threadid;
    int count;

    mFinished = true;

    FMOD_OS_Thread_GetCurrentID(&threadid);

    if (!(mMode & FMOD_NONBLOCKING) || threadid != mSystem->mMainThreadID)
    {
        FMOD_OS_CriticalSection_Enter(mSystem->mStreamUpdateCrit);
    }

    FMOD_OS_CriticalSection_Enter(mSystem->mStreamRealchanCrit);

    for (count = 0; count < mNumRealChannels; count++)
    {
        if (mRealChannel[count])
        {
            if (resetstate)
            {
                mRealChannel[count]->mFlags &= ~CHANNELREAL_FLAG_IN_USE;
                mRealChannel[count]->mFlags &= ~CHANNELREAL_FLAG_ALLOCATED;
                mRealChannel[count]->mFlags &= ~CHANNELREAL_FLAG_PLAYING;
                mRealChannel[count]->mFlags &= ~CHANNELREAL_FLAG_PAUSED;
                mRealChannel[count]->mFlags &= ~CHANNELREAL_FLAG_PAUSEDFORSETPOS;
                mRealChannel[count]->mFlags &= ~CHANNELREAL_FLAG_ENDDELAY;
                mRealChannel[count]->mFlags |=  CHANNELREAL_FLAG_STOPPED;
            }

            if (mRealChannel[count]->mDelayEnd && !ignoreenddelay)
            {
                mFlags |= CHANNELREAL_FLAG_ENDDELAY;
                mRealChannel[count]->mFlags |= CHANNELREAL_FLAG_ENDDELAY;
            }

            result = mRealChannel[count]->stop(ignoreenddelay, true, force);

            mRealChannel[count]->mSound  = 0;
            mRealChannel[count]->mDSP    = 0;
            mRealChannel[count]->mParent = 0;
            mRealChannel[count]          = 0;
        }
    }

    FMOD_OS_CriticalSection_Leave(mSystem->mStreamRealchanCrit);

    if (!(mMode & FMOD_NONBLOCKING) || threadid != mSystem->mMainThreadID)
    {
        FMOD_OS_CriticalSection_Leave(mSystem->mStreamUpdateCrit);
    }

    FMOD_OS_CriticalSection_Enter(mSystem->mStreamListCrit);
    {
        if (mSystem->mStreamListChannelNext == &mStreamNode)
        {
            mSystem->mStreamListChannelNext = mStreamNode.getNext();
        }

        mStreamNode.removeNode();
        mStreamNode.setData(0);
    }
    FMOD_OS_CriticalSection_Leave(mSystem->mStreamListCrit);

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
// VERIFY: 8aa9f23e manual
// OFFSET: 0x0091EA90
FMOD_RESULT ChannelStream::stop(bool ignoreenddelay, bool resetstate)
{
    return stop(ignoreenddelay, resetstate);
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
// VERIFY: e0830747 auto
// OFFSET: 0x0091EAA0
FMOD_RESULT ChannelStream::setPaused(bool paused)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->setPaused(paused);
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
// VERIFY: 8d444d4e auto
// OFFSET: 0x0091EAE0
FMOD_RESULT ChannelStream::setVolume(float volume)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    if (mNumRealChannels > 1 && mParent->mLevels)
    {
        for (count = 0; count < mNumRealChannels; count++)
        {
            result = mRealChannel[count]->updateSpeakerLevels(volume);
        }
    }
    else
    {
        for (count = 0; count < mNumRealChannels; count++)
        {
            result = mRealChannel[count]->setVolume(volume);
        }
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
// VERIFY: 53886873 auto
// OFFSET: 0x0091EB60
FMOD_RESULT ChannelStream::setFrequency(float frequency)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->setFrequency(frequency);
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
// VERIFY: 3e7a3457 auto
// OFFSET: 0x0091EBA0
FMOD_RESULT ChannelStream::setPan(float pan, float fbpan)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        if (mNumRealChannels > 1 && (mNumRealChannels == 2 || (mSound && mSound->mDefaultChannelMask == SPEAKER_ALLSTEREO)))
        {
            pan = (count & 1) ? 1.0f : -1.0f;
        }

        result = mRealChannel[count]->setPan(pan, fbpan);
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
// VERIFY: 8e0ae62a auto
// OFFSET: 0x0091EC10
FMOD_RESULT ChannelStream::setDelay(unsigned int startdelay, unsigned int enddelay)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->setDelay(startdelay, enddelay);
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
// VERIFY: e1e7f957 auto
// OFFSET: 0x0091EC50
FMOD_RESULT ChannelStream::setSpeakerMix(float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->setSpeakerMix(frontleft, frontright, center, lfe, backleft, backright, sideleft, sideright);
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
// VERIFY: 76ca593c auto
// OFFSET: 0x0091ECC0
FMOD_RESULT ChannelStream::setSpeakerLevels(int speaker, float *levels, int numlevels)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->setSpeakerLevels(speaker, levels, numlevels);
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
// VERIFY: b82a04a1 manual
// OFFSET: 0x0091F6F0
FMOD_RESULT ChannelStream::setPosition(unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result = FMOD_OK;
    Stream *stream;
    bool paused = false;
    FMOD_UINT_NATIVE threadid;
    int count;

    if (mFlags & CHANNELREAL_FLAG_STOPPED)
    {
        return FMOD_ERR_INVALID_HANDLE;
    }

    stream = SAFE_CAST(Stream, mSound);
    if (!stream)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (postype == FMOD_TIMEUNIT_MS)
    {
        position = (unsigned int)((float)position * stream->mDefaultFrequency * 0.001f);
        postype = FMOD_TIMEUNIT_PCM;
    }
    else if (postype == FMOD_TIMEUNIT_PCMBYTES)
    {
        SoundI::getSamplesFromBytes(position, &position, stream->mChannels, stream->mFormat);
        postype = FMOD_TIMEUNIT_PCM;
    }
#ifdef FMOD_SUPPORT_SENTENCING
    else if (postype == FMOD_TIMEUNIT_SENTENCE)
    {
        stream->mSubSoundIndex = stream->mSubSoundList[position];
        postype = FMOD_TIMEUNIT_MS;
        position = 0;
    }
#endif

    if (postype == FMOD_TIMEUNIT_PCM)
    {
        if (stream->mSubSoundIndex == stream->mCodec->mSubSoundIndex && !(stream->mFlags & FMOD_SOUND_FLAG_WANTSTOFLUSH) && !(mFlags & CHANNELREAL_FLAG_PLAYING))
        {
            return result;
        }
    }

    FMOD_OS_Thread_GetCurrentID(&threadid);

    mRealChannel[0]->getPaused(&paused);

    FMOD_OS_CriticalSection_Enter(mSystem->mStreamUpdateCrit);

    for (count = 0; count < mNumRealChannels; count++)
    {
        mRealChannel[count]->setPaused(true);
    }

    result = stream->setPosition(position, postype);
    if (result == FMOD_OK)
    {
        for (count = 0; count < mNumRealChannels; count++)
        {
            mRealChannel[count]->setPosition(0, FMOD_TIMEUNIT_PCM);
        }

        mLastPCM        = 0;
        mDecodeOffset   = 0;
        mPosition       = position;
        mSamplesWritten = 0;
        mSamplesPlayed  = 0;

        result = stream->flush();
    }

    for (count = 0; count < mNumRealChannels; count++)
    {
        mRealChannel[count]->setPaused(paused);
    }

    FMOD_OS_CriticalSection_Leave(mSystem->mStreamUpdateCrit);

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
// VERIFY: 9c7eca8f manual
// OFFSET: 0x0091F260
FMOD_RESULT ChannelStream::getPosition(unsigned int *position, FMOD_TIMEUNIT postype)
{
    Stream *stream;
    bool getsubsoundtime = false;
    unsigned int realpos, pos;
    int diff, count;

    if (!position)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    stream = SAFE_CAST(Stream, mSound);
    if (!stream)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (postype == FMOD_TIMEUNIT_SENTENCE_MS)
    {
        postype = FMOD_TIMEUNIT_MS;
        getsubsoundtime = true;
    }
    else if (postype == FMOD_TIMEUNIT_SENTENCE_PCM)
    {
        postype = FMOD_TIMEUNIT_PCM;
        getsubsoundtime = true;
    }
    else if (postype == FMOD_TIMEUNIT_SENTENCE_PCMBYTES)
    {
        postype = FMOD_TIMEUNIT_PCMBYTES;
        getsubsoundtime = true;
    }
    else if (postype == FMOD_TIMEUNIT_SENTENCE || postype == FMOD_TIMEUNIT_SENTENCE_SUBSOUND)
    {
        getsubsoundtime = true;
    }

    if (getsubsoundtime && !stream->mSubSoundList)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (postype != FMOD_TIMEUNIT_MS &&
        postype != FMOD_TIMEUNIT_PCM &&
        postype != FMOD_TIMEUNIT_PCMBYTES &&
        postype != FMOD_TIMEUNIT_SENTENCE &&
        postype != FMOD_TIMEUNIT_SENTENCE_SUBSOUND)
    {
        return stream->getPosition(position, postype);
    }

    if (mRealChannel[0]->getPosition(&realpos, FMOD_TIMEUNIT_PCM) != FMOD_OK)
    {
        realpos = 0;
    }

    diff = realpos - mSamplesWritten % stream->mSample->mLength;
    if (diff < 0)
    {
        diff += stream->mSample->mLength;
    }

    pos = mPosition;

    while (diff)
    {
        unsigned int endpoint, step;

        if (mMode & FMOD_LOOP_NORMAL && mLoopCount)
        {
            endpoint = mLoopStart + mLoopLength - 1;
        }
        else
        {
            endpoint = mSound->mLength - 1;
        }

        step = diff;
        if (endpoint - pos + 1 < (unsigned int)diff)
        {
            step = endpoint - pos + 1;
        }

        pos += step;
        if (pos > endpoint)
        {
            if (stream->mMode & FMOD_LOOP_NORMAL && mLoopCount)
            {
                pos = mLoopStart;
            }
            else
            {
                pos = mSound->mLength;
            }
        }

        if (!step)
        {
            break;
        }

        diff -= step;
    }

    count = 0;

#ifdef FMOD_SUPPORT_SENTENCING
    if (getsubsoundtime)
    {
        while (count < mSound->mSubSoundListNum)
        {
            unsigned int lengthpcm;

            if (mSound->mSubSoundShared)
            {
                FMOD_RESULT result;
                FMOD_CODEC_WAVEFORMAT waveformat;

                result = stream->mCodec->mDescription.getwaveformat(stream->mCodec, stream->mSubSoundList[count], &waveformat);
                if (result != FMOD_OK)
                {
                    return result;
                }
                lengthpcm = waveformat.lengthpcm;
            }
            else
            {
                lengthpcm = mSound->mSubSound[mSound->mSubSoundList[count]]->mLength;
            }

            if (pos < lengthpcm)
            {
                break;
            }

            pos -= lengthpcm;
            count++;
        }
    }
#endif

    if (postype == FMOD_TIMEUNIT_SENTENCE)
    {
        *position = count;
    }
#ifdef FMOD_SUPPORT_SENTENCING
    else if (postype == FMOD_TIMEUNIT_SENTENCE_SUBSOUND)
    {
        *position = mSound->mSubSoundList[count];
    }
#endif
    else if (postype == FMOD_TIMEUNIT_PCM)
    {
        *position = pos;
    }
    else if (postype == FMOD_TIMEUNIT_PCMBYTES)
    {
        SoundI::getBytesFromSamples(pos, position, mSound->mChannels, mSound->mFormat);
    }
    else if (postype == FMOD_TIMEUNIT_MS)
    {
        *position = (unsigned int)((float)pos / mSound->mDefaultFrequency * 1000.0f);
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
// VERIFY: b6c4e6dd auto
// OFFSET: 0x0091ED00
FMOD_RESULT ChannelStream::setLoopPoints(unsigned int loopstart, unsigned int looplength)
{
    if (ChannelReal::setLoopPoints(loopstart, looplength) == FMOD_OK)
    {
        mSound->setLoopPoints(loopstart, FMOD_TIMEUNIT_PCM, loopstart + looplength - 1, FMOD_TIMEUNIT_PCM);
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
// VERIFY: e8a78ba3 auto
// OFFSET: 0x0091ED40
FMOD_RESULT ChannelStream::setLoopCount(int loopcount)
{
    if (ChannelReal::setLoopCount(loopcount) == FMOD_OK)
    {
        mSound->setLoopCount(loopcount);
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
// VERIFY: d23481fd auto
// OFFSET: 0x0091ED70
FMOD_RESULT ChannelStream::set3DAttributes()
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->set3DAttributes();
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
// VERIFY: c920e37f auto
// OFFSET: 0x0091EDA0
FMOD_RESULT ChannelStream::set3DMinMaxDistance()
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->set3DMinMaxDistance();
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
// VERIFY: 6a1a6a93 auto
// OFFSET: 0x0091EDD0
FMOD_RESULT ChannelStream::set3DOcclusion(float directocclusion, float reverbocclusion)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->set3DOcclusion(directocclusion, reverbocclusion);
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
// VERIFY: 98e75020 auto
// OFFSET: 0x0091EE10
FMOD_RESULT ChannelStream::setReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels; count++)
    {
        result = mRealChannel[count]->setReverbProperties(prop);
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
// VERIFY: 4425a2f7 auto
// OFFSET: 0x0091EE50
FMOD_RESULT ChannelStream::getReverbProperties(FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    FMOD_RESULT result = FMOD_OK;
    int count;

    for (count = 0; count < mNumRealChannels && count < 1; count++)
    {
        result = mRealChannel[count]->getReverbProperties(prop);
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
// VERIFY: 69ac7322 auto
// OFFSET: 0x0091EE90
FMOD_RESULT ChannelStream::isPlaying(bool *isplaying, bool includethreadlatency)
{
    *isplaying = !mFinished;

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
// VERIFY: 08775700 manual
// OFFSET: 0x0091EEB0
FMOD_RESULT ChannelStream::getSpectrum(float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
{
    return mRealChannel[0]->getSpectrum(spectrumarray, numvalues, channeloffset, windowtype);
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
// VERIFY: 5267b939 manual
// OFFSET: 0x0091EED0
FMOD_RESULT ChannelStream::getWaveData(float *wavearray, int numvalues, int channeloffset)
{
    return mRealChannel[0]->getWaveData(wavearray, numvalues, channeloffset);
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
// VERIFY: 51d27768 manual
// OFFSET: 0x0091EEF0
FMOD_RESULT ChannelStream::getDSPHead(DSPI **dsp)
{
    return mRealChannel[0]->getDSPHead(dsp);
}

}

#endif
