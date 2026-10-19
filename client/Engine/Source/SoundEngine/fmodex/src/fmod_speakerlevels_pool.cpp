#include "fmod_settings.h"

#include "fmod_speakerlevels_pool.h"
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
SpeakerLevelsPool::SpeakerLevelsPool()
{
    mLevelsPool = 0;
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
// VERIFY: 6655e3db manual
// OFFSET: 0x00921A10
FMOD_RESULT SpeakerLevelsPool::alloc(float **levels)
{
    int freepos           = mSystem->mNumChannels;
    int numoutputchannels = mSystem->mMaxOutputChannels;

    if (!mLevelsPool)
    {
        mLevelsPool = (LevelsInfo *)FMOD_Memory_Calloc(sizeof(LevelsInfo) * mSystem->mNumChannels);
        if (!mLevelsPool)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    /*
        If it is PROLOGIC, we need enough room for 6 speakers, even though mMaxOutputChannels is 2
    */
    if (mSystem->mSpeakerMode == FMOD_SPEAKERMODE_PROLOGIC)
    {
        numoutputchannels = 6;
    }

    /*
        Search for already alloced mLevels, but not in use 
    */
    for (int i = 0; i < mSystem->mNumChannels; i++)
    {
        if(!mLevelsPool[i].inuse && mLevelsPool[i].levelsmemory)
        {
            /*
                Use this
            */
            FMOD_memset(mLevelsPool[i].levelsmemory, 0, sizeof(float) * numoutputchannels * mSystem->mMaxInputChannels);
            
            mLevelsPool[i].inuse = true;
            *levels              = mLevelsPool[i].levelsmemory;

            return FMOD_OK;
        }

        if(!mLevelsPool[i].levelsmemory)
        {
            if (i < freepos)
            {
                freepos = i;
            }
        }
    }

    /*
        Didn't find anything, alloc a new one
    */
    mLevelsPool[freepos].levelsmemory = (float *)FMOD_Memory_Calloc(sizeof(float) * numoutputchannels * mSystem->mMaxInputChannels);
    if (!mLevelsPool[freepos].levelsmemory)
    {
        return FMOD_ERR_MEMORY;
    }

    mLevelsPool[freepos].inuse = true;
    *levels                    = mLevelsPool[freepos].levelsmemory;

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
// VERIFY: f3bae20c auto
// OFFSET: 0x00921B50
FMOD_RESULT SpeakerLevelsPool::free(float *levels)
{
    if (mLevelsPool)
    {
        for (int i = 0; i < mSystem->mNumChannels; i++)
        {
            if (mLevelsPool[i].levelsmemory == levels)
            {
                mLevelsPool[i].inuse = false;
                break;
            }
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
// VERIFY: 2c8260d3 auto
// OFFSET: 0x00921BA0
FMOD_RESULT SpeakerLevelsPool::release()
{
    if (mLevelsPool)
    {
        for (int i = 0; i < mSystem->mNumChannels; i++)
        {
            if (mLevelsPool[i].levelsmemory)
            {
                FMOD_Memory_Free(mLevelsPool[i].levelsmemory);
                mLevelsPool[i].levelsmemory = NULL;
            }
        }

        FMOD_Memory_Free(mLevelsPool);
        mLevelsPool = NULL;
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
