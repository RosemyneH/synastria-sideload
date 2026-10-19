#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL

#include "fmod_memory.h"
#include "fmod_output.h"
#include "fmod_output_software.h"
#include "fmod_sample_openal.h"
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
// VERIFY: c6c0cc76 auto
// OFFSET: 0x0094B7F0
SampleOpenAL::SampleOpenAL()
{
    mBuffer = 0;
    mBufferMemory = 0;
    mDataEndCopied = false;
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
// VERIFY: 2e6223a5 auto
// OFFSET: 0x0094B820
FMOD_RESULT SampleOpenAL::release(bool freethis)
{
    FMOD_RESULT result;

    if (!mSystem)
    {
        return FMOD_ERR_UNINITIALIZED;
    }
    
    while (mOpenState != FMOD_OPENSTATE_READY && mOpenState != FMOD_OPENSTATE_ERROR)
    {
        FMOD_OS_Time_Sleep(2);
    }

    result = mSystem->stopSound(this);
    if (result != FMOD_OK)
    {
        return result;
    }

	if (mBufferMemory)
	{
        FMOD_Memory_Free(mBufferMemory);
        mBufferMemory = 0;
	}
	
	if (mLoopPointDataEnd && mLoopPointDataEnd != mLoopPointDataEndMemory)
	{
	    FMOD_Memory_Free(mLoopPointDataEnd);
	    mLoopPointDataEnd = 0;
	}

    mBuffer = 0;

    return Sample::release(freethis);
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
// VERIFY: 04dd2092 auto
// OFFSET: 0x0093E150
FMOD_RESULT SampleOpenAL::setMode(FMOD_MODE mode)
{
    FMOD_RESULT result;

    result = Sample::setMode(mode);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = setLoopPointData();
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

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
// VERIFY: f896c377 auto
// OFFSET: 0x0093E170
FMOD_RESULT SampleOpenAL::lockInternal(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    FMOD_RESULT result;
    char *src = 0;
    unsigned int overflowbytes, pointB;
        
    /*
        If we're locking part of the sample that has been modified by the loop point stuff, restore the old data first.
    */
    result = getBytesFromSamples(FMOD_DSP_RESAMPLER_OVERFLOWLENGTH, &overflowbytes);
    if (result != FMOD_OK)
    {
        return result;
    }
  
    result = getBytesFromSamples(mLoopStart + mLoopLength, &pointB);
    if (result != FMOD_OK)
    {
        return result;
    }
 
    if (offset >= pointB && offset < pointB + overflowbytes)
    {
        result = restoreLoopPointData();
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    src = (char *)mBuffer;
        
    if (offset >= mLengthBytes || offset < 0 || length < 0) 
	{
		*ptr1 = 0;
        if (ptr2)
        {
		    *ptr2 = 0;
        }
		*len1 = 0;
        if (len2)
        {
		    *len2 = 0;
        }
		return FMOD_ERR_INVALID_PARAM;
	}

	if (offset + length <= mLengthBytes)
	{
        *ptr1 = src + offset;
		*len1 = length;
        if (ptr2)
        {
		    *ptr2 = 0;
        }
        if (len2)
        {
		    *len2 = 0;
        }
	}
	/*
        Otherwise return wrapped pointers in pt1 and ptr2
    */
	else
	{
        *ptr1 = src + offset;
		*len1 = mLengthBytes - offset;
		*ptr2 = src;
		*len2 = length - (mLengthBytes - offset);
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
// VERIFY: f9985ff7 auto
// OFFSET: 0x0093E460
FMOD_RESULT SampleOpenAL::unlockInternal(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    FMOD_RESULT result;

    result = setLoopPointData();
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

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/        
// VERIFY: cecf952c auto
// OFFSET: 0x0094B8F0
FMOD_RESULT SampleOpenAL::setBufferData(void *data) 
{ 
    mBuffer = data; 
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
FMOD_RESULT SampleOpenAL::setLoopPoints(unsigned int loopstart, unsigned int looplength)
{
    if (loopstart >= mLength || loopstart + looplength > mLength)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    restoreLoopPointData();

    mLoopStart = loopstart;
    mLoopLength = looplength;

    setLoopPointData();

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
// VERIFY: 664e6853 manual
// OFFSET: 0x0093D8F0
FMOD_RESULT SampleOpenAL::setLoopPointData()
{
    FMOD_RESULT result;
    unsigned int overflowbytes,  pointA, pointB;

    if ((mFormat != FMOD_SOUND_FORMAT_PCM8 &&
         mFormat != FMOD_SOUND_FORMAT_PCM16 &&
         mFormat != FMOD_SOUND_FORMAT_PCM24 &&
         mFormat != FMOD_SOUND_FORMAT_PCM32 &&
         mFormat != FMOD_SOUND_FORMAT_PCMFLOAT) || !mLoopPointDataEnd)
    {
        return FMOD_OK;
    }

    result = getBytesFromSamples(FMOD_DSP_RESAMPLER_OVERFLOWLENGTH, &overflowbytes);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = getBytesFromSamples(mLoopStart, &pointA);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = getBytesFromSamples(mLoopStart + mLoopLength, &pointB);
    if (result != FMOD_OK)
    {
        return result;
    }
    
    if (mMode & FMOD_LOOP_BIDI)
    {
        int count;

        FMOD_memcpy(mLoopPointDataEnd, (char *)mBuffer + pointB, overflowbytes); /* backup the data first */
        mDataEndCopied = true;

        switch (mFormat)
        {
            case FMOD_SOUND_FORMAT_PCM8:
            {
                char *srcptr, *destptr;

                srcptr = destptr = (char *)mBuffer + pointB;
                srcptr -= mChannels;
                for (count=0; count < FMOD_DSP_RESAMPLER_OVERFLOWLENGTH * mChannels; count++)
                {
                    *destptr = *srcptr;
                    destptr++;
                    srcptr--;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCM16:
            {
                short *srcptr, *destptr;

                srcptr = destptr = (short *)((char *)mBuffer + pointB);
                srcptr -= mChannels;
                for (count=0; count < FMOD_DSP_RESAMPLER_OVERFLOWLENGTH * mChannels; count++)
                {
                    *destptr = *srcptr;
                    destptr++;
                    srcptr--;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCM24:
            {
                char *srcptr, *destptr;

                srcptr = destptr = (char *)mBuffer + pointB;
                srcptr -= (mChannels * 3);
                for (count=0; count < FMOD_DSP_RESAMPLER_OVERFLOWLENGTH * mChannels; count++)
                {
                    destptr[0] = srcptr[0];
                    destptr[1] = srcptr[1];
                    destptr[2] = srcptr[2];
                    destptr += 3;
                    srcptr  -= 3;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCM32:
            case FMOD_SOUND_FORMAT_PCMFLOAT:
            {
                int *srcptr, *destptr;

                srcptr = destptr = (int *)((char *)mBuffer + pointB);
                srcptr -= mChannels;
                for (count=0; count < FMOD_DSP_RESAMPLER_OVERFLOWLENGTH * mChannels; count++)
                {
                    *destptr = *srcptr;
                    destptr++;
                    srcptr--;
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else if (mMode & FMOD_LOOP_NORMAL)
    {
        FMOD_memcpy(mLoopPointDataEnd, (char *)mBuffer + pointB, overflowbytes);
        mDataEndCopied = true;

        FMOD_memcpy((char *)mBuffer + pointB, (char *)mBuffer + pointA, overflowbytes);
    }
    else if (mMode & FMOD_LOOP_OFF)
    {
        if(mDataEndCopied)
        {
            FMOD_memcpy((char *)mBuffer + pointB, mLoopPointDataEnd,overflowbytes);
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
// VERIFY: e7b3981b auto
// OFFSET: 0x0093DEB0
FMOD_RESULT SampleOpenAL::restoreLoopPointData()
{
    FMOD_RESULT result;
    unsigned int overflowbytes, pointA, pointB;

    if ((mFormat != FMOD_SOUND_FORMAT_PCM8 &&
         mFormat != FMOD_SOUND_FORMAT_PCM16 &&
         mFormat != FMOD_SOUND_FORMAT_PCM24 &&
         mFormat != FMOD_SOUND_FORMAT_PCM32 &&
         mFormat != FMOD_SOUND_FORMAT_PCMFLOAT) || !mLoopPointDataEnd)
    {
        return FMOD_OK;
    }

    result = getBytesFromSamples(FMOD_DSP_RESAMPLER_OVERFLOWLENGTH, &overflowbytes);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = getBytesFromSamples(mLoopStart, &pointA);
    if (result != FMOD_OK)
    {
        return result;
    }
    result = getBytesFromSamples(mLoopStart + mLoopLength, &pointB);
    if (result != FMOD_OK)
    {
        return result;
    }

    if(mDataEndCopied)
    {
        FMOD_memcpy((char *)mBuffer + pointB, mLoopPointDataEnd,overflowbytes);
    }

    return FMOD_OK;
}


}

#endif /* FMOD_SUPPORT_OPENAL */
