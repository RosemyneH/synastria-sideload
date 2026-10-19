/*$ preserve start $*/

#include "fmod_settings.h"
#include "fmod_systemi.h"
#include "fmod_cmdlog.h"

namespace FMOD
{


//AJS
/*AJS
#ifdef FMOD_SUPPORT_CMDLOG
    result = FMOD_CmdLog_Release();
    if (result != FMOD_OK)
    {
        return result;
    }
#endif
AJS*/

/*$ preserve end $*/


// VERIFY: 651e68d2 auto
// OFFSET: 0x008D1150
FMOD_RESULT System::release()
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->release();
    }
}


// VERIFY: 94aa1aa9 auto
// OFFSET: 0x008D1180
FMOD_RESULT System::setOutput(FMOD_OUTPUTTYPE output)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setOutput(output);
    }
}


// VERIFY: 98ba3f2e auto
// OFFSET: 0x008D11B0
FMOD_RESULT System::getOutput(FMOD_OUTPUTTYPE *output)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getOutput(output);
    }
}


// VERIFY: ae9c45a4 auto
// OFFSET: 0x008D11E0
FMOD_RESULT System::getNumDrivers(int *numdrivers)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getNumDrivers(numdrivers);
    }
}


// VERIFY: 790a73b3 auto
// OFFSET: 0x008D1210
FMOD_RESULT System::getDriverName(int id, char *name, int namelen)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getDriverName(id, name, namelen);
    }
}
// VERIFY: e08da19b auto
// OFFSET: 0x008D1240
FMOD_RESULT System::getDriverCaps(int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getDriverCaps(id, caps, minfrequency, maxfrequency, controlpanelspeakermode);
    }
}


// VERIFY: ac0b609f auto
// OFFSET: 0x008D1280
FMOD_RESULT System::setDriver(int driver)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setDriver(driver);
    }
}


FMOD_RESULT System::getDriver(int *driver)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getDriver(driver);
    }
}


// VERIFY: da4a2dbf auto
// OFFSET: 0x008D12B0
FMOD_RESULT System::setHardwareChannels(int min2d, int max2d, int min3d, int max3d)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setHardwareChannels(min2d, max2d, min3d, max3d);
    }
}


// VERIFY: 9454cbbf auto
// OFFSET: 0x008D12F0
FMOD_RESULT System::setSoftwareChannels(int numsoftwarechannels)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setSoftwareChannels(numsoftwarechannels);
    }
}


FMOD_RESULT System::getSoftwareChannels(int *numsoftwarechannels)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getSoftwareChannels(numsoftwarechannels);
    }
}


// VERIFY: b218b21a auto
// OFFSET: 0x008D1320
FMOD_RESULT System::setSoftwareFormat(int samplerate, FMOD_SOUND_FORMAT format, int numoutputchannels, int maxinputchannels, FMOD_DSP_RESAMPLER resamplemethod)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setSoftwareFormat(samplerate, format, numoutputchannels, maxinputchannels, resamplemethod);
    }
}


FMOD_RESULT System::getSoftwareFormat(int *samplerate, FMOD_SOUND_FORMAT *format, int *numoutputchannels, int *maxinputchannels, FMOD_DSP_RESAMPLER *resamplemethod, int *bits)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getSoftwareFormat(samplerate, format, numoutputchannels, maxinputchannels, resamplemethod, bits);
    }
}


// VERIFY: 329ad2b0 auto
// OFFSET: 0x008D1360
FMOD_RESULT System::setDSPBufferSize(unsigned int bufferlength, int numbuffers)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setDSPBufferSize(bufferlength, numbuffers);
    }
}


FMOD_RESULT System::getDSPBufferSize(unsigned int *bufferlength, int *numbuffers)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getDSPBufferSize(bufferlength, numbuffers);
    }
}


FMOD_RESULT System::setFileSystem(FMOD_FILE_OPENCALLBACK useropen, FMOD_FILE_CLOSECALLBACK userclose, FMOD_FILE_READCALLBACK userread, FMOD_FILE_SEEKCALLBACK userseek, int blockalign)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setFileSystem(useropen, userclose, userread, userseek, blockalign);
    }
}


FMOD_RESULT System::attachFileSystem(FMOD_FILE_OPENCALLBACK useropen, FMOD_FILE_CLOSECALLBACK userclose, FMOD_FILE_READCALLBACK userread, FMOD_FILE_SEEKCALLBACK userseek)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->attachFileSystem(useropen, userclose, userread, userseek);
    }
}


FMOD_RESULT System::setAdvancedSettings(FMOD_ADVANCEDSETTINGS *settings)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setAdvancedSettings(settings);
    }
}


FMOD_RESULT System::getAdvancedSettings(FMOD_ADVANCEDSETTINGS *settings)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getAdvancedSettings(settings);
    }
}


// VERIFY: b097c977 auto
// OFFSET: 0x008D1390
FMOD_RESULT System::setSpeakerMode(FMOD_SPEAKERMODE speakermode)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setSpeakerMode(speakermode);
    }
}


FMOD_RESULT System::getSpeakerMode(FMOD_SPEAKERMODE *speakermode)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getSpeakerMode(speakermode);
    }
}


// VERIFY: 692cc11c auto
// OFFSET: 0x008D13C0
FMOD_RESULT System::setCallback(FMOD_SYSTEM_CALLBACKTYPE type, FMOD_SYSTEM_CALLBACK callback)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setCallback(type, callback);
    }
}


FMOD_RESULT System::setPluginPath(const char *path)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setPluginPath(path);
    }
}


FMOD_RESULT System::loadPlugin(const char *filename, FMOD_PLUGINTYPE *plugintype, int *index)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->loadPlugin(filename, plugintype, index);
    }
}


FMOD_RESULT System::unloadPlugin(FMOD_PLUGINTYPE plugintype, int index)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->unloadPlugin(plugintype, index);
    }
}


FMOD_RESULT System::getNumPlugins(FMOD_PLUGINTYPE plugintype, int *numplugins)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getNumPlugins(plugintype, numplugins);
    }
}
FMOD_RESULT System::getPluginInfo(FMOD_PLUGINTYPE plugintype, int index, char *name, int namelen, unsigned int *version)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getPluginInfo(plugintype, index, name, namelen, version);
    }
}


FMOD_RESULT System::setOutputByPlugin(int index)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setOutputByPlugin(index);
    }
}


FMOD_RESULT System::getOutputByPlugin(int *index)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getOutputByPlugin(index);
    }
}
FMOD_RESULT System::createCodec(FMOD_CODEC_DESCRIPTION *description)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->createCodec(description);
    }
}


// VERIFY: e906e2a7 auto
// OFFSET: 0x008D13F0
FMOD_RESULT System::init(int maxchannels, FMOD_INITFLAGS flags, void *extradriverdata)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->init(maxchannels, flags, extradriverdata);
    }
}


// VERIFY: a2ac2398 auto
// OFFSET: 0x008D1420
FMOD_RESULT System::close()
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->close();
    }
}


// VERIFY: f0a0adcb auto
// OFFSET: 0x008D1450
FMOD_RESULT System::update()
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->update();
    }
}


// VERIFY: 513e699c auto
// OFFSET: 0x008D1480
FMOD_RESULT System::set3DSettings(float dopplerscale, float distancefactor, float rolloffscale)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->set3DSettings(dopplerscale, distancefactor, rolloffscale);
    }
}


FMOD_RESULT System::get3DSettings(float *dopplerscale, float *distancefactor, float *rolloffscale)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->get3DSettings(dopplerscale, distancefactor, rolloffscale);
    }
}


FMOD_RESULT System::set3DNumListeners(int numlisteners)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->set3DNumListeners(numlisteners);
    }
}


FMOD_RESULT System::get3DNumListeners(int *numlisteners)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->get3DNumListeners(numlisteners);
    }
}


// VERIFY: 4c657a45 auto
// OFFSET: 0x008D14C0
FMOD_RESULT System::set3DListenerAttributes(int listener, const FMOD_VECTOR *pos, const FMOD_VECTOR *vel, const FMOD_VECTOR *forward, const FMOD_VECTOR *up)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->set3DListenerAttributes(listener, pos, vel, forward, up);
    }
}


// VERIFY: c8e2f8fe auto
// OFFSET: 0x008D1500
FMOD_RESULT System::get3DListenerAttributes(int listener, FMOD_VECTOR *pos, FMOD_VECTOR *vel, FMOD_VECTOR *forward, FMOD_VECTOR *up)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->get3DListenerAttributes(listener, pos, vel, forward, up);
    }
}


// VERIFY: 47b404c9 auto
// OFFSET: 0x008D1540
FMOD_RESULT System::set3DRolloffCallback(FMOD_3D_ROLLOFFCALLBACK callback)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->set3DRolloffCallback(callback);
    }
}


FMOD_RESULT System::set3DSpeakerPosition(FMOD_SPEAKER speaker, float x, float y, bool active)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->set3DSpeakerPosition(speaker, x, y, active);
    }
}


FMOD_RESULT System::get3DSpeakerPosition(FMOD_SPEAKER speaker, float *x, float *y, bool *active)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->get3DSpeakerPosition(speaker, x, y, active);
    }
}


FMOD_RESULT System::setStreamBufferSize(unsigned int filebuffersize, FMOD_TIMEUNIT filebuffersizetype)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setStreamBufferSize(filebuffersize, filebuffersizetype);
    }
}


FMOD_RESULT System::getStreamBufferSize(unsigned int *filebuffersize, FMOD_TIMEUNIT *filebuffersizetype)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getStreamBufferSize(filebuffersize, filebuffersizetype);
    }
}


// VERIFY: 51948ab9 auto
// OFFSET: 0x008D1570
FMOD_RESULT System::getVersion(unsigned int *version)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getVersion(version);
    }
}


FMOD_RESULT System::getOutputHandle(void **handle)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getOutputHandle(handle);
    }
}


FMOD_RESULT System::getChannelsPlaying(int *channels)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getChannelsPlaying(channels);
    }
}


// VERIFY: c0d4225c auto
// OFFSET: 0x008D15A0
FMOD_RESULT System::getHardwareChannels(int *num2d, int *num3d, int *total)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getHardwareChannels(num2d, num3d, total);
    }
}


FMOD_RESULT System::getCPUUsage(float *dsp, float *stream, float *update, float *total)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getCPUUsage(dsp, stream, update, total);
    }
}


FMOD_RESULT System::getSoundRAM(int *currentalloced, int *maxalloced, int *total)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getSoundRAM(currentalloced, maxalloced, total);
    }
}


FMOD_RESULT System::getNumCDROMDrives(int *numdrives)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getNumCDROMDrives(numdrives);
    }
}


FMOD_RESULT System::getCDROMDriveName(int drive, char *drivename, int drivenamelen, char *scsiname, int scsinamelen, char *devicename, int devicenamelen)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getCDROMDriveName(drive, drivename, drivenamelen, scsiname, scsinamelen, devicename, devicenamelen);
    }
}


FMOD_RESULT System::getSpectrum(float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getSpectrum(spectrumarray, numvalues, channeloffset, windowtype);
    }
}


FMOD_RESULT System::getWaveData(float *wavearray, int numvalues, int channeloffset)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getWaveData(wavearray, numvalues, channeloffset);
    }
}


// VERIFY: 007077ed auto
// OFFSET: 0x008D15D0
FMOD_RESULT System::createSound(const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, Sound **sound)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->createSound(name_or_data, mode, exinfo, (SoundI **)sound);
    }
}


// VERIFY: 875dc763 auto
// OFFSET: 0x008D1610
FMOD_RESULT System::createStream(const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, Sound **sound)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->createStream(name_or_data, mode, exinfo, (SoundI **)sound);
    }
}


// VERIFY: 8c44fdc9 auto
// OFFSET: 0x008D1650
FMOD_RESULT System::createDSP(FMOD_DSP_DESCRIPTION *description, DSP **dsp)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->createDSP(description, (DSPI **)dsp);
    }
}


// VERIFY: ed8ff790 auto
// OFFSET: 0x008D1680
FMOD_RESULT System::createDSPByType(FMOD_DSP_TYPE type, DSP **dsp)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->createDSPByType(type, (DSPI **)dsp);
    }
}


FMOD_RESULT System::createDSPByIndex(int index, DSP **dsp)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->createDSPByIndex(index, (DSPI **)dsp);
    }
}


// VERIFY: f2a662bb auto
// OFFSET: 0x008D16B0
FMOD_RESULT System::createChannelGroup(const char *name, ChannelGroup **channelgroup)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->createChannelGroup(name, (ChannelGroupI **)channelgroup);
    }
}


// VERIFY: d0a57c0d auto
// OFFSET: 0x008D16E0
FMOD_RESULT System::createSoundGroup(const char *name, SoundGroup **soundgroup)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->createSoundGroup(name, (SoundGroupI **)soundgroup);
    }
}


FMOD_RESULT System::createReverb(Reverb **reverb)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->createReverb((ReverbI **)reverb);
    }
}


// VERIFY: 57805105 auto
// OFFSET: 0x008D1710
FMOD_RESULT System::playSound(FMOD_CHANNELINDEX channelid, Sound *sound, bool paused, Channel **channel)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->playSound(channelid, (SoundI *)sound, paused, (ChannelI **)channel);
    }
}


// VERIFY: a28dc7c0 auto
// OFFSET: 0x008D1750
FMOD_RESULT System::playDSP(FMOD_CHANNELINDEX channelid, DSP *dsp, bool paused, Channel **channel)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->playDSP(channelid, (DSPI *)dsp, paused, (ChannelI **)channel);
    }
}


// VERIFY: ec755605 auto
// OFFSET: 0x008D1790
FMOD_RESULT System::getChannel(int channelid, Channel **channel)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getChannel(channelid, (ChannelI **)channel);
    }
}


FMOD_RESULT System::getMasterChannelGroup(ChannelGroup **channelgroup)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getMasterChannelGroup((ChannelGroupI **)channelgroup);
    }
}


FMOD_RESULT System::getMasterSoundGroup(SoundGroup **soundgroup)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getMasterSoundGroup((SoundGroupI **)soundgroup);
    }
}


// VERIFY: c7a38e56 auto
// OFFSET: 0x008D17C0
FMOD_RESULT System::setReverbProperties(const FMOD_REVERB_PROPERTIES *prop)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setReverbProperties(prop);
    }
}


FMOD_RESULT System::getReverbProperties(FMOD_REVERB_PROPERTIES *prop)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getReverbProperties(prop);
    }
}


FMOD_RESULT System::setReverbAmbientProperties(FMOD_REVERB_PROPERTIES *prop)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setReverbAmbientProperties(prop);
    }
}


FMOD_RESULT System::getReverbAmbientProperties(FMOD_REVERB_PROPERTIES *prop)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getReverbAmbientProperties(prop);
    }
}


FMOD_RESULT System::getDSPHead(DSP **dsp)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getDSPHead((DSPI **)dsp);
    }
}


FMOD_RESULT System::addDSP(DSP *dsp)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->addDSP((DSPI *)dsp);
    }
}


FMOD_RESULT System::lockDSP()
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->lockDSP();
    }
}


FMOD_RESULT System::unlockDSP()
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->unlockDSP();
    }
}


// VERIFY: aabe8a88 auto
// OFFSET: 0x008D17F0
FMOD_RESULT System::setRecordDriver(int driver)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setRecordDriver(driver);
    }
}


FMOD_RESULT System::getRecordDriver(int *driver)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getRecordDriver(driver);
    }
}
// VERIFY: d188f42d auto
// OFFSET: 0x008D1820
FMOD_RESULT System::getRecordNumDrivers(int *numdrivers)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getRecordNumDrivers(numdrivers);
    }
}


// VERIFY: bdbcd8f4 auto
// OFFSET: 0x008D1850
FMOD_RESULT System::getRecordDriverName(int id, char *name, int namelen)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getRecordDriverName(id, name, namelen);
    }
}
FMOD_RESULT System::getRecordDriverCaps(int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getRecordDriverCaps(id, caps, minfrequency, maxfrequency);
    }
}


// VERIFY: 0c1380a2 auto
// OFFSET: 0x008D1880
FMOD_RESULT System::getRecordPosition(unsigned int *position)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getRecordPosition(position);
    }
}


// VERIFY: f074e908 auto
// OFFSET: 0x008D18B0
FMOD_RESULT System::recordStart(Sound *sound, bool loop)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->recordStart((SoundI *)sound, loop);
    }
}


// VERIFY: 241a223e auto
// OFFSET: 0x008D18E0
FMOD_RESULT System::recordStop()
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->recordStop();
    }
}


// VERIFY: 58efbadb auto
// OFFSET: 0x008D1910
FMOD_RESULT System::isRecording(bool *recording)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->isRecording(recording);
    }
}


FMOD_RESULT System::createGeometry(int maxpolygons, int maxvertices, Geometry **geometry)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->createGeometry(maxpolygons, maxvertices, (GeometryI **)geometry);
    }
}


FMOD_RESULT System::setGeometrySettings(float maxworldsize)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setGeometrySettings(maxworldsize);
    }
}


FMOD_RESULT System::getGeometrySettings(float *maxworldsize)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getGeometrySettings(maxworldsize);
    }
}


FMOD_RESULT System::loadGeometry(const void *data, int datasize, Geometry **geometry)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->loadGeometry(data, datasize, (GeometryI **)geometry);
    }
}
FMOD_RESULT System::setNetworkProxy(const char *proxy)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setNetworkProxy(proxy);
    }
}


FMOD_RESULT System::getNetworkProxy(char *proxy, int proxylen)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getNetworkProxy(proxy, proxylen);
    }
}


FMOD_RESULT System::setNetworkTimeout(int timeout)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setNetworkTimeout(timeout);
    }
}


FMOD_RESULT System::getNetworkTimeout(int *timeout)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getNetworkTimeout(timeout);
    }
}


FMOD_RESULT System::setUserData(void *_userdata)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->setUserData(_userdata);
    }
}


FMOD_RESULT System::getUserData(void **_userdata)
{
    FMOD_RESULT result;
    SystemI *systemi;

    result = SystemI::validate(this, &systemi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return systemi->getUserData(_userdata);
    }
}



/*$ preserve start $*/
}
/*$ preserve end $*/
