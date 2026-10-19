#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_USERCODEC

#include "fmod.h"
#include "fmod_codec_user.h"
#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_soundi.h"
#include "fmod_string.h"

#include <stdio.h>

namespace FMOD
{


FMOD_CODEC_DESCRIPTION_EX usercodec;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetCodecDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_CODEC_DESCRIPTION_EX * F_API FMODGetCodecDescriptionEx()
    {
        return CodecUser::getDescriptionEx();
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
// VERIFY: ef479b12 manual
// OFFSET: 0x008E8F30
FMOD_CODEC_DESCRIPTION_EX *CodecUser::getDescriptionEx()
{
    FMOD_memset(&usercodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    usercodec.name        = "FMOD User Reader Codec";
    usercodec.version     = 0x00010100;
    usercodec.timeunits   = FMOD_TIMEUNIT_PCM;
    usercodec.open        = &CodecUser::openCallback;
    usercodec.close       = &CodecUser::closeCallback;
    usercodec.read        = &CodecUser::readCallback;
    usercodec.setposition = &CodecUser::setPositionCallback;

    usercodec.mType       = FMOD_SOUND_TYPE_USER;
    usercodec.mSize       = sizeof(CodecUser);

    return &usercodec;
}


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
// VERIFY: b39686ad manual
// OFFSET: 0x008E8CF0
FMOD_RESULT CodecUser::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    FMOD_RESULT     result = FMOD_OK;

    init(FMOD_SOUND_TYPE_USER);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecUser::openInternal", "attempting to open user codec..\n"));

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    waveformat = &mWaveFormat;

#if defined(PLATFORM_PS2) || defined(PLATFORM_PSP)
    if (userexinfo->format == FMOD_SOUND_FORMAT_VAG)
    {
        /* Do nothing */
    }
    else
#endif
    if (userexinfo->format < FMOD_SOUND_FORMAT_PCM8 || userexinfo->format > FMOD_SOUND_FORMAT_PCMFLOAT)
    {
        return FMOD_ERR_FORMAT;
    }

	/*
        Get size of file in bytes
    */
	result = mFile->getSize(&waveformat[0].lengthbytes);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        TODO : Open and fill in needed codec values
    */
	mSrcDataOffset  = 0;

    waveformat[0].format     = userexinfo->format;
	waveformat[0].channels   = userexinfo->numchannels;
    waveformat[0].frequency  = userexinfo->defaultfrequency;
	SoundI::getSamplesFromBytes(userexinfo->length, &waveformat[0].lengthpcm, userexinfo->numchannels, userexinfo->format);
    waveformat[0].blockalign = waveformat[0].channels * 16 / 8;

    /*
        Fill out base class members, also pointing to or allocating storage for them.
    */
    numsubsounds = 0;

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecUser::openInternal", "Done.  format = %d, channels %d, frequency %d, lengthpcm %d, blockalign %d.\n", waveformat[0].format, waveformat[0].channels, waveformat[0].frequency, waveformat[0].lengthpcm, waveformat[0].blockalign));

    return result;
}


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
FMOD_RESULT CodecUser::closeInternal()
{       
    /*
        TODO : Close codec / cleanup
    */

    return FMOD_OK;
}


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
FMOD_RESULT CodecUser::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    FMOD_RESULT result = FMOD_OK;
   
    *bytesread = sizebytes;

    return result;	
}


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
FMOD_RESULT CodecUser::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result = FMOD_OK;

    return result;
}


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
// VERIFY: 6a2cac4b manual
// OFFSET: 0x008E8EF0
FMOD_RESULT F_CALLBACK CodecUser::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecUser *cuser = (CodecUser *)codec;

    return cuser->openInternal(usermode, userexinfo);
}


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
FMOD_RESULT F_CALLBACK CodecUser::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecUser *cuser = (CodecUser *)codec;

    return cuser->closeInternal();
}


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
// VERIFY: 63b04397 auto
// OFFSET: 0x008E8CD0
FMOD_RESULT F_CALLBACK CodecUser::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecUser *cuser = (CodecUser *)codec;

    return cuser->readInternal(buffer, sizebytes, bytesread);
}

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
FMOD_RESULT F_CALLBACK CodecUser::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecUser *cuser = (CodecUser *)codec;

    return cuser->setPositionInternal(subsound, position, postype);
}

}

#endif

