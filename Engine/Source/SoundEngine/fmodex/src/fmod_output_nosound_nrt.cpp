#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_NOSOUND

#include "fmod_memory.h"
#include "fmod_output_nosound_nrt.h"
#include "fmod_soundi.h"
#include "fmod_string.h"
#include "fmod_systemi.h"
#include "fmod_time.h"

namespace FMOD
{

FMOD_OUTPUT_DESCRIPTION_EX nosoundoutput_nrt;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetOutputDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_OUTPUT_DESCRIPTION_EX * F_API FMODGetOutputDescriptionEx()
    {
        return OutputNoSound_NRT::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 666faf10 auto
// OFFSET: 0x00914C50
FMOD_OUTPUT_DESCRIPTION_EX *OutputNoSound_NRT::getDescriptionEx()
{
    FMOD_memset(&nosoundoutput_nrt, 0, sizeof(FMOD_OUTPUT_DESCRIPTION_EX));

    nosoundoutput_nrt.name          = "FMOD NoSound Output - Non real-time";
    nosoundoutput_nrt.version       = 0x00010100;
    nosoundoutput_nrt.polling       = false;
    nosoundoutput_nrt.getnumdrivers = &OutputNoSound_NRT::getNumDriversCallback;
    nosoundoutput_nrt.getdrivername = &OutputNoSound_NRT::getDriverNameCallback;
    nosoundoutput_nrt.getdrivercaps = &OutputNoSound_NRT::getDriverCapsCallback;
    nosoundoutput_nrt.init          = &OutputNoSound_NRT::initCallback;
    nosoundoutput_nrt.close         = &OutputNoSound_NRT::closeCallback;
    nosoundoutput_nrt.update        = &OutputNoSound_NRT::updateCallback;

    /*
        Private members
    */
    nosoundoutput_nrt.mType         = FMOD_OUTPUTTYPE_NOSOUND_NRT;
    nosoundoutput_nrt.mSize         = sizeof(OutputNoSound_NRT);

    return &nosoundoutput_nrt;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputNoSound_NRT::getNumDrivers(int *numdrivers)
{
    *numdrivers = 1;

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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputNoSound_NRT::getDriverName(int driver, char *name, int namelen)
{
	FMOD_strncpy(name, "NoSound Driver", namelen);

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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputNoSound_NRT::getDriverCaps(int id, FMOD_CAPS *caps)
{
    *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_MULTICHANNEL);
    *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_FORMAT_PCM8);
    *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_FORMAT_PCM16);
    *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_FORMAT_PCM24);
    *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_FORMAT_PCM32);
    *caps = (FMOD_CAPS)(*caps | FMOD_CAPS_OUTPUT_FORMAT_PCMFLOAT);

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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: f7821295 auto
// OFFSET: 0x00914A90
FMOD_RESULT OutputNoSound_NRT::init(int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata)
{
    FMOD_RESULT         result;
    FMOD_SOUND_FORMAT   format;
    unsigned int        bufferlengthbytes = 0;
    int                 channels;

    Plugin::init(); /* Set up gGlobal - for debug / file / memory access by this plugin. */

	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputNoSound_NRT::init", "Initializing.\n"));

    result = mSystem->getSoftwareFormat(0, &format, &channels, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }
    
    mBufferLength = dspbufferlength;

    result = SoundI::getBytesFromSamples(mBufferLength, &bufferlengthbytes, channels, format);
    if (result != FMOD_OK)
    {
        return result;
    }

    mBuffer = FMOD_Memory_Calloc(bufferlengthbytes);
    if (!mBuffer)
    {
        return FMOD_ERR_MEMORY;
    }

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputNoSound_NRT::init", "Done.\n"));

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
FMOD_RESULT OutputNoSound_NRT::close()
{
    if (mBuffer)
    {
        FMOD_Memory_Free(mBuffer);
        mBuffer = 0;
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
    
	[SEE_ALSO]
]
*/
FMOD_RESULT OutputNoSound_NRT::update()
{
#ifdef FMOD_SUPPORT_SOFTWARE
    FMOD_RESULT   result;

    result = mix(mBuffer, mBufferLength);
    if (result != FMOD_OK)
    {
        return FMOD_OK;
    }
#endif

    return FMOD_OK;
}


/*
    ==============================================================================================================

    CALLBACK INTERFACE

    ==============================================================================================================
*/


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK OutputNoSound_NRT::getNumDriversCallback(FMOD_OUTPUT_STATE *output, int *numdrivers)
{
    OutputNoSound_NRT *nosound = (OutputNoSound_NRT *)output;

    return nosound->getNumDrivers(numdrivers);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK OutputNoSound_NRT::getDriverNameCallback(FMOD_OUTPUT_STATE *output, int id, char *name, int namelen)
{
    OutputNoSound_NRT *nosound = (OutputNoSound_NRT *)output;

    return nosound->getDriverName(id, name, namelen);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT OutputNoSound_NRT::getDriverCapsCallback(FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps)
{
    OutputNoSound_NRT *nosound = (OutputNoSound_NRT *)output;

    return nosound->getDriverCaps(id, caps);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: f295a13f auto
// OFFSET: 0x00914C10
FMOD_RESULT F_CALLBACK OutputNoSound_NRT::initCallback(FMOD_OUTPUT_STATE *output, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata)
{
    OutputNoSound_NRT *nosound = (OutputNoSound_NRT *)output;

    return nosound->init(selecteddriver, flags, outputrate, outputchannels, outputformat, dspbufferlength, dspnumbuffers, extradriverdata);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: d8031cf4 manual
// OFFSET: 0x00914A10
FMOD_RESULT F_CALLBACK OutputNoSound_NRT::closeCallback(FMOD_OUTPUT_STATE *output)
{
    OutputNoSound_NRT *nosound = (OutputNoSound_NRT *)output;

    return nosound->close();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: c88f0c92 auto
// OFFSET: 0x00914A60
FMOD_RESULT F_CALLBACK OutputNoSound_NRT::updateCallback(FMOD_OUTPUT_STATE *output)
{
    OutputNoSound_NRT *nosound = (OutputNoSound_NRT *)output;

    return nosound->update();
}



}

#endif
