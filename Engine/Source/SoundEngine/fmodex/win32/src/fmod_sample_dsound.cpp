#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_DSOUND

#include "fmod.h"
#include "fmod_output_dsound.h"
#include "fmod_sample_dsound.h"
#include "fmod_systemi.h"

namespace FMOD
{


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 0cd676dc auto
// OFFSET: 0x009510C0
SampleDSound::SampleDSound()
{
    mBuffer = 0;
    mBuffer3D = 0;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: cbd55ab6 auto
// OFFSET: 0x009510E0
FMOD_RESULT SampleDSound::release(bool freethis)
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

	/*
        Free sample's secondary buffer
    */
	if (mBuffer)
	{
        if (mOutput)
        {
            mOutput->mBufferMemoryCurrent -= mLengthBytes;
        }

		mBuffer->Release();
        mBuffer = 0;
	}

    if(mBuffer3D)
    {
        mBuffer3D->Release();
        mBuffer3D = 0;
    }

    return Sample::release(freethis);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: c910bc9e manual
// OFFSET: 0x00951190
FMOD_RESULT SampleDSound::lockInternal(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
	HRESULT hr;

    if (!mBuffer)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    hr = mBuffer->Lock(offset, length, ptr1, (DWORD *)len1, ptr2, (DWORD *)len2, 0);   /* not DSBLOCK_FROMWRITECURSOR OR DSBLOCK_ENTIREBUFFER */
    if (hr == E_INVALIDARG)
    {
        return FMOD_ERR_INVALID_PARAM;
    }
    else if (hr == DSBSTATUS_BUFFERLOST)
    {
        DWORD locflags = 0;

        if (!(mMode & FMOD_UNIQUE))
        {
            if (mLOCSoftware)
            {
	        	locflags |= DSBPLAY_LOCSOFTWARE;
			}
            else
            {
    			locflags |= DSBPLAY_LOCHARDWARE;
            }
        }
        mBuffer->Restore();
        mBuffer->Play(0, 0, locflags | DSBPLAY_LOOPING);
    }
    else if (hr != DS_OK)
    {
        return FMOD_ERR_OUTPUT_DRIVERCALL;

    }
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: b5103a0e auto
// OFFSET: 0x00951240
FMOD_RESULT SampleDSound::unlockInternal(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
	HRESULT hr;

	if (!mBuffer)
	{
		return FMOD_ERR_INVALID_PARAM;
	}

    if (mFormat == FMOD_SOUND_FORMAT_PCM8)
    {
        if (ptr1 && len1)
        {
            unsigned int count;
            unsigned char *destptr = (unsigned char *)ptr1;

            for (count = 0; count < len1; count++)
            {
                *destptr++ ^= 128;
            }
        }
        if (ptr2 && len2)
        {
            unsigned int count;
            unsigned char *destptr = (unsigned char *)ptr2;

            for (count = 0; count < len2; count++)
            {
                *destptr++ ^= 128;
            }
        }
    }

	hr = mBuffer->Unlock(ptr1, len1, ptr2, len2);
	if (hr != DS_OK)
    {
		return FMOD_ERR_OUTPUT_DRIVERCALL;
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

#endif /* FMOD_SUPPORT_DSOUND */
