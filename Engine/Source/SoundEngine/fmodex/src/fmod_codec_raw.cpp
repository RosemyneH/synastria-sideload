#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_RAW

#include "fmod.h"
#include "fmod_codec_raw.h"
#include "fmod_codec_wav.h"
#include "fmod_codec_wav_imaadpcm.h"
#include "fmod_dsp_codec.h"
#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_soundi.h"
#include "fmod_string.h"

#ifdef PLATFORM_PS3_SPU
#include "fmod_systemi_spu.h"
#else
#include "fmod_systemi.h"
#endif

#include <stdio.h>

#ifdef PLATFORM_PS3
extern unsigned int _binary_spu_fmod_codec_raw_pic_start[];
#endif


namespace FMOD
{


FMOD_CODEC_DESCRIPTION_EX rawcodec;

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
        return CodecRaw::getDescriptionEx();
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
// VERIFY: 0027815b manual
// OFFSET: 0x008F5760
FMOD_CODEC_DESCRIPTION_EX *CodecRaw::getDescriptionEx()
{
#ifndef PLATFORM_PS3_SPU
    FMOD_memset(&rawcodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    rawcodec.name        = "FMOD Raw Codec";
    rawcodec.version     = 0x00010100;
    rawcodec.timeunits   = FMOD_TIMEUNIT_PCM | FMOD_TIMEUNIT_RAWBYTES;
    rawcodec.open        = &CodecRaw::openCallback;
    rawcodec.close       = &CodecRaw::closeCallback;
    rawcodec.read        = &CodecRaw::readCallback;
    rawcodec.setposition = &CodecRaw::setPositionCallback;
    rawcodec.canpoint    = &CodecRaw::canPointCallback;

    rawcodec.mType       = FMOD_SOUND_TYPE_RAW;
    rawcodec.mSize       = sizeof(CodecRaw);

    #ifdef PLATFORM_PS3
    rawcodec.mModule     = (FMOD_OS_LIBRARY *)_binary_spu_fmod_codec_raw_pic_start;
    #endif
#else
    rawcodec.read        = &CodecRaw::readCallback;
    rawcodec.setposition = &CodecRaw::setPositionCallback;
#endif

    return &rawcodec;
}

#ifndef PLATFORM_PS3_SPU

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
// VERIFY: 0872f47a manual
// OFFSET: 0x008F5440
FMOD_RESULT CodecRaw::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    FMOD_RESULT     result = FMOD_OK;

    init(FMOD_SOUND_TYPE_RAW);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecRaw::openInternal", "attempting to open as RAW..\n"));

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    waveformat = &mWaveFormat;

	/*
        Get size of file in bytes
    */
	result = mFile->getSize(&waveformat[0].lengthbytes);
    if (result != FMOD_OK)
    {
        return result;
    }

    mSrcDataOffset  = 0;

    if (usermode & FMOD_SOFTWARE)
    {
        if (usermode & FMOD_CREATECOMPRESSEDSAMPLE)
        {
            if (1
#if defined(FMOD_SUPPORT_IMAADPCM)
                && userexinfo->format != FMOD_SOUND_FORMAT_IMAADPCM 
#endif                
            )
            {
                return FMOD_ERR_FORMAT;
            }
        }
        else if (userexinfo->format < FMOD_SOUND_FORMAT_PCM8 || userexinfo->format > FMOD_SOUND_FORMAT_PCMFLOAT)
        {
            return FMOD_ERR_FORMAT;
        }
    }

    waveformat[0].format     = userexinfo->format;
	waveformat[0].channels   = userexinfo->numchannels;
    waveformat[0].frequency  = userexinfo->defaultfrequency;
	SoundI::getSamplesFromBytes(waveformat[0].lengthbytes, &waveformat[0].lengthpcm, userexinfo->numchannels, userexinfo->format);
    waveformat[0].blockalign = waveformat[0].channels * 16 / 8;

    /*
        Fill out base class members, also pointing to or allocating storage for them.
    */
    numsubsounds = 0;

#if defined(FMOD_SUPPORT_IMAADPCM) && defined(FMOD_SUPPORT_DSPCODEC)
    if (waveformat[0].format == FMOD_SOUND_FORMAT_IMAADPCM)
    {
        int count;

        if (waveformat[0].channels > 2)                           
        {
            return FMOD_ERR_TOOMANYCHANNELS;    /* Sorry we're only allocating memory for a pool of maximum stereo voices. */
        }
        
        mSamplesPerADPCMBlock = 64;
        mReadBufferLength     = 36 * waveformat[0].channels;

        if (!mSystem->mDSPCodecPool_ADPCM.mNumDSPCodecs)
        {
            result = mSystem->mDSPCodecPool_ADPCM.init(FMOD_DSP_CATEGORY_DSPCODECADPCM, 64, mSystem->mAdvancedSettings.maxADPCMcodecs ? mSystem->mAdvancedSettings.maxADPCMcodecs : FMOD_ADVANCEDSETTINGS_MAXADPCMCODECS);
            if (result != FMOD_OK)
            {
                return result;
            }

            for (count = 0; count < mSystem->mDSPCodecPool_ADPCM.mNumDSPCodecs; count++)
            {
                DSPCodec *dspcodec = SAFE_CAST(DSPCodec, mSystem->mDSPCodecPool_ADPCM.mPool[count]);
                CodecWav *wav      = (CodecWav *)dspcodec->mCodec;

                wav->mSrcFormat  = &wav->mSrcFormatMemory;
                wav->mReadBuffer = mSystem->mDSPCodecPool_ADPCM.mReadBuffer;
                wav->mSrcFormat->Format.wFormatTag = WAVE_FORMAT_IMA_ADPCM;
            }
        }
    }
#endif
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
FMOD_RESULT CodecRaw::closeInternal()
{       
    return FMOD_OK;
}

#endif

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
FMOD_RESULT CodecRaw::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{   
    FMOD_RESULT result;

    if (waveformat[0].format == FMOD_SOUND_FORMAT_PCM16)
    {
        result = mFile->read(buffer, 2, sizebytes / 2, bytesread);

        *bytesread *= 2;    /* convert from 16bit words back to bytes */
    }
    else
    {
        result = mFile->read(buffer, 1, sizebytes, bytesread);
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 24ab3c76 auto
// OFFSET: 0x008F5340
FMOD_RESULT CodecRaw::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result;
    unsigned int raw;

    if (postype == FMOD_TIMEUNIT_RAWBYTES)
    {
        raw = position;
    }
    else
    {
        raw = (unsigned int)((FMOD_UINT64)position * (FMOD_UINT64)waveformat[0].lengthbytes / (FMOD_UINT64)waveformat[0].lengthpcm);
        raw /= waveformat[0].blockalign;
        raw *= waveformat[0].blockalign;
    }

    result = mFile->seek(mSrcDataOffset + raw, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    return FMOD_OK;
}


#ifndef PLATFORM_PS3_SPU
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
FMOD_RESULT CodecRaw::canPointInternal() 
{
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
// VERIFY: 74abc6f5 manual
// OFFSET: 0x008F5720
FMOD_RESULT F_CALLBACK CodecRaw::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecRaw *raw = (CodecRaw *)codec;

    return raw->openInternal(usermode, userexinfo);
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
FMOD_RESULT F_CALLBACK CodecRaw::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecRaw *raw = (CodecRaw *)codec;

    return raw->closeInternal();
}

#endif

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
// VERIFY: 5158d528 auto
// OFFSET: 0x008F53A0
FMOD_RESULT F_CALLBACK CodecRaw::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecRaw *raw = (CodecRaw *)codec;

    return raw->readInternal(buffer, sizebytes, bytesread);
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
// VERIFY: 5e65e21c auto
// OFFSET: 0x008F5410
FMOD_RESULT F_CALLBACK CodecRaw::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecRaw *raw = (CodecRaw *)codec;

    return raw->setPositionInternal(subsound, position, postype);
}


#ifndef PLATFORM_PS3_SPU
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
FMOD_RESULT F_CALLBACK CodecRaw::canPointCallback(FMOD_CODEC_STATE *codec)
{
    CodecRaw *raw = (CodecRaw *)codec;

    return raw->canPointInternal();
}
#endif

}

#endif

