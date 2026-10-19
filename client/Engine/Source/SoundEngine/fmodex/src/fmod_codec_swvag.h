#ifndef _FMOD_CODEC_VAG_H
#define _FMOD_CODEC_VAG_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_VAG

#include "fmod_codeci.h"
#include "fmod_types.h"

namespace FMOD
{

    #define FMOD_VAG_START 0x04
    #define FMOD_VAG_LOOP  0x02
    #define FMOD_VAG_END   0x01

    #ifdef FMOD_SUPPORT_PRAGMAPACK
        #pragma pack(1)
    #endif

    typedef struct
    {
	    unsigned char	format[4] FMOD_PACKED_INTERNAL;	/* always 'VAGp' for identifying*/
	    unsigned int	ver       FMOD_PACKED_INTERNAL;	/* format version (2) */
	    unsigned int	ssa       FMOD_PACKED_INTERNAL;	/* Source Start Address, always 0 (reserved for VAB format) */
	    unsigned int	size      FMOD_PACKED_INTERNAL;	/* Sound Data Size in byte */

	    unsigned int	fs        FMOD_PACKED_INTERNAL;	/* sampling frequency, 44100(>pt1000), 32000(>pt), 22000(>pt0800)... */
	    unsigned short	volL      FMOD_PACKED_INTERNAL;	/* base volume for Left channel */
	    unsigned short	volR      FMOD_PACKED_INTERNAL;	/* base volume for Right channel */
	    unsigned short	pitch     FMOD_PACKED_INTERNAL;	/* base pitch (includes fs modulation)*/
	    unsigned short	ADSR1     FMOD_PACKED_INTERNAL;	/* base ADSR1 (see SPU manual) */
	    unsigned short	ADSR2     FMOD_PACKED_INTERNAL;	/* base ADSR2 (see SPU manual) */
	    unsigned short	reserved  FMOD_PACKED_INTERNAL;	/* not in use */

	    char			name[16]  FMOD_PACKED_INTERNAL;

    } FMOD_PACKED FMOD_VAG_HDR;

    #ifdef FMOD_SUPPORT_PRAGMAPACK
        #ifdef 	CODEWARRIOR
        #pragma pack(0)
        #else
        #pragma pack()
        #endif
    #endif

    struct CodecVAG_Context
    {
        float mS1, mS2;
    };

    class CodecVAG : public Codec
    {
        friend class CodecFSB;

      private:
        
        FMOD_CODEC_WAVEFORMAT   mWaveFormat;
        unsigned char           mUnknown[56];
        CodecVAG_Context        mContext;

        #ifdef PLATFORM_PS3
        unsigned char           mPCMBlock[56 + 16]; /* +16 for alignment */
        #else
        unsigned char           mPCMBlock[56];
        #endif

        FMOD_RESULT openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT closeInternal();
        FMOD_RESULT readInternal(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype);

      public:

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();       
    };
}

#endif  /* FMOD_SUPPORT_VAG */

#endif


