/*$ preserve start $*/

#include "fmod_settings.h"

#include "fmod.hpp"
#include "fmod_channelgroupi.h"
#include "fmod_systemi.h"

namespace FMOD
{
/*$ preserve end $*/


// VERIFY: 2d265276 auto
// OFFSET: 0x008D1BF0
FMOD_RESULT ChannelGroup::release()
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->release();
    }
}


FMOD_RESULT ChannelGroup::getSystemObject(System **system)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getSystemObject(system);
    }
}


// VERIFY: 0461c9ed auto
// OFFSET: 0x008D1C20
FMOD_RESULT ChannelGroup::setVolume(float volume)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->setVolume(volume);
    }
}


FMOD_RESULT ChannelGroup::getVolume(float *volume)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getVolume(volume);
    }
}


FMOD_RESULT ChannelGroup::setPitch(float pitch)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->setPitch(pitch);
    }
}


FMOD_RESULT ChannelGroup::getPitch(float *pitch)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getPitch(pitch);
    }
}


FMOD_RESULT ChannelGroup::set3DOcclusion(float directocclusion, float reverbocclusion)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->set3DOcclusion(directocclusion, reverbocclusion);
    }
}


FMOD_RESULT ChannelGroup::get3DOcclusion(float *directocclusion, float *reverbocclusion)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->get3DOcclusion(directocclusion, reverbocclusion);
    }
}


FMOD_RESULT ChannelGroup::setPaused(bool paused)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->setPaused(paused);
    }
}


FMOD_RESULT ChannelGroup::getPaused(bool *paused)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getPaused(paused);
    }
}


FMOD_RESULT ChannelGroup::setMute(bool mute)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->setMute(mute);
    }
}


FMOD_RESULT ChannelGroup::getMute(bool *mute)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getMute(mute);
    }
}


FMOD_RESULT ChannelGroup::stop()
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->stop();
    }
}


FMOD_RESULT ChannelGroup::overrideVolume(float volume)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->overrideVolume(volume);
    }
}


FMOD_RESULT ChannelGroup::overrideFrequency(float frequency)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->overrideFrequency(frequency);
    }
}


FMOD_RESULT ChannelGroup::overridePan(float pan)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->overridePan(pan);
    }
}


FMOD_RESULT ChannelGroup::overrideReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->overrideReverbProperties(prop);
    }
}


FMOD_RESULT ChannelGroup::override3DAttributes(const FMOD_VECTOR *pos, const FMOD_VECTOR *vel)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->override3DAttributes(pos, vel);
    }
}


FMOD_RESULT ChannelGroup::overrideSpeakerMix(float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->overrideSpeakerMix(frontleft, frontright, center, lfe, backleft, backright, sideleft, sideright);
    }
}


FMOD_RESULT ChannelGroup::addGroup(ChannelGroup *group)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->addGroup((ChannelGroupI *)group);
    }
}


FMOD_RESULT ChannelGroup::getNumGroups(int *numgroups)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getNumGroups(numgroups);
    }
}


FMOD_RESULT ChannelGroup::getGroup(int index, ChannelGroup **group)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getGroup(index, (ChannelGroupI **)group);
    }
}


FMOD_RESULT ChannelGroup::getParentGroup(ChannelGroup **group)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getParentGroup((ChannelGroupI **)group);
    }
}


// VERIFY: 7f4eab1b auto
// OFFSET: 0x008D1C50
FMOD_RESULT ChannelGroup::getDSPHead(DSP **dsp)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getDSPHead((DSPI **)dsp);
    }
}


FMOD_RESULT ChannelGroup::addDSP(DSP *dsp)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->addDSP((DSPI *)dsp);
    }
}


FMOD_RESULT ChannelGroup::getName(char *name, int namelen)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getName(name, namelen);
    }
}


// VERIFY: c51b7376 auto
// OFFSET: 0x008D1C80
FMOD_RESULT ChannelGroup::getNumChannels(int *numchannels)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getNumChannels(numchannels);
    }
}


FMOD_RESULT ChannelGroup::getChannel(int index, Channel **channel)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getChannel(index, (Channel **)channel);
    }
}


FMOD_RESULT ChannelGroup::getSpectrum(float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getSpectrum(spectrumarray, numvalues, channeloffset, windowtype);
    }
}


FMOD_RESULT ChannelGroup::getWaveData(float *wavearray, int numvalues, int channeloffset)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getWaveData(wavearray, numvalues, channeloffset);
    }
}


FMOD_RESULT ChannelGroup::setUserData(void *_userdata)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->setUserData(_userdata);
    }
}


FMOD_RESULT ChannelGroup::getUserData(void **_userdata)
{
    FMOD_RESULT result;
    ChannelGroupI *channelgroupi;

    result = ChannelGroupI::validate(this, &channelgroupi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return channelgroupi->getUserData(_userdata);
    }
}



/*$ preserve start $*/
}
/*$ preserve end $*/
