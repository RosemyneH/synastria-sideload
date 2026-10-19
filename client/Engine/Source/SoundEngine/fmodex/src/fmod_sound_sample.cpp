#include "fmod_settings.h"

#include "fmod_codeci.h"
#include "fmod_sound_sample.h"
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
// VERIFY: 65d4c73e auto
// OFFSET: 0x0091AF20
Sample::Sample()
{
    mNumSubSamples = 0;
    mLockCanRead = true;
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
// VERIFY: ee36b9b3 manual
// OFFSET: 0x0091B2E0
FMOD_RESULT Sample::lock(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{   
    int count;
    unsigned int samples = 0;

    if (!ptr1 || !len1)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (mNumSubSamples < 1)
    {
        return lockInternal(offset, length, ptr1, ptr2, len1, len2);
    }

    if (mMode & FMOD_CREATECOMPRESSEDSAMPLE)
    {
        return FMOD_ERR_FORMAT;
    }

    FMOD_OS_CriticalSection_Enter(mSystem->mMultiSubSampleLockBufferCrit);

    *ptr1 = mLockBuffer;
    if (ptr2)
    {
        *ptr2 = 0;
    }
    *len1 = length;
    if (len2)
    {
        *len2 = 0;
    }

    mLockLength = length;
    mLockOffset = offset;
   
    SoundI::getSamplesFromBytes(length, &samples);

    length /= mNumSubSamples;
    offset /= mNumSubSamples;

    for (count = 0; count < mNumSubSamples; count++)
    {
        if (mSubSample[count]->mLockCanRead)
        {
            unsigned int count2;
            unsigned int in_len1, in_len2;
            void *in_ptr1, *in_ptr2;

            mSubSample[count]->lock(offset, length, &in_ptr1, &in_ptr2, &in_len1, &in_len2);

            switch (mFormat)
            {
                case FMOD_SOUND_FORMAT_PCM8:
                {
                    signed char *dest = ((signed char *)*ptr1) + count;
                    signed char *src  = (signed char *)in_ptr1;

                    count2 = samples >> 3;
                    while (count2)
                    {
                        dest[0]                  = src[0];
                        dest[mNumSubSamples * 1] = src[1];
                        dest[mNumSubSamples * 2] = src[2];
                        dest[mNumSubSamples * 3] = src[3];
                        dest[mNumSubSamples * 4] = src[4];
                        dest[mNumSubSamples * 5] = src[5];
                        dest[mNumSubSamples * 6] = src[6];
                        dest[mNumSubSamples * 7] = src[7];
                        
                        dest += mNumSubSamples * 8;
                        src += 8;
                        count2--;
                    }                    
                    
                    count2 = samples & 7;
                    while (count2)
                    {
                        dest[0] = src[0];
                        
                        dest += mNumSubSamples;
                        src ++;
                        count2--;
                    }                    
                    break;
                }
                case FMOD_SOUND_FORMAT_GCADPCM:
                {
                    samples = length/2;
                }
                case FMOD_SOUND_FORMAT_PCM16:
                {
                    signed short *dest = ((signed short *)*ptr1) + count;
                    signed short *src  = (signed short *)in_ptr1;

                    count2 = samples >> 3;
                    while (count2)
                    {
                        dest[0]                  = src[0];
                        dest[mNumSubSamples * 1] = src[1];
                        dest[mNumSubSamples * 2] = src[2];
                        dest[mNumSubSamples * 3] = src[3];
                        dest[mNumSubSamples * 4] = src[4];
                        dest[mNumSubSamples * 5] = src[5];
                        dest[mNumSubSamples * 6] = src[6];
                        dest[mNumSubSamples * 7] = src[7];
                        
                        dest += mNumSubSamples * 8;
                        src += 8;
                        count2--;
                    }                    
                    
                    count2 = samples & 7;
                    while (count2)
                    {
                        dest[0] = src[0];
                        
                        dest += mNumSubSamples;
                        src ++;
                        count2--;
                    }                    

                    break;
                }
                case FMOD_SOUND_FORMAT_PCM24:
                {
                    for (count2 = 0; count2 < samples; count2++)
                    {
                        ((unsigned char *)*ptr1)[(((mNumSubSamples * count2) + count) * 3) + 0] = ((unsigned char *)in_ptr1)[(count2 * 3) + 0];
                        ((unsigned char *)*ptr1)[(((mNumSubSamples * count2) + count) * 3) + 1] = ((unsigned char *)in_ptr1)[(count2 * 3) + 1];
                        ((unsigned char *)*ptr1)[(((mNumSubSamples * count2) + count) * 3) + 2] = ((unsigned char *)in_ptr1)[(count2 * 3) + 2];
                    }
                    break;
                }
                case FMOD_SOUND_FORMAT_PCM32:
                case FMOD_SOUND_FORMAT_PCMFLOAT:
                {
                    unsigned int *dest = ((unsigned int *)*ptr1) + count;
                    unsigned int *src  = (unsigned int *)in_ptr1;

                    count2 = samples >> 3;
                    while (count2)
                    {
                        dest[0]                  = src[0];
                        dest[mNumSubSamples * 1] = src[1];
                        dest[mNumSubSamples * 2] = src[2];
                        dest[mNumSubSamples * 3] = src[3];
                        dest[mNumSubSamples * 4] = src[4];
                        dest[mNumSubSamples * 5] = src[5];
                        dest[mNumSubSamples * 6] = src[6];
                        dest[mNumSubSamples * 7] = src[7];
                        
                        dest += mNumSubSamples * 8;
                        src += 8;
                        count2--;
                    }                    
                    
                    count2 = samples & 7;
                    while (count2)
                    {
                        dest[0] = src[0];
                        
                        dest += mNumSubSamples;
                        src ++;
                        count2--;
                    }                    
                    break;
                }
                case FMOD_SOUND_FORMAT_IMAADPCM:
                {
                    /* Stereo is a special case where it is interleaved every 4 bytes. */
                    if (mNumSubSamples == 2)
                    {
                        samples = length / 4;
                    
                        unsigned int *dest = ((unsigned int *)*ptr1) + count;
                        unsigned int *src  = (unsigned int *)in_ptr1;

                        count2 = samples >> 3;
                        while (count2)
                        {
                            dest[0]                  = src[0];
                            dest[mNumSubSamples * 1] = src[1];
                            dest[mNumSubSamples * 2] = src[2];
                            dest[mNumSubSamples * 3] = src[3];
                            dest[mNumSubSamples * 4] = src[4];
                            dest[mNumSubSamples * 5] = src[5];
                            dest[mNumSubSamples * 6] = src[6];
                            dest[mNumSubSamples * 7] = src[7];
                        
                            dest += mNumSubSamples * 8;
                            src += 8;
                            count2--;
                        }                    
                    
                        count2 = samples & 7;
                        while (count2)
                        {
                            dest[0] = src[0];
                        
                            dest += mNumSubSamples;
                            src ++;
                            count2--;
                        }                    
                        break;
                   }
                   /* else drop through to the following case and use 36 byte blocks. */
                }
                case FMOD_SOUND_FORMAT_VAG:
                {
                    unsigned int blocksize = 0;
                    unsigned int blocks;
                    char *dest = (char *)*ptr1;
                    char *src = (char *)in_ptr1;
                
                    SoundI::getBytesFromSamples(1, &blocksize, 1, mFormat);

                    blocks = length / blocksize;
                
                    dest += (count * blocksize);
                
                    for (count2 = 0; count2 < blocks; count2++)
                    {
                        FMOD_memcpy(dest, src, blocksize);
                        dest += (blocksize * mNumSubSamples);
                        src  += blocksize;
                    }                    
                    break;
                }
                case FMOD_SOUND_FORMAT_XMA:
                {
                    signed char *dest = ((signed char *)*ptr1) + count;
                    signed char *src  = (signed char *)in_ptr1;

                    count2 = samples >> 3;
                    while (count2)
                    {
                        dest[0]                  = src[0];
                        dest[mNumSubSamples * 1] = src[1];
                        dest[mNumSubSamples * 2] = src[2];
                        dest[mNumSubSamples * 3] = src[3];
                        dest[mNumSubSamples * 4] = src[4];
                        dest[mNumSubSamples * 5] = src[5];
                        dest[mNumSubSamples * 6] = src[6];
                        dest[mNumSubSamples * 7] = src[7];
                        
                        dest += mNumSubSamples * 8;
                        src += 8;
                        count2--;
                    }                    
                    
                    count2 = samples & 7;
                    while (count2)
                    {
                        dest[0] = src[0];
                        
                        dest += mNumSubSamples;
                        src ++;
                        count2--;
                    }                    
                    break;
                }
                default:
                {
                    FMOD_OS_CriticalSection_Leave(mSystem->mMultiSubSampleLockBufferCrit);

                    return FMOD_ERR_FORMAT;
                }
            }

            mSubSample[count]->unlock(in_ptr1, in_ptr2, in_len1, in_len2);
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
// VERIFY: deda40ef manual
// OFFSET: 0x0091BB80
FMOD_RESULT Sample::unlock(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    int count;
    unsigned int length, offset, samples = 0;

    if (!ptr1 || !len1)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (mNumSubSamples < 1)
    {
        return unlockInternal(ptr1, ptr2, len1, len2);
    }
   
    SoundI::getSamplesFromBytes(mLockLength, &samples);

    length = mLockLength / mNumSubSamples;
    offset = mLockOffset / mNumSubSamples;

    for (count = 0; count < mNumSubSamples; count++)
    {
        unsigned int count2;
        unsigned int in_len1, in_len2;
        void *in_ptr1, *in_ptr2;

        mSubSample[count]->lock(offset, length, &in_ptr1, &in_ptr2, &in_len1, &in_len2);

        switch (mFormat)
        {
            case FMOD_SOUND_FORMAT_PCM8:
            {
                signed char *src = ((signed char *)ptr1) + count;
                signed char *dest = (signed char *)in_ptr1;

                count2 = samples >> 3;
                while (count2)
                {
                    dest[0] = src[0];
                    dest[1] = src[mNumSubSamples * 1];
                    dest[2] = src[mNumSubSamples * 2];
                    dest[3] = src[mNumSubSamples * 3];
                    dest[4] = src[mNumSubSamples * 4];
                    dest[5] = src[mNumSubSamples * 5];
                    dest[6] = src[mNumSubSamples * 6];
                    dest[7] = src[mNumSubSamples * 7];
                    
                    src += mNumSubSamples * 8;
                    dest += 8;
                    count2--;
                }                    
                
                count2 = samples & 7;
                while (count2)
                {
                    dest[0] = src[0];
                    
                    src += mNumSubSamples;
                    dest ++;
                    count2--;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_GCADPCM:
            {               
                samples = length/2;
            }
            case FMOD_SOUND_FORMAT_PCM16:
            {
                signed short *src = ((signed short *)ptr1) + count;
                signed short *dest  = (signed short *)in_ptr1;

                count2 = samples >> 3;
                while (count2)
                {
                    dest[0] = src[0];
                    dest[1] = src[mNumSubSamples * 1];
                    dest[2] = src[mNumSubSamples * 2];
                    dest[3] = src[mNumSubSamples * 3];
                    dest[4] = src[mNumSubSamples * 4];
                    dest[5] = src[mNumSubSamples * 5];
                    dest[6] = src[mNumSubSamples * 6];
                    dest[7] = src[mNumSubSamples * 7];
                    
                    src += mNumSubSamples * 8;
                    dest += 8;
                    count2--;
                }                    
                
                count2 = samples & 7;
                while (count2)
                {
                    dest[0] = src[0];
                    
                    src += mNumSubSamples;
                    dest ++;
                    count2--;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCM24:
            {
                for (count2 = 0; count2 < samples; count2++)
                {
                    ((unsigned char *)in_ptr1)[(count2 * 3) + 0] = ((unsigned char *)ptr1)[(((mNumSubSamples * count2) + count) * 3) + 0];
                    ((unsigned char *)in_ptr1)[(count2 * 3) + 1] = ((unsigned char *)ptr1)[(((mNumSubSamples * count2) + count) * 3) + 1];
                    ((unsigned char *)in_ptr1)[(count2 * 3) + 2] = ((unsigned char *)ptr1)[(((mNumSubSamples * count2) + count) * 3) + 2];
                }
                break;
            }
            case FMOD_SOUND_FORMAT_PCM32:
            case FMOD_SOUND_FORMAT_PCMFLOAT:
            {
                unsigned int *src = ((unsigned int *)ptr1) + count;
                unsigned int *dest = (unsigned int *)in_ptr1;

                count2 = samples >> 3;
                while (count2)
                {
                    dest[0] = src[0];
                    dest[1] = src[mNumSubSamples * 1];
                    dest[2] = src[mNumSubSamples * 2];
                    dest[3] = src[mNumSubSamples * 3];
                    dest[4] = src[mNumSubSamples * 4];
                    dest[5] = src[mNumSubSamples * 5];
                    dest[6] = src[mNumSubSamples * 6];
                    dest[7] = src[mNumSubSamples * 7];
                    
                    src += mNumSubSamples * 8;
                    dest += 8;
                    count2--;
                }                    
                
                count2 = samples & 7;
                while (count2)
                {
                    dest[0] = src[0];
                    
                    src += mNumSubSamples;
                    dest ++;
                    count2--;
                }
                break;
            }
            case FMOD_SOUND_FORMAT_IMAADPCM:
            {
                /* Stereo is a special case where it is interleaved every 4 bytes. */
                if (mNumSubSamples == 2)
                {
                    samples = length / 4;

                    unsigned int *src = ((unsigned int *)ptr1) + count;
                    unsigned int *dest = (unsigned int *)in_ptr1;

                    count2 = samples >> 3;
                    while (count2)
                    {
                        dest[0] = src[0];
                        dest[1] = src[mNumSubSamples * 1];
                        dest[2] = src[mNumSubSamples * 2];
                        dest[3] = src[mNumSubSamples * 3];
                        dest[4] = src[mNumSubSamples * 4];
                        dest[5] = src[mNumSubSamples * 5];
                        dest[6] = src[mNumSubSamples * 6];
                        dest[7] = src[mNumSubSamples * 7];
                    
                        src += mNumSubSamples * 8;
                        dest += 8;
                        count2--;
                    }                    
                
                    count2 = samples & 7;
                    while (count2)
                    {
                        dest[0] = src[0];
                    
                        src += mNumSubSamples;
                        dest ++;
                        count2--;
                    }
                    break;
                }
                /* else drop through to the following case and use 36byte blocks. */
            }
            case FMOD_SOUND_FORMAT_VAG:
            {
                unsigned int blocksize = 0;
                unsigned int blocks;
                char *src = (char *)ptr1;
                char *dest = (char *)in_ptr1;

                SoundI::getBytesFromSamples(1, &blocksize, 1, mFormat);

                blocks = length / blocksize;

                src += (count * blocksize);

                for (count2 = 0; count2 < blocks; count2++)
                {
                    FMOD_memcpy(dest, src, blocksize);
                    dest += blocksize;
                    src  += (blocksize * mNumSubSamples);
                }
                break;
            }
            case FMOD_SOUND_FORMAT_XMA:
            {
                signed char *src = ((signed char *)ptr1) + count;
                signed char *dest = (signed char *)in_ptr1;

                count2 = samples >> 3;
                while (count2)
                {
                    dest[0] = src[0];
                    dest[1] = src[mNumSubSamples * 1];
                    dest[2] = src[mNumSubSamples * 2];
                    dest[3] = src[mNumSubSamples * 3];
                    dest[4] = src[mNumSubSamples * 4];
                    dest[5] = src[mNumSubSamples * 5];
                    dest[6] = src[mNumSubSamples * 6];
                    dest[7] = src[mNumSubSamples * 7];
                    
                    src += mNumSubSamples * 8;
                    dest += 8;
                    count2--;
                }                    
                
                count2 = samples & 7;
                while (count2)
                {
                    dest[0] = src[0];
                    
                    src += mNumSubSamples;
                    dest ++;
                    count2--;
                }
                break;
            }
            default:
            {
                FMOD_OS_CriticalSection_Leave(mSystem->mMultiSubSampleLockBufferCrit);

                return FMOD_ERR_FORMAT;
            }
        }

        mSubSample[count]->unlock(in_ptr1, in_ptr2, in_len1, in_len2);
    }
    
    FMOD_OS_CriticalSection_Leave(mSystem->mMultiSubSampleLockBufferCrit);

//    FMOD_Memory_Free(mLockBuffer);

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
// VERIFY: a67435ce auto
// OFFSET: 0x0091AF70
FMOD_RESULT Sample::release(bool freethis)
{
    int count;
    
    if (mSystem)
    {
        mSystem->stopSound(this);

        if (mLockBuffer)
        {
            mSystem->mMultiSubSampleLockBuffer.free();
        }
    }

    for (count = 0; count < mNumSubSamples; count++)
    {
        if (mSubSample[count])
        {
            mSubSample[count]->mCodec = 0;      /* Dont free the codec multiple times */
            mSubSample[count]->release();
            mSubSample[count] = 0;
        }
    }

    return SoundI::release(freethis);
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
// VERIFY: efed93a3 auto
// OFFSET: 0x0091B000
FMOD_RESULT Sample::setDefaults(float frequency, float volume, float pan, int priority)
{
    FMOD_RESULT result;
    int count;

    result = SoundI::setDefaults(frequency, volume, pan, priority);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < mNumSubSamples; count++)
    {
        mSubSample[count]->setDefaults(frequency, volume, pan, priority);
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
// VERIFY: 304341aa auto
// OFFSET: 0x0091B080
FMOD_RESULT Sample::setVariations(float frequencyvar, float volumevar, float panvar)
{
    FMOD_RESULT result;
    int count;

    result = SoundI::setVariations(frequencyvar, volumevar, panvar);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < mNumSubSamples; count++)
    {
        mSubSample[count]->setVariations(frequencyvar, volumevar, panvar);
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
// VERIFY: 23b8e2f9 auto
// OFFSET: 0x0091B0F0
FMOD_RESULT Sample::set3DMinMaxDistance(float min, float max)
{
    FMOD_RESULT result;
    int count;

    result = SoundI::set3DMinMaxDistance(min, max);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < mNumSubSamples; count++)
    {
        mSubSample[count]->set3DMinMaxDistance(min, max);
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
// VERIFY: a613c93c auto
// OFFSET: 0x0091B160
FMOD_RESULT Sample::set3DConeSettings(float insideconeangle, float outsideconeangle, float outsidevolume)
{
    FMOD_RESULT result;
    int count;

    result = SoundI::set3DConeSettings(insideconeangle, outsideconeangle, outsidevolume);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < mNumSubSamples; count++)
    {
        mSubSample[count]->set3DConeSettings(insideconeangle, outsideconeangle, outsidevolume);
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
// VERIFY: 4ba8f5aa auto
// OFFSET: 0x0091B1D0
FMOD_RESULT  Sample::setMode(FMOD_MODE mode)
{
    FMOD_RESULT result;
    int count;

    result = SoundI::setMode(mode);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < mNumSubSamples; count++)
    {
        mSubSample[count]->setMode(mode);
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
// VERIFY: d9adf225 auto
// OFFSET: 0x0091B220
FMOD_RESULT  Sample::setLoopCount(int loopcount)
{
    FMOD_RESULT result;
    int count;

    result = SoundI::setLoopCount(loopcount);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < mNumSubSamples; count++)
    {
        mSubSample[count]->setLoopCount(loopcount);
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
// VERIFY: 67e3442b auto
// OFFSET: 0x0091B270
FMOD_RESULT Sample::setLoopPoints(unsigned int loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int loopend, FMOD_TIMEUNIT loopendtype)
{
    FMOD_RESULT result;
    int count;

    result = SoundI::setLoopPoints(loopstart, loopstarttype, loopend, loopendtype);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < mNumSubSamples; count++)
    {
        mSubSample[count]->setLoopPoints(loopstart, loopstarttype, loopend, loopendtype);
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

}
