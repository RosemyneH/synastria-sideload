/*$ preserve start $*/

#include "fmod_settings.h"

#include "fmod_dspi.h"

namespace FMOD
{
/*$ preserve end $*/


// VERIFY: 4878e3d8 auto
// OFFSET: 0x008D0760
FMOD_RESULT DSP::release()
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->release();
    }
}


FMOD_RESULT DSP::getSystemObject(System **system)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getSystemObject(system);
    }
}


// VERIFY: c42aab09 auto
// OFFSET: 0x008D0790
FMOD_RESULT DSP::addInput(DSP *target)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->addInput((DSPI *)target);
    }
}


FMOD_RESULT DSP::disconnectFrom(DSP *target)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->disconnectFrom((DSPI *)target);
    }
}


// VERIFY: 0d3d548c auto
// OFFSET: 0x008D07C0
FMOD_RESULT DSP::disconnectAll(bool inputs, bool outputs)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->disconnectAll(inputs, outputs);
    }
}


// VERIFY: 891f5edf auto
// OFFSET: 0x008D07F0
FMOD_RESULT DSP::remove()
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->remove();
    }
}


FMOD_RESULT DSP::getNumInputs(int *numinputs)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getNumInputs(numinputs);
    }
}


FMOD_RESULT DSP::getNumOutputs(int *numoutputs)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getNumOutputs(numoutputs);
    }
}


FMOD_RESULT DSP::getInput(int index, DSP **input)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getInput(index, (DSPI **)input);
    }
}


// VERIFY: bd20ae4b auto
// OFFSET: 0x008D0820
FMOD_RESULT DSP::getOutput(int index, DSP **output)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getOutput(index, (DSPI **)output);
    }
}


FMOD_RESULT DSP::setInputMix(int index, float volume)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->setInputMix(index, volume);
    }
}


FMOD_RESULT DSP::getInputMix(int index, float *volume)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getInputMix(index, volume);
    }
}


FMOD_RESULT DSP::setInputLevels(int index, FMOD_SPEAKER speaker, float *levels, int numlevels)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->setInputLevels(index, speaker, levels, numlevels);
    }
}


FMOD_RESULT DSP::getInputLevels(int index, FMOD_SPEAKER speaker, float *levels, int numlevels)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getInputLevels(index, speaker, levels, numlevels);
    }
}


FMOD_RESULT DSP::setOutputMix(int index, float volume)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->setOutputMix(index, volume);
    }
}


FMOD_RESULT DSP::getOutputMix(int index, float *volume)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getOutputMix(index, volume);
    }
}


FMOD_RESULT DSP::setOutputLevels(int index, FMOD_SPEAKER speaker, float *levels, int numlevels)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->setOutputLevels(index, speaker, levels, numlevels);
    }
}


FMOD_RESULT DSP::getOutputLevels(int index, FMOD_SPEAKER speaker, float *levels, int numlevels)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getOutputLevels(index, speaker, levels, numlevels);
    }
}


// VERIFY: bf1dce1b auto
// OFFSET: 0x008D0850
FMOD_RESULT DSP::setActive(bool active)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->setActive(active);
    }
}


FMOD_RESULT DSP::getActive(bool *active)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getActive(active);
    }
}


// VERIFY: c3101a45 auto
// OFFSET: 0x008D0890
FMOD_RESULT DSP::setBypass(bool bypass)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->setBypass(bypass);
    }
}


FMOD_RESULT DSP::getBypass(bool *bypass)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getBypass(bypass);
    }
}
FMOD_RESULT DSP::reset()
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->reset();
    }
}


// VERIFY: adc76eff auto
// OFFSET: 0x008D08D0
FMOD_RESULT DSP::setParameter(int index, float value)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->setParameter(index, value);
    }
}


FMOD_RESULT DSP::getParameter(int index, float *value, char *valuestr, int valuestrlen)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getParameter(index, value, valuestr, valuestrlen);
    }
}


FMOD_RESULT DSP::getNumParameters(int *numparams)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getNumParameters(numparams);
    }
}


FMOD_RESULT DSP::getParameterInfo(int index, char *name, char *label, char *description, int descriptionlen, float *min, float *max)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getParameterInfo(index, name, label, description, descriptionlen, min, max);
    }
}


FMOD_RESULT DSP::showConfigDialog(void *hwnd, bool show)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->showConfigDialog(hwnd, show);
    }
}


FMOD_RESULT DSP::getInfo(char *name, unsigned int *version, int *channels, int *configwidth, int *configheight)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getInfo(name, version, channels, configwidth, configheight);
    }
}


FMOD_RESULT DSP::getType(FMOD_DSP_TYPE *type)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getType(type);
    }
}


FMOD_RESULT DSP::setDefaults(float frequency, float volume, float pan, int priority)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->setDefaults(frequency, volume, pan, priority);
    }
}


FMOD_RESULT DSP::getDefaults(float *frequency, float *volume, float *pan, int *priority)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getDefaults(frequency, volume, pan, priority);
    }
}


FMOD_RESULT DSP::setUserData(void *_userdata)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->setUserData(_userdata);
    }
}


// VERIFY: e6432cda auto
// OFFSET: 0x008D0900
FMOD_RESULT DSP::getUserData(void **_userdata)
{
    FMOD_RESULT result;
    DSPI *dspi;

    result = DSPI::validate(this, &dspi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return dspi->getUserData(_userdata);
    }
}



/*$ preserve start $*/
}
/*$ preserve end $*/
