#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_STREAMING

#include "fmod_async.h"
#include "fmod_codeci.h"
#include "fmod_channel_stream.h"
#include "fmod_file.h"
#include "fmod_sound_sample.h"
#include "fmod_sound_stream.h"
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
// VERIFY: 46c0ca1d auto
// OFFSET: 0x0091E480
Stream::Stream()
{
    mLastPos          = 0;
    mSubSound         = 0;
    mLoopCountCurrent = -1;
    mBlockSize        = 1;
    mChannel          = 0;
    mInitialPosition  = 0;
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
// VERIFY: 8bcf4140 manual
// OFFSET: 0x0091DE80
FMOD_RESULT Stream::fill(unsigned int offset, unsigned int length)
{
    FMOD_RESULT result = FMOD_OK;
    unsigned int read = 0;

    if (!(mFlags & FMOD_SOUND_FLAG_FINISHED))
    {
        unsigned int len = length;

        do
        {
            unsigned int endpoint, streamlength, size, r, oldoffset;
            Stream *stream = this;

            if (mSubSound)
            {
                int index;
#ifdef FMOD_SUPPORT_SENTENCING
                if (mSubSoundList)
                {
                    index = mSubSoundList[mSubSoundListCurrent];
                }
                else
#endif
                {
                    index = mSubSoundIndex;
                }

                stream = SAFE_CAST(Stream, mSubSound[index]);
                if (!stream)
                {
                    break;
                }

                if (mSubSoundShared)
                {
                    FMOD_CODEC_WAVEFORMAT waveformat;

#ifdef FMOD_SUPPORT_SENTENCING
                    if (mSubSoundList)
                    {
                        stream->mCodec->mDescription.getwaveformat(stream->mCodec, mSubSoundList[mSubSoundListCurrent], &waveformat);
                    }
                    else
#endif
                    {
                        stream->mCodec->mDescription.getwaveformat(stream->mCodec, mSubSoundIndex, &waveformat);
                    }
                    streamlength = waveformat.lengthpcm;
                }
                else
                {
                    streamlength = stream->mLength;
                }
            }
            else
            {
                streamlength = mLength;
            }

            if (mMode & FMOD_LOOP_NORMAL && mLoopCountCurrent)
            {
                endpoint = stream->mLoopStart + stream->mLoopLength - 1;
            }
            else
            {
                if (mSample->mCodec->mFlags & FMOD_CODEC_ACCURATELENGTH)
                {
                    endpoint = streamlength - 1;
                }
                else
                {
                    endpoint = (unsigned int)-1;
                }
            }

            size = len;
            oldoffset = offset;

            if (offset + size > mSample->mLength)
            {
                size = mSample->mLength - offset;
            }

            if (stream->mPosition > endpoint)
            {
                size = 0;
            }
            else if (stream->mPosition + size > endpoint)
            {
                size = (endpoint - stream->mPosition) + 1;
            }

            result = mSample->read(offset, size, &r);
            if (result != FMOD_OK)
            {
                if (result != FMOD_ERR_FILE_EOF)
                {
                    return result;
                }
            }
            else if (!r)
            {
                break;
            }

            read   += r;
            len    -= r;
            offset += r;
            if (oldoffset + r >= mSample->mLength)
            {
                offset = 0;
            }

            stream->mLastPos = stream->mPosition;
            stream->mPosition += r;

            if (stream->mPosition > endpoint || result == FMOD_ERR_FILE_EOF)
            {
                bool finished = true;

#ifdef FMOD_SUPPORT_SENTENCING
                if (mSubSoundList)
                {
                    bool wrapped = false;

                    do
                    {
                        mSubSoundListCurrent++;
                        if (mSubSoundListCurrent >= mSubSoundListNum)
                        {
                            if (!(mMode & FMOD_LOOP_NORMAL && mLoopCountCurrent))
                            {
                                wrapped = true;
                            }
                            mSubSoundListCurrent = 0;
                        }

                        mSubSoundIndex = mSubSoundList[mSubSoundListCurrent];
                        stream = SAFE_CAST(Stream, mSubSound[mSubSoundIndex]);
                    } while (!stream && !wrapped);

                    if (stream)
                    {
                        mSample->mCodec = stream->mCodec;

                        if (!wrapped)
                        {
                            if (mCodec == mSample->mCodec)
                            {
                                result = mSample->seek(mSubSoundIndex, 0);
                            }
                            else
                            {
                                result = mSample->seek(-1, 0);
                            }
                        }

                        stream->mPosition = 0;
                    }

                    finished = wrapped;
                }
#endif

                if (finished)
                {
                    if (!(mMode & FMOD_LOOP_NORMAL && mLoopCountCurrent))
                    {
                        if (stream != this)
                        {
                            mFlags |= FMOD_SOUND_FLAG_FINISHED;
                            mPosition = mLength;
                        }
                        stream->mFlags |= FMOD_SOUND_FLAG_FINISHED;
                        stream->mPosition = streamlength;
                        break;
                    }

                    stream->mPosition = mLoopStart;
                    if (mLength != (unsigned int)-1)
                    {
                        result = mSample->seek(mSubSoundIndex, mLoopStart);
                        if (result != FMOD_OK)
                        {
                            return result;
                        }
                        stream->mPosition = mSample->mPosition;
                    }

                    if (mLoopCountCurrent > 0)
                    {
                        mLoopCountCurrent--;
                    }
                }
            }
            else
            {
                if (!r)
                {
                    break;
                }
            }
        } while (len);
    }

    if (read < length)
    {
        unsigned int size;
        unsigned int len;

        len = length - read;
        do
        {
            size = len;
            if (offset + size > mSample->mLength)
            {
                size = mSample->mLength - offset;
            }

            mSample->clear(offset, size);

            len -= size;
            offset += size;
            if (offset >= mSample->mLength)
            {
                offset = 0;
            }
        } while (len);
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
// VERIFY: bf05cc98 auto
// OFFSET: 0x0091E1D0
FMOD_RESULT Stream::flush()
{
    FMOD_RESULT result;

    result = fill(0, mSample->mLength);
    if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
    {
        return result;
    }

    mFlags &= ~FMOD_SOUND_FLAG_WANTSTOFLUSH;

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
// VERIFY: d3047022 manual
// OFFSET: 0x0091E200
FMOD_RESULT Stream::setPosition(unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result = FMOD_OK;
    bool seekable = true;
    unsigned int endpoint;

    if (postype == FMOD_TIMEUNIT_PCM)
    {
        if (mMode & FMOD_LOOP_OFF)
        {
            endpoint = mLength - 1;
        }
        else
        {
            endpoint = mLoopStart + mLoopLength - 1;
        }

        if (position > endpoint)
        {
            return FMOD_ERR_INVALID_PARAM;
        }
    }

    if (mCodec->mFile)
    {
        seekable = mCodec->mFile->mSeekable;
    }

    mFlags &= ~FMOD_SOUND_FLAG_FINISHED;

    if (!seekable)
    {
        if (mLastPos != 0 || position != 0)
        {
            return FMOD_ERR_FILE_COULDNOTSEEK;
        }
        return result;
    }

#ifdef FMOD_SUPPORT_SENTENCING
    if (mSubSound && mSubSoundList && postype == FMOD_TIMEUNIT_PCM)
    {
        int count;
        unsigned int offset = 0;

        for (count = 0; count < mSubSoundListNum; count++)
        {
            SoundI *subsound = mSubSound[mSubSoundList[count]];

            if (subsound)
            {
                unsigned int length;

                if (!mSubSoundList)
                {
                    return FMOD_ERR_INVALID_PARAM;
                }

                if (mSubSoundShared)
                {
                    FMOD_CODEC_WAVEFORMAT waveformat;

                    result = mCodec->mDescription.getwaveformat(mCodec, mSubSoundList[count], &waveformat);
                    if (result != FMOD_OK)
                    {
                        return result;
                    }
                    length = waveformat.lengthpcm;
                }
                else
                {
                    length = subsound->mLength;
                }

                if (position >= offset && position < offset + length)
                {
                    Stream *substream = SAFE_CAST(Stream, subsound);

                    mSubSoundListCurrent       = count;
                    mSubSoundIndex             = mSubSoundList[count];
                    substream->mSubSoundIndex  = mSubSoundIndex;

                    substream->updateSubSound(mSubSoundIndex, true);
                    substream->setPosition(position - offset, FMOD_TIMEUNIT_PCM);
                    break;
                }

                offset += length;
            }
        }
    }
    else
#endif
    if (mSubSound && postype == FMOD_TIMEUNIT_PCM)
    {
        SoundI *subsound = mSubSound[mSubSoundIndex];

        if (subsound)
        {
            Stream *substream = SAFE_CAST(Stream, subsound);

            result = substream->setPosition(position, FMOD_TIMEUNIT_PCM);
        }
    }
    else
    {
        mCodec->reset();

        result = mCodec->setPosition(mSubSoundIndex, position, postype);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (mSubSoundParent)
        {
            mSubSoundParent->mSubSoundIndex = mSubSoundIndex;
        }
    }

    if (mSample->mPostSetPositionCallback)
    {
        mSample->mPostSetPositionCallback((FMOD_SOUND *)this, mSubSoundIndex, position, postype);
    }

    if (postype != FMOD_TIMEUNIT_MS && postype != FMOD_TIMEUNIT_PCM && postype != FMOD_TIMEUNIT_PCMBYTES)
    {
        position = 0;
    }

    mPosition = position;
    mLastPos  = position;

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
// VERIFY: 8965286a manual
// OFFSET: 0x0091E4D0
FMOD_RESULT Stream::getPosition(unsigned int *position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result;

    if (mOpenState != FMOD_OPENSTATE_READY)
    {
        return FMOD_ERR_NOTREADY;
    }

    if (!position)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (postype == (FMOD_TIMEUNIT)(FMOD_TIMEUNIT_SENTENCE_SUBSOUND | FMOD_TIMEUNIT_BUFFERED))
    {
        *position = mSubSoundListCurrent;
    }
    else if (postype == FMOD_TIMEUNIT_PCM || postype == FMOD_TIMEUNIT_PCMBYTES || postype == FMOD_TIMEUNIT_MS)
    {
        if (postype == FMOD_TIMEUNIT_PCM)
        {
            *position = mLastPos;
        }
        else if (postype == FMOD_TIMEUNIT_MS)
        {
            *position = (unsigned int)((float)mLastPos * mDefaultFrequency * 0.001f);
        }
    }
    else
    {
        result = mCodec->getPosition(position, postype);
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
    
	[SEE_ALSO]
]
*/
// VERIFY: 474b040b auto
// OFFSET: 0x0091E450
FMOD_RESULT Stream::setLoopCount(int loopcount)
{
    if (mOpenState != FMOD_OPENSTATE_READY)
    {
        return FMOD_ERR_NOTREADY;
    }

    mLoopCountCurrent = loopcount;
    mLoopCount = loopcount;

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

}

#endif  /* #ifdef FMOD_SUPPORT_STREAMING */
