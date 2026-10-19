#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_NOSOUND

#include "fmod_memory.h"
#include "fmod_output_nosound.h"
#include "fmod_soundi.h"
#include "fmod_string.h"
#include "fmod_systemi.h"
#include "fmod_time.h"

namespace FMOD
{

FMOD_OUTPUT_DESCRIPTION_EX nosoundoutput;

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
        return OutputNoSound::getDescriptionEx();
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
// VERIFY: 785cae02 auto
// OFFSET: 0x00915030
FMOD_OUTPUT_DESCRIPTION_EX *OutputNoSound::getDescriptionEx()
{
    FMOD_memset(&nosoundoutput, 0, sizeof(FMOD_OUTPUT_DESCRIPTION_EX));

    nosoundoutput.name          = "FMOD NoSound Output";
    nosoundoutput.version       = 0x00010100;
    nosoundoutput.polling       = true;
    nosoundoutput.getnumdrivers = &OutputNoSound::getNumDriversCallback;
    nosoundoutput.getdrivername = &OutputNoSound::getDriverNameCallback;
    nosoundoutput.getdrivercaps = &OutputNoSound::getDriverCapsCallback;
    nosoundoutput.init          = &OutputNoSound::initCallback;
    nosoundoutput.close         = &OutputNoSound::closeCallback;
    nosoundoutput.getposition   = &OutputNoSound::getPositionCallback;
    nosoundoutput.lock          = &OutputNoSound::lockCallback;

    /*
        Private members
    */
    nosoundoutput.mType         = FMOD_OUTPUTTYPE_NOSOUND;
    nosoundoutput.mSize         = sizeof(OutputNoSound);

    return &nosoundoutput;
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
FMOD_RESULT OutputNoSound::getNumDrivers(int *numdrivers)
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
FMOD_RESULT OutputNoSound::getDriverName(int driver, char *name, int namelen)
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
FMOD_RESULT OutputNoSound::getDriverCaps(int id, FMOD_CAPS *caps)
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
// VERIFY: 09a42327 auto
// OFFSET: 0x00914E40
FMOD_RESULT OutputNoSound::init(int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata)
{
    Plugin::init(); /* Set up gGlobal - for debug / file / memory access by this plugin. */

	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputNoSound::init", "Initializing.\n"));

    SoundI::getBytesFromSamples(dspbufferlength * dspnumbuffers, &mBlockLengthBytes, outputchannels, *outputformat);
    
    mBuffer = (char *)FMOD_Memory_Calloc(mBlockLengthBytes);
    if (!mBuffer)
    {
        return FMOD_ERR_MEMORY;
    }


    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "OutputNoSound::init", "Done.\n"));

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
FMOD_RESULT OutputNoSound::close()
{
    if (mBuffer)
    {
        FMOD_Memory_Free(mBuffer);
    }
	mBuffer = NULL;

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
FMOD_RESULT OutputNoSound::getPosition(unsigned int *pcm)
{
    FMOD_RESULT     result;
    unsigned int    pos = 0;
    int             outputrate;

    /*
        Get the values FMOD had set for rate, format, channels.
    */
    result = mSystem->getSoftwareFormat(&outputrate, 0, 0, 0, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    FMOD_OS_Time_GetMs(&pos);
	
    pos *= outputrate;
    pos /= 1000;

	*pcm = pos;

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
// VERIFY: d368a7e6 manual
// OFFSET: 0x00914CE0
FMOD_RESULT OutputNoSound::lock(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
	offset %= mBlockLengthBytes;

	if (offset + length > mBlockLengthBytes)
	{
		*ptr1 = (char *)mBuffer + offset;
		*ptr2 = (char *)mBuffer;
		*len1 = mBlockLengthBytes - offset;
		*len2 = length - (mBlockLengthBytes - offset);
	}
	else
	{
		*ptr1 = (char *)mBuffer + offset;
		*ptr2 = NULL;
		*len1 = length;
		*len2 = 0;
	}

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
// VERIFY: a61c5372 auto
// OFFSET: 0x009152B0
FMOD_RESULT F_CALLBACK OutputNoSound::getNumDriversCallback(FMOD_OUTPUT_STATE *output, int *numdrivers)
{
    OutputNoSound *nosound = (OutputNoSound *)output;

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
// VERIFY: c4ce9933 auto
// OFFSET: 0x00914D70
FMOD_RESULT F_CALLBACK OutputNoSound::getDriverNameCallback(FMOD_OUTPUT_STATE *output, int id, char *name, int namelen)
{
    OutputNoSound *nosound = (OutputNoSound *)output;

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
// VERIFY: 14adcfbc auto
// OFFSET: 0x00914D90
FMOD_RESULT OutputNoSound::getDriverCapsCallback(FMOD_OUTPUT_STATE *output, int id, FMOD_CAPS *caps)
{
    OutputNoSound *nosound = (OutputNoSound *)output;

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
// VERIFY: 1aa60fb7 auto
// OFFSET: 0x00914FF0
FMOD_RESULT F_CALLBACK OutputNoSound::initCallback(FMOD_OUTPUT_STATE *output, int selecteddriver, FMOD_INITFLAGS flags, int *outputrate, int outputchannels, FMOD_SOUND_FORMAT *outputformat, int dspbufferlength, int dspnumbuffers, void *extradriverdata)
{
    OutputNoSound *nosound = (OutputNoSound *)output;

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
// VERIFY: c3f2d7c0 manual
// OFFSET: 0x00914DB0
FMOD_RESULT F_CALLBACK OutputNoSound::closeCallback(FMOD_OUTPUT_STATE *output)
{
    OutputNoSound *nosound = (OutputNoSound *)output;

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
// VERIFY: b2fe518e manual
// OFFSET: 0x00915AA0
FMOD_RESULT F_CALLBACK OutputNoSound::getPositionCallback(FMOD_OUTPUT_STATE *output, unsigned int *pcm)
{
    OutputNoSound *nosound = (OutputNoSound *)output;

    return nosound->getPosition(pcm);
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
// VERIFY: d1eb1385 auto
// OFFSET: 0x00914E00
FMOD_RESULT F_CALLBACK OutputNoSound::lockCallback(FMOD_OUTPUT_STATE *output, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    OutputNoSound *nosound = (OutputNoSound *)output;

    return nosound->lock(offset, length, ptr1, ptr2, len1, len2);
}


}

#endif
