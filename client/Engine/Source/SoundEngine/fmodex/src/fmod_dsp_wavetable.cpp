#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_channel_software.h"
#include "fmod_dsp_wavetable.h"
#include "fmod_dsp_resampler.h"
#include "fmod_dsp_resampler_nointerp.h"
#include "fmod_dsp_resampler_linear.h"
#include "fmod_dsp_resampler_cubic.h"
#include "fmod_dsp_resampler_spline.h"
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
// VERIFY: 4e34843a auto
// OFFSET: 0x00929DA0
FMOD_RESULT DSPWaveTable::alloc(FMOD_DSP_DESCRIPTION_EX *description)
{
    FMOD_RESULT result;

    result = DSPI::alloc(description);
    if (result != FMOD_OK)
    {
        return result;
    }

    mFrequency = 0;

    mSystem->getSoftwareFormat(&mTargetFrequency, 0, 0, 0, 0, 0);

    mDirection = DSPWAVETABLE_SPEEDDIR_FORWARDS;

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
// VERIFY: 1232faed auto
// OFFSET: 0x00929DE0
FMOD_RESULT DSPWaveTable::addInput(DSPI *target)
{
    return DSPI::addInput(target);
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
// VERIFY: ed9db0cd manual
// OFFSET: 0x00929DF0
FMOD_RESULT DSPWaveTable::execute(float *inbuffer, float **outbuffer, unsigned int *length, int inchannels, int *outchannels, FMOD_SPEAKERMODE speakermode)
{
    *outbuffer = inbuffer;

    mFlags &= ~FMOD_DSP_FLAG_IDLE;

    if (mFlags & FMOD_DSP_FLAG_VISITED)
    {
        *outbuffer = mBuffer;       /* Return the already processed buffer */
    }
    else
    {
        void            *readbuffer;
        SampleSoftware  *samplesoftware;
        unsigned int     outlength       = *length;
        unsigned int     readoffset      = 0;

        samplesoftware = (SampleSoftware *)mSound;
        if (!samplesoftware)
        {
            return FMOD_ERR_INVALID_PARAM;
        }

        if (mBuffer)
        {
            *outbuffer = mBuffer;
        }

        readbuffer = samplesoftware->mBuffer;
        if (!readbuffer)
        {
            FMOD_memset(*outbuffer, 0, *length * sizeof(float) * mSound->mChannels);
            mFlags |= FMOD_DSP_FLAG_FINISHED;
            *outchannels = mSound->mChannels;
            return FMOD_OK;
        }

        do
        {
            unsigned int        rlength;
            FMOD_SINT64P        speed;
            FMOD_RESAMPLER_END  endflag = FMOD_RESAMPLER_END_MIXBUFFER;

            /*
                Work out what is going to end first.  The source data or the output?
            */
            speed = mSpeed;
            if (mDirection == DSPWAVETABLE_SPEEDDIR_BACKWARDS)
            {
                speed.mValue = -speed.mValue;
            }

            rlength = outlength;     /* Start it off defaulting to the amount requested */

            if (mSpeed.mValue > 0x100)
            {
                FMOD_UINT64P mixesleft, fracleft;

                if (mDirection == DSPWAVETABLE_SPEEDDIR_BACKWARDS)
                {
                    mixesleft.mHi = mPosition.mHi;
                    if (mixesleft.mHi >= mChannel->mLoopStart)
                    {
                        mixesleft.mHi -= mChannel->mLoopStart;
                    }
                    mixesleft.mLo = 0;
                }
                else
                {
                    unsigned int endpoint;

                    if (mChannel->mMode & (FMOD_LOOP_NORMAL | FMOD_LOOP_BIDI) && mChannel->mLoopCount)
                    {
                        endpoint = mChannel->mLoopStart + mChannel->mLoopLength;
                    }
                    else
                    {
                        endpoint = mSound->mLength - 1;
                    }

                    if (endpoint > mPosition.mHi)
                    {
                        mixesleft.mHi = endpoint;
                        mixesleft.mLo = 0;
                        mixesleft.mValue -= mPosition.mValue;
                    }
                    else
                    {
                        mixesleft.mValue = 0;
                    }
                }

                fracleft.mValue   = mixesleft.mValue % mSpeed.mValue;
                mixesleft.mValue /= mSpeed.mValue;

                if (fracleft.mValue)						/* round the count up (this could be done better) */
                {
					mixesleft.mValue++;
                }

				if (mixesleft.mLo <= outlength)
    			{
                    endflag = FMOD_RESAMPLER_END_SOUND;
					rlength = mixesleft.mLo;
				}
            }

            /*
                Resample the src buffer into the destination.
            */
            if (mSpeed.mHi == 1 && speed.mLo == 0)
            {
                FMOD_Resampler_NoInterp(*outbuffer + (readoffset * mSound->mChannels), rlength, readbuffer, mSound->mFormat, &mPosition, &speed, mSound->mChannels);
            }
            else
            {
                switch (mSystem->mResampleMethod)
                {
                    case FMOD_DSP_RESAMPLER_NOINTERP:
                    {
                        FMOD_Resampler_NoInterp(*outbuffer + (readoffset * mSound->mChannels), rlength, readbuffer, mSound->mFormat, &mPosition, &speed, mSound->mChannels);
                        break;
                    }
                    case FMOD_DSP_RESAMPLER_LINEAR:
                    {
                        FMOD_Resampler_Linear(*outbuffer + (readoffset * mSound->mChannels), rlength, readbuffer, mSound->mFormat, &mPosition, &speed, mSound->mChannels);
                        break;
                    }
                    case FMOD_DSP_RESAMPLER_CUBIC:
                    {
                        FMOD_Resampler_Cubic(*outbuffer + (readoffset * mSound->mChannels), rlength, readbuffer, mSound->mFormat, &mPosition, &speed, mSound->mChannels);
                        break;
                    }
                    case FMOD_DSP_RESAMPLER_SPLINE:
                    {
                        FMOD_Resampler_Spline(*outbuffer + (readoffset * mSound->mChannels), rlength, readbuffer, mSound->mFormat, &mPosition, &speed, mSound->mChannels);
                        break;
                    }
                    default:
                    {
                        FMOD_Resampler_Linear(*outbuffer + (readoffset * mSound->mChannels), rlength, readbuffer, mSound->mFormat, &mPosition, &speed, mSound->mChannels);
                        break;
                    }
                }
            }

            outlength  -= rlength;
            readoffset += rlength;

            if (endflag == FMOD_RESAMPLER_END_SOUND)
            {
                if (mChannel->mMode & FMOD_LOOP_BIDI && mChannel->mLoopCount)
                {
                    if ((int)mPosition.mHi < 0)
                    {
                        mPosition.mHi = 0;
                    }

                    if (mDirection == DSPWAVETABLE_SPEEDDIR_FORWARDS)
                    {
                        mDirection = DSPWAVETABLE_SPEEDDIR_BACKWARDS;
                    }
                    else
                    {
                        mDirection = DSPWAVETABLE_SPEEDDIR_FORWARDS;
                    }
                }
                else if (mChannel->mMode & FMOD_LOOP_NORMAL && mChannel->mLoopCount)
                {
                    if (mDirection == DSPWAVETABLE_SPEEDDIR_BACKWARDS)
                    {
                        mPosition.mHi += mChannel->mLoopLength;

                        /*
                            Safety check.  Sometimes the speed is so high it might go past double the length of the loop.
                        */
                        while (mPosition.mHi < mChannel->mLoopStart)
                        {
                            mPosition.mHi += mChannel->mLoopLength;
                        }
                    }
                    else
                    {
                        mPosition.mHi -= mChannel->mLoopLength;

                        /*
                            Safety check.  Sometimes the speed is so high it might go past double the length of the loop.
                        */
                        while (mPosition.mHi >= mChannel->mLoopStart + mChannel->mLoopLength)
                        {
                            mPosition.mHi -= mChannel->mLoopLength;
                        }

                        if (mChannel->mLoopCount > 0)
                        {
                            mChannel->mLoopCount --;
                        }
                    }
                }
                else
                {
                    mPosition.mHi = mSound->mLength;
                    mPosition.mLo = 0;

                    FMOD_memset(*outbuffer + (readoffset * mSound->mChannels), 0, outlength * mSound->mChannels * sizeof(float));

                    mFlags |= FMOD_DSP_FLAG_FINISHED;
                    break;
                }
            }
        } while (outlength);
    }

    *outchannels = mSound->mChannels;

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
FMOD_RESULT DSPWaveTable::setPositionInternal(unsigned int position)
{
    if (!mSound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (position >= mSound->mLength)
    {
        position = mSound->mLength;
    }

    mPosition.mHi = position;
    mPosition.mLo = 0;

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
FMOD_RESULT DSPWaveTable::setParameterInternal(int index, float value)
{
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
FMOD_RESULT DSPWaveTable::getParameterInternal(int index, float *value, char *valuestr)
{   
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
// VERIFY: c9e0222e auto
// OFFSET: 0x0092A1E0
FMOD_RESULT DSPWaveTable::setFrequency(float frequency)
{
    if (frequency < 0)
    {
        mDirection = DSPWAVETABLE_SPEEDDIR_BACKWARDS;
        frequency = -frequency;
    }
    else
    {
        SampleSoftware  *samplesoftware = (SampleSoftware *)mSound;

        if (samplesoftware && !(mChannel->mMode & FMOD_LOOP_BIDI))
        {
            mDirection = DSPWAVETABLE_SPEEDDIR_FORWARDS;
        }
    }

    mFrequency = frequency;

    mSpeed.mValue = (FMOD_SINT64)(mFrequency / mTargetFrequency * 4294967296.0f);

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
FMOD_RESULT DSPWaveTable::getFrequency(float *frequency)
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
// VERIFY: 49223937 auto
// OFFSET: 0x0092A250
FMOD_RESULT F_CALLBACK DSPWaveTable::setPositionCallback(FMOD_DSP_STATE *dsp, unsigned int position)
{
    DSPWaveTable *wavetable = (DSPWaveTable *)dsp;

    return wavetable->setPositionInternal(position);
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
// VERIFY: c239578a auto
// OFFSET: 0x0092A2A0
FMOD_RESULT F_CALLBACK DSPWaveTable::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPWaveTable *wavetable = (DSPWaveTable *)dsp;

    return wavetable->setParameterInternal(index, value);
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
// VERIFY: f63fdcdf auto
// OFFSET: 0x0092A2B0
FMOD_RESULT F_CALLBACK DSPWaveTable::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPWaveTable *wavetable = (DSPWaveTable *)dsp;

    return wavetable->getParameterInternal(index, value, valuestr);
}


}

#endif
