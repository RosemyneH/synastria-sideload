#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_output_timer.h"
#include "fmod_systemi.h"


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
// VERIFY: 2e5b9ea5 manual
// OFFSET: 0x0094A7C0
FMOD_RESULT OutputTimer::timerFunc()
{
    FMOD_RESULT         result          = FMOD_OK;
    unsigned int        playPosition    = 0;
    unsigned int        blockSize       = 0;
    int                 numBlocks       = 0;
    FMOD_SOUND_FORMAT   outputFormat    = FMOD_SOUND_FORMAT_NONE;
    int                 outputChannels  = 0;   
    
    // Playing has stopped
    if (!mPlaying)
    {
        return FMOD_OK;
    }

    mSystem->mDSPTimeStamp.stampIn();

    result = mSystem->getDSPBufferSize(&blockSize, &numBlocks);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mSystem->getSoftwareFormat(0, &outputFormat, &outputChannels, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (mDescription.getposition)
    {
        result = mDescription.getposition(this, &playPosition);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    // Determine which block is currently playing
    playPosition /= blockSize;
    playPosition %= numBlocks;

    FLOG((FMOD_DEBUG_TYPE_THREAD, __FILE__, __LINE__, "OutputTimer::timerFunc", "Play block %8d, Fill block %8d\n", playPosition, mFillBlock));

    while (mFillBlock != (int)playPosition)
    {
        void          *ptr1                = NULL;
        void          *ptr2                = NULL;
        unsigned int   len1                = 0;
        unsigned int   len2                = 0;
        unsigned int   writePositionBytes  = 0;
        unsigned int   blockSizeBytes      = 0;
        unsigned int   samplesLocked       = 0;
      
        result = SoundI::getBytesFromSamples(blockSize, &blockSizeBytes, outputChannels, outputFormat);
        if (result != FMOD_OK)
        {
            return result;
        }
        result = SoundI::getBytesFromSamples(mFillBlock * blockSize, &writePositionBytes, outputChannels, outputFormat);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (mDescription.lock)
        {
            result = mDescription.lock(this, writePositionBytes, blockSizeBytes, &ptr1, &ptr2, &len1, &len2);
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        result = SoundI::getSamplesFromBytes(len1, &samplesLocked, outputChannels, outputFormat);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = mix(ptr1, samplesLocked);
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

        mFillBlock++;
        if (mFillBlock >= numBlocks)
        {
            mFillBlock = 0;
        }  
    }

    mSystem->mDSPTimeStamp.stampOut(95);

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
// VERIFY: a57212e4 manual
// OFFSET: 0x0094AC80
FMOD_RESULT OutputTimer::start()
{
    FMOD_RESULT   result       = FMOD_OK;
    unsigned int  blockSize    = 0;
    int           sampleRate   = 0;

    /*
        Calculate the callback time for the mixer
    */
    result = mSystem->getDSPBufferSize(&blockSize, NULL);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mSystem->getSoftwareFormat(&sampleRate, NULL, NULL, NULL, NULL, NULL);
    if (result != FMOD_OK)
    {
        return result;
    }

    mMixerTimerPeriod = (float)blockSize * 1000.0f / (float)sampleRate;
    if (mMixerTimerPeriod < 20)
    {
        mMixerTimerPeriod /= 3;
        if (mMixerTimerPeriod < 1)
        {
            mMixerTimerPeriod = 1;
        }
    }
    else
    {
        mMixerTimerPeriod = 10;
    }

    FLOG((FMOD_DEBUG_TYPE_THREAD, __FILE__, __LINE__, "OutputTimer::start", "Starting timer callback that triggers every %d ms\n", (int)mMixerTimerPeriod));
    mPlaying = true;

    // Mixer update callback critical section
    result = FMOD_OS_CriticalSection_Create(&mMixerCrit);
    if (result != FMOD_OK)
    {
        return result;
    }

    // Setup the timed callback event
    mMixerTimerID = timeSetEvent((unsigned int)mMixerTimerPeriod, 0, (LPTIMECALLBACK)timerFuncCallback, (UINT_PTR)this, TIME_PERIODIC);
    if (!mMixerTimerID)
    {
        FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "OutputTimer::start", "timeSetEvent failed!  Something is really wrong with your PC.  timer period = %d ms\n", (int)mMixerTimerPeriod));
        return FMOD_ERR_OUTPUT_INIT;
    }
    
#ifdef DUMPMIXERTODISK
    fp = fopen("/media/fmod4output.raw", "wb");
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
// VERIFY: 53d6c4a7 auto
// OFFSET: 0x0094A760
FMOD_RESULT OutputTimer::stop()
{
    FMOD_RESULT fResult = FMOD_OK;
    HRESULT     hResult = S_OK;

    // Stop mixer callbacks
    timeKillEvent(mMixerTimerID);
    mMixerTimerID = 0;
    
    // If timer still ticks a few times ensure the callback will do nothing
    mPlaying = false;

    // Ensure that the mixer is finish before returning (wont re-enter because of mPlaying)
    FMOD_OS_CriticalSection_Enter(mMixerCrit);

#ifdef DUMPMIXERTODISK
    if (fp)
    {
        fclose(fp);
        fp = 0;
    }
#endif

    FMOD_OS_CriticalSection_Leave(mMixerCrit);
    FMOD_OS_CriticalSection_Free(mMixerCrit);
    mMixerCrit = NULL;

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
// VERIFY: e5ef8198 auto
// OFFSET: 0x0094AC40
void CALLBACK OutputTimer::timerFuncCallback(UINT uTimerID, UINT uMsg, UINT_PTR dwUser, UINT_PTR dw1, UINT_PTR dw2)
{
    OutputTimer *outputTimer = (OutputTimer *)dwUser;

    FMOD_OS_CriticalSection_Enter(outputTimer->mMixerCrit);
    outputTimer->timerFunc();
    FMOD_OS_CriticalSection_Leave(outputTimer->mMixerCrit);
}

}

#endif