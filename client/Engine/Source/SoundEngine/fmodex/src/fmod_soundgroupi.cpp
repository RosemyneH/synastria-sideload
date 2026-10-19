#include "fmod_settings.h"

#include "fmod_soundgroupi.h"
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
FMOD_RESULT SoundGroupI::validate(SoundGroup *soundgroup, SoundGroupI **soundgroupi)
{
    FMOD::SoundGroupI *sg = (FMOD::SoundGroupI *)soundgroup;

    if (!soundgroup)
    {
        return FMOD_ERR_INVALID_HANDLE;
    }

    if (!soundgroupi)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *soundgroupi = sg;

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

// VERIFY: dc7d9263 auto
// OFFSET: 0x008E47E0
SoundGroupI::SoundGroupI()
{  
    mFadeSpeed = 0.0f;
    mMaxAudible = -1;
    mVolume = 1.0f;
    mName = 0;
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

// VERIFY: f6011bec auto
// OFFSET: 0x008E4810
FMOD_RESULT SoundGroupI::release()
{  
    if (this == mSystem->mSoundGroup)
    {
        return FMOD_ERR_INVALID_HANDLE;
    }

    /*
        First move all sounds back to the main group
    */
    if (mSystem->mSoundGroup)
    {
        while (mSoundHead.getNext() != &mSoundHead)
        {
            SoundI *sound = (SoundI *)mSoundHead.getNext()->getData();
            sound->setSoundGroup(mSystem->mSoundGroup);
        }
    }

    return releaseInternal();
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
// VERIFY: 2ee25a3c auto
// OFFSET: 0x008E46D0
FMOD_RESULT SoundGroupI::releaseInternal()
{  
    if (mName)
    {
        FMOD_Memory_Free(mName);
    }

    removeNode();

    FMOD_Memory_Free(this);

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
FMOD_RESULT SoundGroupI::getSystemObject(System **system)
{
    if (!system)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *system = (System *)mSystem;

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
// VERIFY: 407520ce auto
// OFFSET: 0x008E4730
FMOD_RESULT SoundGroupI::setMaxAudible(int maxaudible)
{
    if (maxaudible < -1)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    mMaxAudible = maxaudible;

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
FMOD_RESULT SoundGroupI::getMaxAudible(int *maxaudible)
{
    if (!maxaudible)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *maxaudible = mMaxAudible;

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

// VERIFY: cee20409 auto
// OFFSET: 0x008E4750
FMOD_RESULT SoundGroupI::setMaxAudibleBehavior(FMOD_SOUNDGROUP_BEHAVIOR behavior)
{
    if (behavior < 0 || behavior >= FMOD_SOUNDGROUP_BEHAVIOR_MAX)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    mMaxAudibleBehavior = behavior;

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
FMOD_RESULT SoundGroupI::getMaxAudibleBehavior(FMOD_SOUNDGROUP_BEHAVIOR *behavior)
{
    if (!behavior)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *behavior = mMaxAudibleBehavior;

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

// VERIFY: 2529fbb8 auto
// OFFSET: 0x008E4770
FMOD_RESULT SoundGroupI::setMuteFadeSpeed(float speed)
{
    if (speed < 0)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    mFadeSpeed = speed;

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
FMOD_RESULT SoundGroupI::getMuteFadeSpeed(float *speed)
{
    if (!speed)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *speed = mFadeSpeed;

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
FMOD_RESULT SoundGroupI::setVolume(float volume)
{
    LinkedListNode *current;

    if (volume < 0)
    {
        volume = 0;
    }
    if (volume > 1)
    {
        volume = 1;
    }

    mVolume = volume;

    for (current = mSoundHead.getNext(); current != &mSoundHead; current = current->getNext())
    {
        ChannelI *channeli;
        SoundI *sound = (SoundI *)current->getData();

        for (channeli = SAFE_CAST(ChannelI, mSystem->mChannelUsedListHead.getNext()); channeli != &mSystem->mChannelUsedListHead; channeli = SAFE_CAST(ChannelI, channeli->getNext()))
        {
            if (channeli->mRealChannel[0])
            {
                SoundI *channelsound = 0;

                channeli->getCurrentSound(&channelsound);
                if (channelsound == sound)
                {
                    channeli->setVolume(channeli->mVolume);
                }
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
FMOD_RESULT SoundGroupI::getVolume(float *volume)
{
    if (!volume)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *volume = mVolume;

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
FMOD_RESULT SoundGroupI::stop()
{
    LinkedListNode *current;

    for (current = mSoundHead.getNext(); current != &mSoundHead; current = current->getNext())
    {
        SoundI *sound = (SoundI *)current->getData();

        mSystem->stopSound(sound);
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
FMOD_RESULT SoundGroupI::getName(char *name, int namelen)
{
    if (!name)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (namelen > FMOD_STRING_MAXNAMELEN)
    {
        namelen = FMOD_STRING_MAXNAMELEN;
    }

    if (mName)
    {
        FMOD_strncpy(name, mName, namelen);
    }
    else
    {
        FMOD_strncpy(name, "(null)", namelen);
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
FMOD_RESULT SoundGroupI::getNumSounds(int *numsounds)
{
    if (!numsounds)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *numsounds = mSoundHead.count();

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
FMOD_RESULT SoundGroupI::getSound(int index, Sound **sound)
{
    int count = 0;
    LinkedListNode *current;

    if (!sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *sound = 0;

    current = mSoundHead.getNext();

    while (current != &mSoundHead)
    {
        if (count == index)
        {
            *sound = (Sound *)((SoundI *)current->getData());
            return FMOD_OK;
        }

        count++;
        current = current->getNext();
    }

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
// VERIFY: 35ad500d manual
// OFFSET: 0x008E47A0
FMOD_RESULT SoundGroupI::getNumPlaying(int *numplaying)
{
    LinkedListNode *current;

    if (!numplaying)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    current = mSoundHead.getNext();
    
    *numplaying = 0;

    while (current != &mSoundHead)
    {
        SoundI *soundi = (SoundI *)current->getData();

        *numplaying += soundi->mNumAudible;

        current = current->getNext();
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
FMOD_RESULT SoundGroupI::setUserData(void *userdata)
{
    mUserData = userdata;

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
FMOD_RESULT SoundGroupI::getUserData(void **userdata)
{
    if (!userdata)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *userdata = mUserData;

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
