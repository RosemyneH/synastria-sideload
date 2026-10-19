#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_VAG

#include "fmod.h"
#include "fmod_codec_swvag.h"
#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_soundi.h"
#include "fmod_string.h"

#include <stdio.h>

namespace FMOD
{


FMOD_CODEC_DESCRIPTION_EX vagcodec;

#if defined(PLUGIN_EXPORTS) && !defined(PLUGIN_FSB)

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetCodecDescriptionEx is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_CODEC_DESCRIPTION_EX * F_API FMODGetCodecDescriptionEx()
    {
        return CodecVAG::getDescriptionEx();
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
// VERIFY: 8869d883 manual
// OFFSET: 0x00910670
FMOD_CODEC_DESCRIPTION_EX *CodecVAG::getDescriptionEx()
{
    FMOD_memset(&vagcodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    vagcodec.name        = "FMOD VAG Codec";
    vagcodec.version     = 0x00010100;
    vagcodec.timeunits   = FMOD_TIMEUNIT_PCM;
    vagcodec.open        = &CodecVAG::openCallback;
    vagcodec.close       = &CodecVAG::closeCallback;
    vagcodec.read        = &CodecVAG::readCallback;
    vagcodec.setposition = &CodecVAG::setPositionCallback;

    vagcodec.mType       = FMOD_SOUND_TYPE_VAG;
    vagcodec.mSize       = sizeof(CodecVAG);

    return &vagcodec;
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
// VERIFY: 7126edb5 auto
// OFFSET: 0x00910270
FMOD_RESULT CodecVAG::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
   FMOD_RESULT     result = FMOD_OK;
    FMOD_VAG_HDR    hdr;

    init(FMOD_SOUND_TYPE_VAG);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecVAG::openInternal", "attempting to open as VAG..\n"));

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mFile->read(&hdr, 1, sizeof(FMOD_VAG_HDR));
    if (result != FMOD_OK)
    {
        return result;
    }

    if (FMOD_strncmp((char *)hdr.format, "VAG", 3))
    {
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__,  "CodecVAG::openInternal", "'VAG' ID check failed [%c%c%c]\n", hdr.format[0], hdr.format[1], hdr.format[2]));

        return FMOD_ERR_FORMAT;
    }

#ifdef PLATFORM_ENDIAN_LITTLE
    hdr.fs   = FMOD_SWAPENDIAN_DWORD(hdr.fs);
    hdr.size = FMOD_SWAPENDIAN_DWORD(hdr.size);
#endif

    mSrcDataOffset  = sizeof(FMOD_VAG_HDR);

    waveformat = &mWaveFormat;

    waveformat[0].format     = FMOD_SOUND_FORMAT_PCM16;
	waveformat[0].channels   = 1;
    waveformat[0].frequency  = hdr.fs;
	waveformat[0].lengthbytes= hdr.size;
	waveformat[0].lengthpcm  = waveformat[0].lengthbytes * 28 / 16;

    mPCMBufferLength = 28;
    mPCMBufferLengthBytes = 56;

    #ifdef PLATFORM_PS3
    mPCMBuffer = (unsigned char *)FMOD_ALIGNPOINTER(mPCMBlock, 16);
    #else
    mPCMBuffer = (unsigned char *)mPCMBlock;
    #endif

    numsubsounds = 0;

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecVAG::openInternal", "successfully opened vag file..\n"));

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
FMOD_RESULT CodecVAG::closeInternal()
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
// VERIFY: e839c73e manual
// OFFSET: 0x009103D0
FMOD_RESULT CodecVAG::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    FMOD_RESULT result;
    char src[16];
    unsigned char *dest = (unsigned char *)buffer;

    static float f[5][2] =
    {
        {    0.0f,         0.0f           },
		{	60.0f / 64.0f, 0.0f           },
		{  115.0f / 64.0f, -52.0f / 64.0f },
		{	98.0f / 64.0f, -55.0f / 64.0f },
		{  122.0f / 64.0f, -60.0f / 64.0f }
    };

    while (sizebytes)
    {
	    unsigned char *p;
	    int predict_nr, shift_factor, flags;
	    int i;
	    int d, s;
        float samples[28];

        result = mFile->read(src, 16, 1, 0);
        if (result != FMOD_OK)
        {
            return result;
        }

	    p=(unsigned char *)src;

		predict_nr = *p++;
		shift_factor = predict_nr & 0xf;
		predict_nr >>= 4;

		flags = *p++; 

#if 0
        /*
            Doesn't make sense to break out because certain things are set.
        */
		if ( flags == (FMOD_VAG_LOOP | FMOD_VAG_START | FMOD_VAG_END) )
        {
			break;	
        }
		if (flags == (FMOD_VAG_LOOP | FMOD_VAG_END))
        {
			break;
        }
		if (flags == FMOD_VAG_START)
        {
			break;
        }
		if (flags == (FMOD_VAG_START | FMOD_VAG_LOOP))
        {
			break;
        }
#endif


		for ( i = 0; i < 28; i += 2 ) 
        {
            d = *p++;
            s = ( d & 0xf ) << 12;
            if ( s & 0x8000 )
            {
                s |= 0xffff0000;
            }
			samples[i] = (float) ( s >> shift_factor);
			s = ( d & 0xf0 ) << 8;
			if ( s & 0x8000 )
            {
				 s |= 0xffff0000;
            }
			samples[i+1] = (float) ( s >> shift_factor);
		}

		for ( i = 0; i < 28; i++ )
        {
            samples[i] += mContext.mS1 * f[predict_nr][0] + mContext.mS2 * f[predict_nr][1];
            mContext.mS2 = mContext.mS1;
            mContext.mS1 = samples[i];
            d = (int) ( samples[i] + 0.5f );
            *dest++ = (d & 0xff);
            *dest++ = (d >> 8);
		}

        *bytesread += 56;
        sizebytes -= 56;
    }

    #ifdef PLATFORM_ENDIAN_BIG

    signed short *wptr = (signed short *)buffer;

    int count = *bytesread / 2;

    for (int i = 0; i < count; i++)
    {
        wptr[i] = FMOD_SWAPENDIAN_WORD(wptr[i]);
    }

    #endif

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
// VERIFY: f424901b auto
// OFFSET: 0x009105A0
FMOD_RESULT CodecVAG::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result;
    int bits;
    unsigned int bytes;

    result = SoundI::getBitsFromFormat(waveformat[0].format, &bits);
    if (result != FMOD_OK)
    {
        return result;
    }

    SoundI::getBytesFromSamples(position, &bytes, 1, FMOD_SOUND_FORMAT_VAG);

    result = mFile->seek(mSrcDataOffset + bytes, SEEK_SET);

    mContext.mS1 = mContext.mS2 = 0;

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
// VERIFY: fb3b813b manual
// OFFSET: 0x00910530
FMOD_RESULT F_CALLBACK CodecVAG::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecVAG *vag = (CodecVAG *)codec;

    return vag->openInternal(usermode, userexinfo);
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
FMOD_RESULT F_CALLBACK CodecVAG::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecVAG *vag = (CodecVAG *)codec;

    return vag->closeInternal();
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
// VERIFY: bbb9c8f9 auto
// OFFSET: 0x00910570
FMOD_RESULT F_CALLBACK CodecVAG::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecVAG *vag = (CodecVAG *)codec;

    return vag->readInternal(buffer, sizebytes, bytesread);
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
// VERIFY: d415439e auto
// OFFSET: 0x00910640
FMOD_RESULT F_CALLBACK CodecVAG::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecVAG *vag = (CodecVAG *)codec;

    return vag->setPositionInternal(subsound, position, postype);
}

}

#endif


