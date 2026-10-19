#include "fmod_settings.h"

#include "fmod.h"
#include "fmod_debug.h"
#include "fmod_output_polled.h"
#include "fmod_memory.h"
#include "fmod_systemi.h"
#include "fmod_thread.h"

// #define DUMPMIXERTODISK

#ifdef DUMPMIXERTODISK

#include <stdio.h>
static FILE *fp = 0;

#endif

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
    
	[SEE_ALSO]
]
*/
// VERIFY: edcb279c manual
// OFFSET: 0x00922950
OutputPolled::OutputPolled()
{
    mCursorBlock = 0;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 3cb210ae manual
// OFFSET: 0x00922B00
FMOD_RESULT OutputPolled::threadFunc()
{
#ifdef FMOD_SUPPORT_SOFTWARE
    FMOD_RESULT           result;
    unsigned int          pcm;
    unsigned int          blocksize;
    int                   numblocks;
    FMOD_SOUND_FORMAT     outputformat;
    int                   outputchannels;
  
    result = mSystem->getDSPBufferSize(&blocksize, &numblocks);
    if (result != FMOD_OK)
    {
        return result;
    }

    numblocks += mMixAheadBlocks;

    result = mSystem->getSoftwareFormat(0, &outputformat, &outputchannels, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (mDescription.getposition)
    {
        result = mDescription.getposition(this, &pcm);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    mSystem->mDSPTimeStamp.stampIn();       /* Dont time the above getposition, directsound falsely stalls, probably on a criticalsection */

    pcm /= blocksize;
    pcm %= numblocks;

    FLOG((FMOD_DEBUG_TYPE_THREAD, __FILE__, __LINE__, "OutputPolled::updateThread", "PCM %8d fillblock %8d\n", pcm, mCursorBlock));

    while (mCursorBlock != (int)pcm)
    {
        void        *ptr1 = 0, *ptr2 = 0;
        unsigned int len1 = 0, len2 = 0;
        unsigned int offbytes = 0, lenbytes = 0, numsamples;
        int          block = mCursorBlock;

        block -= mMixAheadBlocks;
        if (block < 0)
        {
            block += numblocks;
        }

        result = SoundI::getBytesFromSamples(blocksize, &lenbytes, outputchannels, outputformat);
        if (result != FMOD_OK)
        {
            return result;
        }
        result = SoundI::getBytesFromSamples(block * blocksize, &offbytes, outputchannels, outputformat);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (mDescription.lock)
        {
            result = mDescription.lock(this, offbytes, lenbytes, &ptr1, &ptr2, &len1, &len2);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        result = SoundI::getSamplesFromBytes(len1, &numsamples, outputchannels, outputformat);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = mix(ptr1, numsamples);
        if (result != FMOD_OK)
        {
            return result;
        }

        #ifdef DUMPMIXERTODISK
        if (fp)
        {
            fwrite(ptr1, len1, 1, fp);
        }
        #endif

        /*
            ptr2 and len2 should never be non 0.  All updates are block aligned.
        */
        if (mDescription.unlock)
        {
            result = mDescription.unlock(this, ptr1, ptr2, len1, len2);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        mCursorBlock++;
        if (mCursorBlock >= numblocks)
        {
            mCursorBlock = 0;
        }
    }
#endif

    if (mFinishedSema)
    {
        FMOD_OS_Semaphore_Signal(mFinishedSema, false);
    }

#ifdef FMOD_SUPPORT_SOFTWARE
    mSystem->mDSPTimeStamp.stampOut(95);
#endif

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
    
	[SEE_ALSO]
]
*/
// VERIFY: 50582eff auto
// OFFSET: 0x00922990
FMOD_RESULT OutputPolled::start()
{
    FMOD_RESULT  result;

    if (mSystem->mFlags & FMOD_INIT_SYNCMIXERWITHUPDATE)
    {
        mPolledFromMainThread = true;
    }

    if (mPolledFromMainThread)
    {
        FLOG((FMOD_DEBUG_TYPE_THREAD, __FILE__, __LINE__, "OutputPolled::start", "Starting thread to be triggered from System::update\n"));

        result = initThread("FMOD mixer thread", 0, 0, MIXER_THREADPRIORITY, 0, MIXER_STACKSIZE, true, 0);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = FMOD_OS_Semaphore_Create(&mFinishedSema);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    else
    {
        unsigned int blocksize;
        int          rate;
        float        ms;

        result = mSystem->getDSPBufferSize(&blocksize, 0);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = mSystem->getSoftwareFormat(&rate, 0, 0, 0, 0, 0);
        if (result != FMOD_OK)
        {
            return result;
        }

        ms = (float)blocksize * 1000.0f / (float)rate;
        if (ms < 20)
        {
            ms /= 3;
            if (ms < 1)
            {
                ms = 1;
            }
        }
        else
        {
            ms = 10;
        }

        FLOG((FMOD_DEBUG_TYPE_THREAD, __FILE__, __LINE__, "OutputPolled::start", "Starting thread that automatically wakes up every %d ms\n", (int)ms));

        result = initThread("FMOD mixer thread", 0, 0, MIXER_THREADPRIORITY, 0, MIXER_STACKSIZE, false, (int)ms);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    
#ifdef DUMPMIXERTODISK
    fp = fopen("C:\\media\\fmod4output.raw", "wb");
#endif

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
    
	[SEE_ALSO]
]
*/
// VERIFY: 0df28cba auto
// OFFSET: 0x00922A90
FMOD_RESULT OutputPolled::stop()
{
    FMOD_RESULT result;

    result = closeThread();
    if (result != FMOD_OK)
    {
        return result;
    }

    if (mFinishedSema)
    {
        result = FMOD_OS_Semaphore_Free(mFinishedSema);
        if (result != FMOD_OK)
        {
            return result;
        }
    }


#ifdef DUMPMIXERTODISK
    if (fp)
    {
        fclose(fp);
        fp = 0;
    }
#endif

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
    
	[SEE_ALSO]
]
*/


}
