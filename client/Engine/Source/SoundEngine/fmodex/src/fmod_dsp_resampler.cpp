#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_channel_software.h"
#include "fmod_dsp_codec.h"
#include "fmod_dsp_resampler_nointerp.h"
#include "fmod_dsp_resampler_linear.h"
#include "fmod_dsp_resampler_cubic.h"
#include "fmod_dsp_resampler_spline.h"
#include "fmod_dsp_resampler.h"
#include "fmod_sample_software.h"

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
// VERIFY: 587870c6 auto
// OFFSET: 0x0091C460
DSPResampler::DSPResampler()
{
    mResampleBufferMemory    = 0;
    mResampleBuffer          = 0;
    mTargetFrequency         = 0;
    mSpeed.mValue            = 0;
    mResampleBufferPos       = 0;
    mResamplePosition.mValue = 0;
    mPosition.mValue         = 0;
    mOverflowLength          = FMOD_DSP_RESAMPLER_OVERFLOWLENGTH;
    mResampleFinishPos       = (unsigned int)-1;
    mFill                    = 2;
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
// VERIFY: 6dcf8b0b auto
// OFFSET: 0x0091C360
FMOD_RESULT DSPResampler::release(bool freethis)
{
    DSPI::release(false);

    if (mResampleBufferMemory)
    {
        FMOD_Memory_Free(mResampleBufferMemory);
        mResampleBufferMemory = 0;
    }

    if (freethis)
    {
        FMOD_Memory_Free(this);
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
// VERIFY: d804c55c manual
// OFFSET: 0x0091C4D0
FMOD_RESULT DSPResampler::alloc(FMOD_DSP_DESCRIPTION_EX *description)
{
    FMOD_RESULT result;
    int channels;
    unsigned int bytespersample;

    result = DSPI::alloc(description);
    if (result != FMOD_OK)
    {
        return result;
    }

    mSystem->getSoftwareFormat(&mTargetFrequency, 0, 0, 0, 0, 0);

    if (description->mResamplerBlockLength)
    {
        mResampleBlockLength = description->mResamplerBlockLength;
        channels             = description->channels;
    }
    else
    {
        result = mSystem->getDSPBufferSize(&mResampleBlockLength, 0);
        if (result != FMOD_OK)
        {
            return result;
        }
        channels = mSystem->mMaxInputChannels;
    }

    mResampleBufferLength = mResampleBlockLength * 2;   /* *2 = double buffer */

    if (mDescription.mFormat == FMOD_SOUND_FORMAT_NONE)
    {
        mDescription.mFormat = FMOD_SOUND_FORMAT_PCMFLOAT;
    }

    SoundI::getBytesFromSamples(1, &bytespersample, channels, mDescription.mFormat);

    if (0)
    {
    }
    #ifdef FMOD_SUPPORT_DSPCODEC
    #ifdef FMOD_SUPPORT_MPEG
    else if (mDescription.mCategory == FMOD_DSP_CATEGORY_DSPCODECMPEG)
    {
        DSPCodecMPEG *dspcodecmpeg = SAFE_CAST(DSPCodecMPEG, this);
        mResampleBuffer = (char *)FMOD_ALIGNPOINTER(dspcodecmpeg->mResampleBufferMemory, 16);
    }
        #ifdef FMOD_SUPPORT_XMA
    else if (mDescription.mCategory == FMOD_DSP_CATEGORY_DSPCODECXMA)
    {
        DSPCodecXMA *dspcodecxma = SAFE_CAST(DSPCodecXMA, this);
        mResampleBuffer = (char *)FMOD_ALIGNPOINTER(dspcodecxma->mResampleBufferMemory, 16);
    }
    #endif
    #ifdef FMOD_SUPPORT_IMAADPCM
    else if (mDescription.mCategory == FMOD_DSP_CATEGORY_DSPCODECADPCM)
    {
        DSPCodecADPCM *dspcodecadpcm = SAFE_CAST(DSPCodecADPCM, this);
        mResampleBuffer = (char *)FMOD_ALIGNPOINTER(dspcodecadpcm->mResampleBufferMemory, 16);
    }
    #endif
    #ifdef FMOD_SUPPORT_RAW
    else if (mDescription.mCategory == FMOD_DSP_CATEGORY_DSPCODECRAW)
    {
        DSPCodecRaw *dspcodecraw = SAFE_CAST(DSPCodecRaw, this);
        mResampleBuffer = (char *)FMOD_ALIGNPOINTER(dspcodecraw->mResampleBufferMemory, 16);
    }
    #endif
    #endif  /* FMOD_SUPPORT_DSPCODEC */
    else
    {
        mResampleBufferMemory = FMOD_Memory_Calloc(((mResampleBufferLength + (mOverflowLength * 4)) * bytespersample) + 16);
        if (!mResampleBufferMemory)
        {
            return FMOD_ERR_MEMORY;
        }
        mResampleBuffer = (char *)FMOD_ALIGNPOINTER(mResampleBufferMemory, 16);
    }

    mResampleBuffer = (char *)mResampleBuffer + (mOverflowLength * bytespersample);
    mResampleBuffer = (char *)FMOD_ALIGNPOINTER(mResampleBuffer, 16);

    mResampleBufferPos       = 0;
    mResampleFinishPos       = (unsigned int)-1;
    mFill                    = 2;
    mPosition.mValue         = 0;
    mResamplePosition.mValue = 0;

    return FMOD_OK;
}

#endif

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
// VERIFY: 57b49e63 manual
// OFFSET: 0x0091C710
FMOD_RESULT DSPResampler::execute(float *inbuffer, float **outbuffer, unsigned int *length, int inchannels, int *outchannels, FMOD_SPEAKERMODE speakermode)
{
    FMOD_RESULT  result = FMOD_OK;
    unsigned int bytespersample;
    int          len, readoffset;
    FMOD_SINT64P speed;
    float       *buffer;

    if (mFlags & FMOD_DSP_FLAG_FINISHED)
    {
        return FMOD_OK;
    }

    mFlags &= ~FMOD_DSP_FLAG_IDLE;

    SoundI::getBytesFromSamples(1, &bytespersample, mDescription.channels, mDescription.mFormat);

    if (mFlags & FMOD_DSP_FLAG_VISITED)
    {
        mFlags &= ~FMOD_DSP_FLAG_IDLE;

        *outbuffer   = mBuffer;      /* Return the already processed buffer */
        *outchannels = mDescription.channels;
        return FMOD_OK;
    }

    len        = *length;
    readoffset = 0;
    buffer     = mBuffer ? mBuffer : inbuffer;

    speed.mValue = mSpeed.mValue;

    do
    {
        unsigned int        rlength;
        FMOD_RESAMPLER_END  endflag = FMOD_RESAMPLER_END_MIXBUFFER;

        /*
            Fill the double buffer if nescessary.
        */
        while (mFill)
        {
            unsigned int   readlen, oldresamplebufferpos;
            void          *resamplebuff;

            oldresamplebufferpos = mResampleBufferPos;
            readlen              = mResampleBlockLength;
            resamplebuff         = (char *)mResampleBuffer + (mResampleBufferPos * bytespersample);

            if ((int)mResampleFinishPos == -1)
            {
                instance = (FMOD_DSP *)this;

                /*
                    Read from the dsp unit's read function.  Bring a chunk of data in to fill the back buffer of the resampler double-buffer.
                */
                result = mDescription.read((FMOD_DSP_STATE *)this, 0, (float *)resamplebuff, readlen, mDescription.channels, mDescription.channels);
            }
            else
            {
                FMOD_memset(resamplebuff, 0, readlen * mDescription.channels * sizeof(signed short));
                result = FMOD_OK;
            }

            mResampleBufferPos += readlen;
            if (mResampleBufferPos >= mResampleBufferLength)
            {
                mResampleBufferPos = 0;
            }

            if ((int)mResampleFinishPos == -1 && result != FMOD_OK)
            {
                mResampleFinishPos = mResampleBufferPos;
                if (!mResampleFinishPos)
                {
                    mResampleFinishPos = mResampleBufferLength;
                }
            }

            /*
                Copy the first samples to the end of the buffer so when interpolation does a read past the end, the data will be valid.
            */
            if (!oldresamplebufferpos)
            {
                unsigned int count;

                for (count=0; count < mOverflowLength * 2 * bytespersample; count++)
                {
                    ((unsigned char *)mResampleBuffer)[(mResampleBufferLength * bytespersample) + count] = ((unsigned char *)mResampleBuffer)[count];
                }
            }

            mFill--;
        }

        /*
            Work out what is going to end first.  The source data or the output?
        */
        rlength = len;     /* Start it off defaulting to the amount requested */

        if (speed.mValue > 0x100)
        {
            FMOD_UINT64P mixesleft, fracleft;
            int nextpos;
            bool endofsound = false;

            nextpos  = mResamplePosition.mHi - mOverflowLength;
            nextpos /= (int)mResampleBlockLength;
            nextpos ++;
            nextpos *= mResampleBlockLength;
            nextpos += mOverflowLength;

            mixesleft.mHi = nextpos;
            mixesleft.mLo = 0;
			mixesleft.mValue -= mResamplePosition.mValue;

            if (mResampleFinishPos != (unsigned int)-1)
            {
                FMOD_UINT64P mixeslefttofinish;

                mixeslefttofinish.mHi = mResampleFinishPos;
                mixeslefttofinish.mLo = 0;
                mixeslefttofinish.mValue -= mResamplePosition.mValue;

                if (mixeslefttofinish.mValue <= mixesleft.mValue)
                {
                    mixesleft.mValue = mixeslefttofinish.mValue;
                    endofsound = true;
                }
            }

			fracleft.mValue = mixesleft.mValue % speed.mValue;
			mixesleft.mValue /= speed.mValue;

            if (fracleft.mValue)						/* round the count up (this could be done better) */
            {
				mixesleft.mValue++;
            }

			if (mixesleft.mValue <= rlength)
    		{
                if (endofsound)
                {
                    endflag = FMOD_RESAMPLER_END_SOUND;
                }
                else
                {
                    endflag = FMOD_RESAMPLER_END_RESAMPLEBUFFER;
                }
				rlength = mixesleft.mLo;
			}
        }

        /*
            Resample the src buffer into the destination.
        */
        if (speed.mHi == 1 && speed.mLo == 0)
        {
            FMOD_Resampler_NoInterp(buffer + (readoffset * mDescription.channels), rlength, mResampleBuffer, mDescription.mFormat, &mResamplePosition, &speed, mDescription.channels);
        }
        else
        {
            switch (mSystem->mResampleMethod)
            {
                case FMOD_DSP_RESAMPLER_NOINTERP:
                {
                    FMOD_Resampler_NoInterp(buffer + (readoffset * mDescription.channels), rlength, mResampleBuffer, mDescription.mFormat, &mResamplePosition, &speed, mDescription.channels);
                    break;
                }
                case FMOD_DSP_RESAMPLER_LINEAR:
                {
                    FMOD_Resampler_Linear(buffer + (readoffset * mDescription.channels), rlength, mResampleBuffer, mDescription.mFormat, &mResamplePosition, &speed, mDescription.channels);
                    break;
                }
                case FMOD_DSP_RESAMPLER_CUBIC:
                {
                    FMOD_Resampler_Cubic(buffer + (readoffset * mDescription.channels), rlength, mResampleBuffer, mDescription.mFormat, &mResamplePosition, &speed, mDescription.channels);
                    break;
                }
                case FMOD_DSP_RESAMPLER_SPLINE:
                {
                    FMOD_Resampler_Spline(buffer + (readoffset * mDescription.channels), rlength, mResampleBuffer, mDescription.mFormat, &mResamplePosition, &speed, mDescription.channels);
                    break;
                }
                default:
                {
                    FMOD_Resampler_Linear(buffer + (readoffset * mDescription.channels), rlength, mResampleBuffer, mDescription.mFormat, &mResamplePosition, &speed, mDescription.channels);
                    break;
                }
            }
        }

        if (mResamplePosition.mHi >= (mResampleBufferLength + mOverflowLength))
        {
            mResamplePosition.mHi -= mResampleBufferLength;
        }

        len        -= rlength;
        readoffset += rlength;

        if (endflag == FMOD_RESAMPLER_END_SOUND)
        {
            FMOD_memset(buffer + (readoffset * mDescription.channels), 0, len * mDescription.channels * sizeof(float));
            mFlags |= FMOD_DSP_FLAG_FINISHED;
            break;
        }
        else if (endflag == FMOD_RESAMPLER_END_RESAMPLEBUFFER)
        {
            mFill++;
        }
    } while (len > 0);

    /*
        Update mPosition and execute any loop / end logic.
        THIS INFO IS FOR DISPLAY ONLY.
    */
    {
        unsigned int endpoint;

        if (mLength < mLoopStart + mLoopLength)
        {
            mLoopLength = mLength - mLoopStart;
        }

        if (mMode & FMOD_LOOP_NORMAL && mLoopCount)
        {
            endpoint = mLoopStart + mLoopLength - 1;
        }
        else
        {
            endpoint = mLength - 1;
        }

        mPosition.mValue += speed.mValue * *length;

        if (mPosition.mHi > endpoint)
        {
            if ((mMode & FMOD_LOOP_NORMAL && mLoopCount) || mLength == (unsigned int)-1)  /* 0xFFFFFFFF should mean it is an infinite netstream */
            {
                mPosition.mHi -= mLoopLength;
            }
            else
            {
                mPosition.mHi = mLength;
            }
        }
    }

    *outbuffer   = buffer;
    *outchannels = mDescription.channels;

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
FMOD_RESULT DSPResampler::addInput(DSPI *target)
{
    return FMOD_ERR_DSP_CONNECTION;
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
// VERIFY: f21ecc2b auto
// OFFSET: 0x0091C3C0
FMOD_RESULT DSPResampler::setFrequency(float frequency)
{
    mFrequency = frequency;
    mSpeed.mValue = (FMOD_SINT64)(frequency / mTargetFrequency * 4294967296.0f);

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
FMOD_RESULT DSPResampler::getFrequency(float *frequency)
{
    if (!frequency)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *frequency = mFrequency;

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
// VERIFY: 57248c0a auto
// OFFSET: 0x0091C400
FMOD_RESULT DSPResampler::setPosition(unsigned int position, bool processinputs)
{
    FMOD_RESULT result;

    result = DSPI::setPosition(position, processinputs);
    if (result != FMOD_OK)
    {
        return result;
    }

    mResampleBufferPos       = 0;
    mResampleFinishPos       = (unsigned int)-1;
    mFill                    = 2;
    mResamplePosition.mValue = 0;
    mPosition.mHi            = position;
    mPosition.mLo            = 0;

    return FMOD_OK;
}


}

#endif
