/*$ preserve start $*/

#include "fmod_settings.h"

#include "fmod.hpp"
#include "fmod_soundgroupi.h"
#include "fmod_systemi.h"

namespace FMOD
{
/*$ preserve end $*/


// VERIFY: 6f3c3cbd auto
// OFFSET: 0x008D1B30
FMOD_RESULT SoundGroup::release()
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->release();
    }
}


FMOD_RESULT SoundGroup::getSystemObject(System **system)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->getSystemObject(system);
    }
}


// VERIFY: 92b0c617 auto
// OFFSET: 0x008D1B60
FMOD_RESULT SoundGroup::setMaxAudible(int maxaudible)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->setMaxAudible(maxaudible);
    }
}


FMOD_RESULT SoundGroup::getMaxAudible(int *maxaudible)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->getMaxAudible(maxaudible);
    }
}


// VERIFY: f8bad028 auto
// OFFSET: 0x008D1B90
FMOD_RESULT SoundGroup::setMaxAudibleBehavior(FMOD_SOUNDGROUP_BEHAVIOR behavior)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->setMaxAudibleBehavior(behavior);
    }
}


FMOD_RESULT SoundGroup::getMaxAudibleBehavior(FMOD_SOUNDGROUP_BEHAVIOR *behavior)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->getMaxAudibleBehavior(behavior);
    }
}


// VERIFY: b7842b56 auto
// OFFSET: 0x008D1BC0
FMOD_RESULT SoundGroup::setMuteFadeSpeed(float speed)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->setMuteFadeSpeed(speed);
    }
}


FMOD_RESULT SoundGroup::getMuteFadeSpeed(float *speed)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->getMuteFadeSpeed(speed);
    }
}


FMOD_RESULT SoundGroup::setVolume(float volume)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->setVolume(volume);
    }
}


FMOD_RESULT SoundGroup::getVolume(float *volume)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->getVolume(volume);
    }
}


FMOD_RESULT SoundGroup::stop()
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->stop();
    }
}


FMOD_RESULT SoundGroup::getName(char *name, int namelen)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->getName(name, namelen);
    }
}


FMOD_RESULT SoundGroup::getNumSounds(int *numsounds)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->getNumSounds(numsounds);
    }
}


FMOD_RESULT SoundGroup::getSound(int index, Sound **sound)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->getSound(index, (Sound **)sound);
    }
}


FMOD_RESULT SoundGroup::getNumPlaying(int *numplaying)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->getNumPlaying(numplaying);
    }
}


FMOD_RESULT SoundGroup::setUserData(void *_userdata)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->setUserData(_userdata);
    }
}


FMOD_RESULT SoundGroup::getUserData(void **_userdata)
{
    FMOD_RESULT result;
    SoundGroupI *soundgroupi;

    result = SoundGroupI::validate(this, &soundgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundgroupi->getUserData(_userdata);
    }
}



/*$ preserve start $*/
}
/*$ preserve end $*/
