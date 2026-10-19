#ifndef _FMOD_CODEC_FSB_H
#define _FMOD_CODEC_FSB_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_FSB

#include "fmod_codeci.h"
#include "fmod_syncpoint.h"

#define FMOD_FSB_NAMELEN             30

#define FMOD_FSB_SOURCE_FORMAT         0x00000001  /* all samples stored in their original compressed format */
#define FMOD_FSB_SOURCE_BASICHEADERS   0x00000002  /* samples should use the basic header structure */
#define FMOD_FSB_SOURCE_ENCRYPTED      0x00000004  /* all sample data is encrypted */
#define FMOD_FSB_SOURCE_BIGENDIANPCM   0x00000008  /* pcm samples have been written out in big-endian format */
#define FMOD_FSB_SOURCE_NOTINTERLEAVED 0x00000010  /* Sample data is not interleaved. */
#define FMOD_FSB_SOURCE_MPEG_PADDED    0x00000020  /* Mpeg frames are now rounded up to the nearest 2 bytes for normal sounds, or 16 bytes for multichannel. */
#define FMOD_FSB_SOURCE_MPEG_PADDED4   0x00000040  /* Mpeg frames are now rounded up to the nearest 4 bytes for normal sounds, or 16 bytes for multichannel. */

//#define FMOD_FSB_FORCE_3_0

#define FMOD_FSB_VERSION_3_0         0x00030000  /* FSB version 3.0 */
#define FMOD_FSB_VERSION_3_1         0x00030001  /* FSB version 3.1 */
#define FMOD_FSB_VERSION_4_0         0x00040000  /* FSB version 4.0 */

/*
    FMOD 3 defines.
*/
#define FSOUND_LOOP_OFF              0x00000001  /* For non looping samples. */
#define FSOUND_LOOP_NORMAL           0x00000002  /* For forward looping samples. */
#define FSOUND_LOOP_BIDI             0x00000004  /* For bidirectional looping samples.  (no effect if in hardware). */
#define FSOUND_8BITS                 0x00000008  /* For 8 bit samples. */
#define FSOUND_16BITS                0x00000010  /* For 16 bit samples. */
#define FSOUND_MONO                  0x00000020  /* For mono samples. */
#define FSOUND_STEREO                0x00000040  /* For stereo samples. */
#define FSOUND_UNSIGNED              0x00000080  /* For user created source data containing unsigned samples. */
#define FSOUND_SIGNED                0x00000100  /* For user created source data containing signed data. */
#define FSOUND_MPEG                  0x00000200  /* For MPEG layer 2/3 data. */
#define FSOUND_CHANNELMODE_ALLMONO   0x00000400  /* Sample is a collection of mono channels. */
#define FSOUND_CHANNELMODE_ALLSTEREO 0x00000800  /* Sample is a collection of stereo channel pairs */
#define FSOUND_HW3D                  0x00001000  /* Attempts to make samples use 3d hardware acceleration. (if the card supports it) */
#define FSOUND_2D                    0x00002000  /* Tells software (not hardware) based sample not to be included in 3d processing. */
#define FSOUND_SYNCPOINTS_NONAMES    0x00004000  /* Specifies that syncpoints are present with no names */
#define FSOUND_DUPLICATE             0x00008000  /* This subsound is a duplicate of the previous one i.e. it uses the same sample data but w/different mode bits */
#define FSOUND_CHANNELMODE_PROTOOLS  0x00010000  /* Sample is 6ch and uses L C R LS RS LFE standard. */
#define FSOUND_MPEGACCURATE          0x00020000  /* For FSOUND_Stream_Open - for accurate FSOUND_Stream_GetLengthMs/FSOUND_Stream_SetTime.  WARNING, see FSOUND_Stream_Open for inital opening time performance issues. */
#define FSOUND_HW2D                  0x00080000  /* 2D hardware sounds.  allows hardware specific effects */
#define FSOUND_3D                    0x00100000  /* 3D software sounds */
#define FSOUND_32BITS                0x00200000  /* For 32 bit (float) samples. */
#define FSOUND_IMAADPCM              0x00400000  /* Contents are stored compressed as IMA ADPCM */
#define FSOUND_VAG                   0x00800000  /* For PS2 only - Contents are compressed as Sony VAG format */
#define FSOUND_XMA                   0x01000000  /* For Xbox360 only - Contents are compressed as XMA format */
#define FSOUND_GCADPCM               0x02000000  /* For Gamecube only - Contents are compressed as Gamecube DSP-ADPCM format */
#define FSOUND_MULTICHANNEL          0x04000000  /* For PS2 and Gamecube only - Contents are interleaved into a multi-channel (more than stereo) format */
#define FSOUND_OGG                   0x08000000  /* For vorbis encoded ogg data */
#define FSOUND_CELT                  0x08000000  /* For vorbis encoded ogg data */
#define FSOUND_MPEG_LAYER3           0x10000000  /* Data is in MP3 format. */
#define FSOUND_MPEG_LAYER2           0x00040000  /* Data is in MP2 format. */
#define FSOUND_LOADMEMORYIOP         0x20000000  /* For PS2 only - "name" will be interpreted as a pointer to data for streaming and samples.  The address provided will be an IOP address */
#define FSOUND_IMAADPCMSTEREO        0x20000000  /* Signify IMA ADPCM is actually stereo not two interleaved mono */
#define FSOUND_IGNORETAGS            0x40000000  /* Skips id3v2 etc tag checks when opening a stream, to reduce seek/read overhead when opening files (helps with CD performance) */
#define FSOUND_SYNCPOINTS            0x80000000  /* Specifies that syncpoints are present */

#define FSOUND_CHANNELMODE_MASK      (FSOUND_CHANNELMODE_ALLMONO | FSOUND_CHANNELMODE_ALLSTEREO)  
#define FSOUND_CHANNELMODE_DEFAULT   0x00000000  /* Determine channel assignment automatically from channel count. */
#define FSOUND_CHANNELMODE_RESERVED  0x00000C00  

#define FSOUND_NORMAL                (FSOUND_16BITS | FSOUND_SIGNED | FSOUND_MONO)

#define FSB_SAMPLE_DATA_ALIGN        32

namespace FMOD
{
    typedef struct
    {
        char            id[4];          /* 'FSB3' */
        int             numsamples;     /* number of samples in the file */
        unsigned int    shdrsize;       /* size in bytes of all of the sample headers including extended information */
        unsigned int    datasize;       /* size in bytes of compressed sample data */

        unsigned int    version;        /* extended fsb version */
        unsigned int    mode;           /* flags that apply to all samples in the fsb */
    } FMOD_FSB3_HEADER;  /* 24 bytes */

    typedef struct
    {
        char            id[4];          /* 'FSB4' */
        int             numsamples;     /* number of samples in the file */
        unsigned int    shdrsize;       /* size in bytes of all of the sample headers including extended information */
        unsigned int    datasize;       /* size in bytes of compressed sample data */

        unsigned int    version;        /* extended fsb version */
        unsigned int    mode;           /* flags that apply to all samples in the fsb */
        char            zero[8];
        unsigned char   hash[16];

    } FMOD_FSB_HEADER; /* 48 bytes */

    typedef struct
    {
        unsigned short  size;
        char            name[FMOD_FSB_NAMELEN];

        unsigned int    lengthsamples;
        unsigned int    lengthcompressedbytes;
        unsigned int    loopstart;
        unsigned int    loopend;

        unsigned int    mode;
        int             deffreq;
        unsigned short  defvol;
        short           defpan;
        unsigned short  defpri;
        unsigned short  numchannels;
    #ifndef FMOD_FSB_FORCE_3_0
	    float           mindistance;
	    float           maxdistance;
        int             varfreq;
        unsigned short  varvol;
        short           varpan;
    #endif
    } FMOD_FSB_SAMPLE_HEADER; /* 80 bytes */

    typedef struct
    {
        unsigned int    lengthsamples;
        unsigned int    lengthcompressedbytes;   

    } FMOD_FSB_SAMPLE_HEADER_BASIC; /* 8 bytes */


    class CodecXMA;
    class CodecWav;
    class CodecMPEG;
    class CodecVAG;
    class CodecCELT;
    class ChannelSoftware;
	class ChannelOpenAL;
	class CodecFSB;

    #define FMOD_FSB_USEHEADERCACHE

#ifdef FMOD_FSB_USEHEADERCACHE
    class CodecFSBCache : public LinkedListNode
    {
    public:
        FMOD_FSB_HEADER                mHeader;             /* These should be a GUID that is stored with the FSB. */
        FMOD_FSB_SAMPLE_HEADER       **mShdr;               /* array of sample header pointers */
        FMOD_FSB_SAMPLE_HEADER_BASIC **mShdrb;              /* array of sample header pointers */
        char                          *mShdrData;           /* Pointer to data block */
        int                            mShareCount;
    };
#endif

    class CodecFSB : public Codec
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

        friend class ChannelSoftware;
		friend class ChannelOpenAL;
        friend class DSPCodec;

      private:

        FMOD_PPCALIGN16(FMOD_FSB_HEADER                mHeader);
        FMOD_PPCALIGN16(FMOD_FSB_SAMPLE_HEADER       **mShdr);              /* array of sample header pointers */
        FMOD_PPCALIGN16(FMOD_FSB_SAMPLE_HEADER_BASIC **mShdrb);             /* array of sample header pointers */
        FMOD_PPCALIGN16(FMOD_FSB_SAMPLE_HEADER        *mFirstSample);       /* first sample header */

        unsigned int                  *mDataOffset;        /* array of offsets to raw sample data */
        int                            mCurrentIndex;      /* current FSB index */
#ifdef FMOD_FSB_USEHEADERCACHE
        CodecFSBCache                 *mCacheEntry;
#endif
   
        int                           *mNumSyncPoints;
        SyncPoint                    **mSyncPoint;
        CodecWav                      *mADPCM;
        bool                           mDecodeADPCM;
        CodecMPEG                     *mMPEG;

        int                            mChannels;
        FMOD_MODE                      mUserMode;

        FMOD_RESULT openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT closeInternal();
        FMOD_RESULT readInternal(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype);
        FMOD_RESULT getPositionInternal(unsigned int *position, FMOD_TIMEUNIT postype);
        FMOD_RESULT soundcreateInternal(int subsound, FMOD_SOUND *sound);
        FMOD_RESULT getWaveFormatInternal(int index, FMOD_CODEC_WAVEFORMAT *waveformat);
        FMOD_RESULT resetInternal();
        FMOD_RESULT canPointInternal();

      public:

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
        static FMOD_RESULT F_CALLBACK getPositionCallback(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);
        static FMOD_RESULT F_CALLBACK soundcreateCallback(FMOD_CODEC_STATE *codec, int subsound, FMOD_SOUND *sound);
        static FMOD_RESULT F_CALLBACK getWaveFormatCallback(FMOD_CODEC_STATE *codec_state, int index, FMOD_CODEC_WAVEFORMAT *waveformat);
        static FMOD_RESULT F_CALLBACK resetCallback(FMOD_CODEC_STATE *codec_state);
        static FMOD_RESULT F_CALLBACK canPointCallback(FMOD_CODEC_STATE *codec_state);

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();        

#ifdef FMOD_FSB_USEHEADERCACHE
        static CodecFSBCache gCacheHead;
#endif

        FMOD_FSB_SAMPLE_HEADER **getShdr() { return mShdr; }
    };
}

#endif  /* FMOD_SUPPORT_FSB */

#endif

