#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_dsp_resampler_nointerp.h"
#include "fmod_dsp_resampler_linear.h"
#include "fmod_dsp_resampler_cubic.h"
#include "fmod_dsp_resampler_spline.h"
#include "fmod_dsp_resampler_multiinput.h"
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
// VERIFY: 4fa7390d manual
// OFFSET: 0x009297E0
FMOD_RESULT DSPResamplerMultiInput::execute(float *inbuffer, float **outbuffer, unsigned int *length, int inchannels, int *outchannels, FMOD_SPEAKERMODE speakermode)
{
    FMOD_RESULT   result   = FMOD_OK;
    int           channels = 0;
    float        *readbuffer;

    if (mFlags & FMOD_DSP_FLAG_FINISHED)
    {
        return FMOD_OK;
    }

    mFlags &= ~FMOD_DSP_FLAG_IDLE;

    if (mFlags & FMOD_DSP_FLAG_VISITED)
    {
        readbuffer = mBuffer;        /* Return the already processed buffer */
    }
    else
    {
        int           inlength       = *length;
        int           readoffset     = 0;
        int           tempbuffindex  = mSystem->mDSPTempBuffIndex;
        FMOD_SINT64P  speed;

        readbuffer   = mBuffer;
        speed.mValue = mSpeed.mValue;

        if (!mBuffer)
        {
            return FMOD_ERR_INTERNAL;
        }

        do
        {
            unsigned int        rlength;
            FMOD_RESAMPLER_END  endflag = FMOD_RESAMPLER_END_MIXBUFFER;

            while (mFill)
            {
                unsigned int  oldresamplebufferpos = mResampleBufferPos;
                float        *resamplebuff         = (float *)mResampleBuffer + (mResampleBufferPos * mResampleBufferChannels);
                unsigned int  readlen              = mResampleBlockLength;
                float        *outbuff;
                int           outch;

                resetVisited();    /* Reset the visited flags so we can execute the children again. */

                mSystem->mDSPTempBuffIndex = tempbuffindex;

                result = DSPFilter::execute(resamplebuff, &outbuff, &readlen, inchannels, &outch, speakermode);
                if (result != FMOD_OK)
                {
                    outbuff = resamplebuff;
                    FMOD_memset(outbuff, 0, readlen * mResampleBufferChannels * sizeof(float));
                    mResampleFinishPos = mResampleBufferPos;
                }

                mResampleBufferChannels = outch;

                if (outbuff != resamplebuff)
                {
                    FMOD_memcpy(resamplebuff, outbuff, readlen * mResampleBufferChannels * sizeof(float));
                }

                mResampleBufferPos += readlen;
                if (mResampleBufferPos >= mResampleBufferLength)
                {
                    mResampleBufferPos = 0;
                }

                /*
                    Copy the first samples to the end of the buffer so when interpolation does a read past the end, the data will be valid.
                */
                if (!oldresamplebufferpos)
                {
                    unsigned int count;

                    for (count=0; count < mResampleBufferChannels * mOverflowLength * 2; count++)
                    {
                        ((float *)mResampleBuffer)[(mResampleBufferLength * mResampleBufferChannels) + count] = ((float *)mResampleBuffer)[count];
                    }
                }

                mFill--;
            }

            /*
                Work out what is going to end first.  The source data or the output?
            */
            rlength = inlength;     /* Start it off defaulting to the amount requested */

            if (speed.mValue > 0x100)
            {
                FMOD_UINT64P mixesleft, fracleft;
                int nextpos;

                nextpos  = mResamplePosition.mHi - mOverflowLength;
                nextpos /= (int)mResampleBlockLength;
                nextpos ++;
                nextpos *= mResampleBlockLength;
                nextpos += mOverflowLength;

                mixesleft.mHi = nextpos;
                mixesleft.mLo = 0;
				mixesleft.mValue -= mResamplePosition.mValue;

				fracleft.mValue = mixesleft.mValue % speed.mValue;
				mixesleft.mValue /= speed.mValue;

                if (fracleft.mValue)						/* round the count up (this could be done better) */
                {
					mixesleft.mValue++;
                }

				if (mixesleft.mValue <= rlength)
    			{
                    endflag = FMOD_RESAMPLER_END_RESAMPLEBUFFER;
					rlength = mixesleft.mLo;
				}
            }

            /*
                Resample the src buffer into the destination.
            */
            if (speed.mHi == 1 && speed.mLo == 0)
            {
                FMOD_memcpy(readbuffer + (readoffset * mResampleBufferChannels), (float *)mResampleBuffer + (mResamplePosition.mHi * mResampleBufferChannels), rlength * sizeof(float) * mResampleBufferChannels);
                mResamplePosition.mValue += speed.mValue * rlength;
            }
            else
            {
                switch (mSystem->mResampleMethod)
                {
                    case FMOD_DSP_RESAMPLER_NOINTERP:
                    {
                        FMOD_Resampler_NoInterp(readbuffer + (readoffset * mResampleBufferChannels), rlength, mResampleBuffer, FMOD_SOUND_FORMAT_PCMFLOAT, &mResamplePosition, &speed, mResampleBufferChannels);
                        break;
                    }
                    case FMOD_DSP_RESAMPLER_LINEAR:
                    {
                        FMOD_Resampler_Linear(readbuffer + (readoffset * mResampleBufferChannels), rlength, mResampleBuffer, FMOD_SOUND_FORMAT_PCMFLOAT, &mResamplePosition, &speed, mResampleBufferChannels);
                        break;
                    }
                    case FMOD_DSP_RESAMPLER_CUBIC:
                    {
                        FMOD_Resampler_Cubic(readbuffer + (readoffset * mResampleBufferChannels), rlength, mResampleBuffer, FMOD_SOUND_FORMAT_PCMFLOAT, &mResamplePosition, &speed, mResampleBufferChannels);
                        break;
                    }
                    case FMOD_DSP_RESAMPLER_SPLINE:
                    {
                        FMOD_Resampler_Spline(readbuffer + (readoffset * mResampleBufferChannels), rlength, mResampleBuffer, FMOD_SOUND_FORMAT_PCMFLOAT, &mResamplePosition, &speed, mResampleBufferChannels);
                        break;
                    }
                    default:
                    {
                        FMOD_Resampler_Linear(readbuffer + (readoffset * mResampleBufferChannels), rlength, mResampleBuffer, FMOD_SOUND_FORMAT_PCMFLOAT, &mResamplePosition, &speed, mResampleBufferChannels);
                        break;
                    }
                }
            }

            if (mResamplePosition.mHi >= (mResampleBufferLength + mOverflowLength))
            {
                mResamplePosition.mHi -= mResampleBufferLength;
            }

            inlength   -= rlength;
            readoffset += rlength;

            if (endflag == FMOD_RESAMPLER_END_SOUND)
            {
                mFlags |= FMOD_DSP_FLAG_FINISHED;
                break;
            }
            else if (endflag == FMOD_RESAMPLER_END_RESAMPLEBUFFER)
            {
                mFill++;
            }
        } while (inlength > 0);

        channels = mResampleBufferChannels;
    }

    *outbuffer   = readbuffer;
    *outchannels = channels;

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
// VERIFY: 5d79e23f auto
// OFFSET: 0x00929BD0
FMOD_RESULT DSPResamplerMultiInput::addInput(DSPI *target)
{
    FMOD_RESULT result;

    result = DSPI::addInput(target);
    if (result != FMOD_OK)
    {
        return result;
    }

    mResampleBufferPos       = 0;
    mFill                    = 2;
    mPosition.mValue         = 0;
    mResamplePosition.mValue = 0;

    return FMOD_OK;
}


}

#endif
