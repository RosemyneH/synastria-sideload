#ifndef _FMOD_CODEC_WAV_H
#define _FMOD_CODEC_WAV_H

#include "fmod_settings.h"

#if defined(FMOD_SUPPORT_WAV) || defined(FMOD_SUPPORT_IMAADPCM)

#include "fmod_codeci.h"
#include "fmod_memory.h"
#include "fmod_types.h"

#if defined(PLATFORM_WINDOWS)
	#include <windows.h>
	#include <mmreg.h>
	#include <msacm.h>   
#endif

namespace FMOD
{
    #ifndef WAVE_FORMAT_PCM
	    #define WAVE_FORMAT_PCM	        1
    #endif

    #ifndef WAVE_FORMAT_IEEE_FLOAT
        #define WAVE_FORMAT_IEEE_FLOAT  3
    #endif

    #ifndef WAVE_FORMAT_MPEG
        #define WAVE_FORMAT_MPEG        0x0050          /*  Microsoft Corporation  */
    #endif

    #ifndef WAVE_FORMAT_MPEGLAYER3
        #define WAVE_FORMAT_MPEGLAYER3  0x0055
    #endif

    #ifndef WAVE_FORMAT_OGGVORBIS
        #define WAVE_FORMAT_OGGVORBIS   0x6750
    #endif

    #ifndef WAVE_FORMAT_EXTENSIBLE
        #define WAVE_FORMAT_EXTENSIBLE  0xFFFE
    #endif 

    #ifdef FMOD_SUPPORT_PRAGMAPACK
	    #pragma pack(1)
    #endif

    #define F_WSMP_NO_TRUNCATION        0x0001
    #define F_WSMP_NO_COMPRESSION       0x0002
    #define WLOOP_TYPE_FORWARD          0

    typedef struct
    {
	    signed char id[4]   FMOD_PACKED_INTERNAL;
	    unsigned int size   FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  WAVE_CHUNK;

    typedef struct
    {
        unsigned short	wFormatTag      FMOD_PACKED_INTERNAL;   /* format type  */
        unsigned short	nChannels       FMOD_PACKED_INTERNAL;   /* number of channels (i.e. mono, stereo...)  */
        unsigned int	nSamplesPerSec  FMOD_PACKED_INTERNAL;   /* sample rate  */
        unsigned int	nAvgBytesPerSec FMOD_PACKED_INTERNAL;   /* for buffer estimation  */
        unsigned short	nBlockAlign     FMOD_PACKED_INTERNAL;   /* block size of data  */
        unsigned short	wBitsPerSample  FMOD_PACKED_INTERNAL;   /* number of bits per sample of mono data */
        unsigned short	cbSize          FMOD_PACKED_INTERNAL;   /* the count in bytes of the size of extra information (after cbSize) */
    } FMOD_PACKED  WAVE_FORMATEX;

    typedef struct
    {
        WAVE_FORMATEX   Format                  FMOD_PACKED_INTERNAL;
        union 
        {
            unsigned short wValidBitsPerSample  FMOD_PACKED_INTERNAL;   /* bits of precision  */
            unsigned short wSamplesPerBlock     FMOD_PACKED_INTERNAL;   /* valid if wBitsPerSample==0 */
            unsigned short wReserved            FMOD_PACKED_INTERNAL;   /* If neither applies, set to zero. */
        } FMOD_PACKED Samples;
        unsigned int    dwChannelMask           FMOD_PACKED_INTERNAL;   /* which channels are */
        FMOD_GUID       SubFormat               FMOD_PACKED_INTERNAL;
    } FMOD_PACKED WAVE_FORMATEXTENSIBLE;

    typedef struct
    {
	    unsigned int Manufacturer   FMOD_PACKED_INTERNAL;
	    unsigned int Product        FMOD_PACKED_INTERNAL;
	    unsigned int SamplePeriod   FMOD_PACKED_INTERNAL;
	    unsigned int Note           FMOD_PACKED_INTERNAL;
	    unsigned int FineTune       FMOD_PACKED_INTERNAL;
	    unsigned int SMPTEFormat    FMOD_PACKED_INTERNAL;
	    unsigned int SMPTEOffset    FMOD_PACKED_INTERNAL;
	    unsigned int Loops          FMOD_PACKED_INTERNAL;
	    unsigned int SamplerData    FMOD_PACKED_INTERNAL;
	    struct
	    {
		    unsigned int Identifier FMOD_PACKED_INTERNAL;
		    unsigned int Type       FMOD_PACKED_INTERNAL;
		    unsigned int Start      FMOD_PACKED_INTERNAL;
		    unsigned int End        FMOD_PACKED_INTERNAL;
		    unsigned int Fraction   FMOD_PACKED_INTERNAL;
		    unsigned int Count      FMOD_PACKED_INTERNAL;
	    } FMOD_PACKED Loop;
    } FMOD_PACKED WAVE_SMPLHEADER;

    typedef struct
    {
	    int		dwIdentifier        FMOD_PACKED_INTERNAL;
	    int		dwPosition          FMOD_PACKED_INTERNAL;
	    char	fccChunk[4]         FMOD_PACKED_INTERNAL;
	    int		dwChunkStart        FMOD_PACKED_INTERNAL;
	    int		dwBlockStart        FMOD_PACKED_INTERNAL;
	    int		dwSampleOffset      FMOD_PACKED_INTERNAL;
    } FMOD_PACKED WAVE_CUEPOINT;

    #ifdef FMOD_SUPPORT_PRAGMAPACK
        #ifdef 	CODEWARRIOR
        #pragma pack(0)
        #else
        #pragma pack()
        #endif
    #endif

    class SyncPoint;
    class ChannelSoftware;
	class ChannelOpenAL;

    class CodecWav  : public Codec
    {
        friend class CodecMPEG;
        friend class CodecMPEGPSP;
        friend class CodecOggVorbis;
        friend class CodecTremor;
        friend class CodecDLS;
        friend class CodecFSB;
        friend class ChannelSoftware;
		friend class ChannelOpenAL;
        friend class DSPCodec;
        friend class DSPCodecPool;
        friend class SystemI;

      private:

#if defined(PLATFORM_WINDOWS) && !defined(__MINGW32__)
        HACMSTREAM              mACMCodec;
#endif

    	WAVE_FORMATEXTENSIBLE   mDestFormat;

        int                     mNumSyncPoints;
        SyncPoint         *mSyncPoint;
        int                     mSamplesPerADPCMBlock;

        FMOD_RESULT parseChunk(unsigned int chunksize);
        FMOD_RESULT openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT closeInternal();
        FMOD_RESULT readInternal(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype);
        FMOD_RESULT soundCreateInternal(int subsound, FMOD_SOUND *sound);
        FMOD_RESULT canPointInternal();

      public:

    	WAVE_FORMATEXTENSIBLE   mSrcFormatMemory;
    	WAVE_FORMATEXTENSIBLE  *mSrcFormat;

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
        static FMOD_RESULT F_CALLBACK soundCreateCallback(FMOD_CODEC_STATE *codec, int subsound, FMOD_SOUND *sound);
        static FMOD_RESULT F_CALLBACK canPointCallback(FMOD_CODEC_STATE *codec);

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();
    };
}

#endif  /* FMOD_SUPPORT_WAV */

#endif


