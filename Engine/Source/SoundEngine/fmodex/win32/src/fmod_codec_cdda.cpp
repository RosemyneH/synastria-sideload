#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_CDDA

#include "fmod.h"
#include "fmod_codec_cdda.h"
#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_file_cdda.h"
#include "fmod_metadata.h"
#include "fmod_os_cdda.h"
#include "fmod_soundi.h"
#include "fmod_string.h"

#include <memory.h>
#include <stdio.h>


namespace FMOD
{


FMOD_CODEC_DESCRIPTION_EX cddacodec;


#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetCodecDescriptionEx is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_CODEC_DESCRIPTION_EX * F_API FMODGetCodecDescriptionEx()
    {
        return CodecCDDA::getDescriptionEx();
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
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 7f328b73 manual
// OFFSET: 0x00913420
FMOD_CODEC_DESCRIPTION_EX *CodecCDDA::getDescriptionEx()
{
    FMOD_memset(&cddacodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    cddacodec.name        = "FMOD CDDA Codec";
    cddacodec.version     = 0x00010100;
    cddacodec.timeunits   = FMOD_TIMEUNIT_PCM;
    cddacodec.open        = &CodecCDDA::openCallback;
    cddacodec.close       = &CodecCDDA::closeCallback;
    cddacodec.read        = &CodecCDDA::readCallback;
    cddacodec.setposition = &CodecCDDA::setPositionCallback;

    cddacodec.mType       = FMOD_SOUND_TYPE_CDDA;
    cddacodec.mSize       = sizeof(CodecCDDA);

    return &cddacodec;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 4d0a15b6 manual
// OFFSET: 0x00913260
FMOD_RESULT CodecCDDA::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    int          count;
    char        *filename;
    CddaFile    *cddafile;
    FMOD_RESULT  result = FMOD_OK;

    mCurrentTrack = -1;

    init(FMOD_SOUND_TYPE_CDDA);

    FLOG((FMOD_DEBUG_LEVEL_ALL, __FILE__, __LINE__, "CodecCDDA::openInternal", "attempting to open as CDDA..\n"));

    mFile->getName(&filename);
    if (!FMOD_OS_CDDA_IsDeviceName(filename))
    {
        return FMOD_ERR_FORMAT;
    }

    cddafile = SAFE_CAST(CddaFile, mFile);

    result = cddafile->getNumTracks(&numsubsounds);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (!numsubsounds)
    {
        return FMOD_ERR_CDDA_NOAUDIO;
    }

    waveformat = (FMOD_CODEC_WAVEFORMAT *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_WAVEFORMAT) * numsubsounds);
    if (!waveformat)
    {
        return FMOD_ERR_MEMORY;
    }

    for (count = 0; count < numsubsounds; count++)
    {
        unsigned int tracklength;

        result = cddafile->getTrackLength(count, &tracklength);
        if (result != FMOD_OK)
        {
            return result;
        }

        sprintf(waveformat[count].name, "Track %d", count+1);
        waveformat[count].format     = FMOD_SOUND_FORMAT_PCM16;
        waveformat[count].channels   = 2;
        waveformat[count].frequency  = 44100;
        waveformat[count].lengthpcm  = tracklength >> 2;
        waveformat[count].blockalign = waveformat[count].channels * 16 / 8;
    }

    mPCMBufferLengthBytes = 256 * 1024;

    result = setPositionInternal(0, 0, FMOD_TIMEUNIT_PCM);

    return result;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT CodecCDDA::closeInternal()
{
    if (waveformat)
    {
        FMOD_Memory_Free(waveformat);
        waveformat = 0;
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
    Win32, Win64

	[SEE_ALSO]
]
*/
FMOD_RESULT CodecCDDA::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    FMOD_RESULT result = FMOD_OK;

    result = mFile->read(buffer, 1, sizebytes, bytesread);
	if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
    {
        return result;
    }

    return result;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 2d46301c auto
// OFFSET: 0x00912FF0
FMOD_RESULT CodecCDDA::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CddaFile    *cddafile;
    FMOD_RESULT  result = FMOD_OK;
    unsigned int offset;

    if (subsound < 0 || (numsubsounds && subsound >= numsubsounds))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    cddafile = SAFE_CAST(CddaFile, mFile);

    if (subsound != mCurrentTrack)
    {
        mCurrentTrack = subsound;

        result = cddafile->openTrack(mCurrentTrack);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    result = SoundI::getBytesFromSamples(position, &offset, waveformat[mCurrentTrack].channels, waveformat[mCurrentTrack].format);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = cddafile->seek(offset, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    return result;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 3deb3ead manual
// OFFSET: 0x009133E0
FMOD_RESULT F_CALLBACK CodecCDDA::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecCDDA *cdda = (CodecCDDA *)codec;

    return cdda->openInternal(usermode, userexinfo);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 007875c1 manual
// OFFSET: 0x00912F70
FMOD_RESULT F_CALLBACK CodecCDDA::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecCDDA *cdda = (CodecCDDA *)codec;

    return cdda->closeInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 01206a2d manual
// OFFSET: 0x00912FC0
FMOD_RESULT F_CALLBACK CodecCDDA::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecCDDA *cdda = (CodecCDDA *)codec;

    return cdda->readInternal(buffer, sizebytes, bytesread);
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64

	[SEE_ALSO]
]
*/
// VERIFY: 7c6ca163 auto
// OFFSET: 0x00913230
FMOD_RESULT F_CALLBACK CodecCDDA::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecCDDA *cdda = (CodecCDDA *)codec;

    return cdda->setPositionInternal(subsound, position, postype);
}

}

#endif
