/*$ preserve start $*/

#include "fmod_settings.h"

#include "fmod_async.h"
#include "fmod_soundi.h"

namespace FMOD
{
/*$ preserve end $*/


// VERIFY: 73f8d2c1 auto
// OFFSET: 0x008D1940
FMOD_RESULT Sound::readyForRelease(bool *ready)
{
    FMOD_RESULT result;
    SoundI *soundi;

    *ready = true;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }

    *ready = soundi->readyForRelease();

    return FMOD_OK;
}


// VERIFY: e733cc13 auto
// OFFSET: 0x008D1970
FMOD_RESULT Sound::release()
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundi->release();
    }
}


FMOD_RESULT Sound::getSystemObject(System **system)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundi->getSystemObject(system);
    }
}


// VERIFY: fa4f49f2 auto
// OFFSET: 0x008D19A0
FMOD_RESULT Sound::lock(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->lock(offset, length, ptr1, ptr2, len1, len2);
    }
}


// VERIFY: 7f312a2b auto
// OFFSET: 0x008D19F0
FMOD_RESULT Sound::unlock(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->unlock(ptr1, ptr2, len1, len2);
    }
}


FMOD_RESULT Sound::setDefaults(float frequency, float volume, float pan, int priority)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->setDefaults(frequency, volume, pan, priority);
    }
}


FMOD_RESULT Sound::getDefaults(float *frequency, float *volume, float *pan, int *priority)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getDefaults(frequency, volume, pan, priority);
    }
}


FMOD_RESULT Sound::setVariations(float frequencyvar, float volumevar, float panvar)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->setVariations(frequencyvar, volumevar, panvar);
    }
}


FMOD_RESULT Sound::getVariations(float *frequencyvar, float *volumevar, float *panvar)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getVariations(frequencyvar, volumevar, panvar);
    }
}


FMOD_RESULT Sound::set3DMinMaxDistance(float min, float max)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->set3DMinMaxDistance(min, max);
    }
}


FMOD_RESULT Sound::get3DMinMaxDistance(float *min, float *max)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->get3DMinMaxDistance(min, max);
    }
}


FMOD_RESULT Sound::set3DConeSettings(float insideconeangle, float outsideconeangle, float outsidevolume)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->set3DConeSettings(insideconeangle, outsideconeangle, outsidevolume);
    }
}


FMOD_RESULT Sound::get3DConeSettings(float *insideconeangle, float *outsideconeangle, float *outsidevolume)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->get3DConeSettings(insideconeangle, outsideconeangle, outsidevolume);
    }
}


FMOD_RESULT Sound::set3DCustomRolloff(FMOD_VECTOR *points, int numpoints)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->set3DCustomRolloff(points, numpoints);
    }
}


FMOD_RESULT Sound::get3DCustomRolloff(FMOD_VECTOR **points, int *numpoints)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->get3DCustomRolloff(points, numpoints);
    }
}


FMOD_RESULT Sound::setSubSound(int index, Sound *subsound)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->setSubSound(index, (SoundI *)subsound);
    }
}


FMOD_RESULT Sound::getSubSound(int index, Sound **subsound)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        /*
            If the async state was FMOD_ERR_FILE_DISKEJECTED, we need to let it do another
            disc access to know if it was re-inserted or not.
        */
        FMOD_RESULT asyncresult = soundi->mAsyncData ? soundi->mAsyncData->mResult : FMOD_OK;

        if (soundi->mOpenState != FMOD_OPENSTATE_READY && asyncresult != FMOD_ERR_FILE_DISKEJECTED)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getSubSound(index, (SoundI **)subsound);
    }
}


FMOD_RESULT Sound::setSubSoundSentence(int *subsoundlist, int numsubsounds)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->setSubSoundSentence(subsoundlist, numsubsounds);
    }
}


// VERIFY: b01c64aa auto
// OFFSET: 0x008D1A40
FMOD_RESULT Sound::getName(char *name, int namelen)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getName(name, namelen);
    }
}


// VERIFY: 20e7e639 auto
// OFFSET: 0x008D1A80
FMOD_RESULT Sound::getLength(unsigned int *length, FMOD_TIMEUNIT lengthtype)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getLength(length, lengthtype);
    }
}


FMOD_RESULT Sound::getFormat(FMOD_SOUND_TYPE *type, FMOD_SOUND_FORMAT *format, int *channels, int *bits)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getFormat(type, format, channels, bits);
    }
}


FMOD_RESULT Sound::getNumSubSounds(int *numsubsounds)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getNumSubSounds(numsubsounds);
    }
}


FMOD_RESULT Sound::getNumTags(int *numtags, int *numtagsupdated)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getNumTags(numtags, numtagsupdated);
    }
}


FMOD_RESULT Sound::getTag(const char *name, int index, FMOD_TAG *tag)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getTag(name, index, tag);
    }
}


FMOD_RESULT Sound::getOpenState(FMOD_OPENSTATE *openstate, unsigned int *percentbuffered, bool *starving)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundi->getOpenState(openstate, percentbuffered, starving);
    }
}


FMOD_RESULT Sound::readData(void *buffer, unsigned int lenbytes, unsigned int *read)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->readData(buffer, lenbytes, read);
    }
}


FMOD_RESULT Sound::seekData(unsigned int pcm)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->seekData(pcm);
    }
}


// VERIFY: c742f956 auto
// OFFSET: 0x008D1AC0
FMOD_RESULT Sound::setSoundGroup(SoundGroup *soundgroup)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->setSoundGroup((SoundGroupI *)soundgroup);
    }
}


FMOD_RESULT Sound::getSoundGroup(SoundGroup **soundgroup)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getSoundGroup((SoundGroupI **)soundgroup);
    }
}


FMOD_RESULT Sound::getNumSyncPoints(int *numsyncpoints)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getNumSyncPoints(numsyncpoints);
    }
}


FMOD_RESULT Sound::getSyncPoint(int index, FMOD_SYNCPOINT **point)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getSyncPoint(index, point);
    }
}


FMOD_RESULT Sound::getSyncPointInfo(FMOD_SYNCPOINT *point, char *name, int namelen, unsigned int *offset, FMOD_TIMEUNIT offsettype)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getSyncPointInfo(point, name, namelen, offset, offsettype);
    }
}


FMOD_RESULT Sound::addSyncPoint(unsigned int offset, FMOD_TIMEUNIT offsettype, const char *name, FMOD_SYNCPOINT **point)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->addSyncPoint(offset, offsettype, name, point);
    }
}


FMOD_RESULT Sound::deleteSyncPoint(FMOD_SYNCPOINT *point)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->deleteSyncPoint(point);
    }
}


FMOD_RESULT Sound::setMode(FMOD_MODE mode)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->setMode(mode);
    }
}


FMOD_RESULT Sound::getMode(FMOD_MODE *mode)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getMode(mode);
    }
}


FMOD_RESULT Sound::setLoopCount(int loopcount)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->setLoopCount(loopcount);
    }
}


FMOD_RESULT Sound::getLoopCount(int *loopcount)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getLoopCount(loopcount);
    }
}


FMOD_RESULT Sound::setLoopPoints(unsigned int loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int loopend, FMOD_TIMEUNIT loopendtype)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->setLoopPoints(loopstart, loopstarttype, loopend, loopendtype);
    }
}


FMOD_RESULT Sound::getLoopPoints(unsigned int *loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int *loopend, FMOD_TIMEUNIT loopendtype)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->getLoopPoints(loopstart, loopstarttype, loopend, loopendtype);
    }
}
FMOD_RESULT Sound::setUserData(void *_userdata)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        if (soundi->mOpenState != FMOD_OPENSTATE_READY)
        {
            return FMOD_ERR_NOTREADY;
        }
        return soundi->setUserData(_userdata);
    }
}


// VERIFY: 9373ca1d auto
// OFFSET: 0x008D1B00
FMOD_RESULT Sound::getUserData(void **_userdata)
{
    FMOD_RESULT result;
    SoundI *soundi;

    result = SoundI::validate(this, &soundi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return soundi->getUserData(_userdata);
    }
}



/*$ preserve start $*/
}
/*$ preserve end $*/
