#ifndef _FORMAT_IMAADPCM_H
#define _FORMAT_IMAADPCM_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_IMAADPCM

#include "fmod_codec_wav.h"

namespace FMOD
{
    #ifndef WAVE_FORMAT_IMA_ADPCM
        #define WAVE_FORMAT_IMA_ADPCM   0x0011
    #endif
    #ifndef WAVE_FORMAT_XBOX_ADPCM
        #define WAVE_FORMAT_XBOX_ADPCM  0x0069
    #endif

    #ifdef FMOD_SUPPORT_PRAGMAPACK
        #pragma pack(1)
    #endif

    typedef struct
    {
        WAVE_FORMATEX   wfx                 FMOD_PACKED_INTERNAL;
        unsigned short  wSamplesPerBlock    FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  WAVE_FORMAT_IMAADPCM;

    #ifdef FMOD_SUPPORT_PRAGMAPACK
        #ifdef 	CODEWARRIOR
        #pragma pack(0)
        #else
        #pragma pack()
        #endif
    #endif


    FMOD_RESULT IMAAdpcm_DecodeM16(unsigned char *pbSrc, signed short *pbDst, unsigned int cBlocks, unsigned int nBlockAlignment, unsigned int cSamplesPerBlock, int channels);
    FMOD_RESULT IMAAdpcm_DecodeS16(unsigned char *pbSrc, signed short *pbDst, unsigned int cBlocks, unsigned int nBlockAlignment, unsigned int cSamplesPerBlock);
}


#endif  /* FMOD_SUPPORT_IMAADPCM */

#endif  /* _FORMAT_IMAADPCM_H */


