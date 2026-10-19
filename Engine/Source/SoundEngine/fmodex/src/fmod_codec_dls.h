#ifndef _FMOD_CODEC_DLS_H
#define _FMOD_CODEC_DLS_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_DLS

#include "fmod_codeci.h"

namespace FMOD
{
    ///////////////////////////////////////////////////////////////////////////
    // Articulation connection graph definitions
    ///////////////////////////////////////////////////////////////////////////
    // Generic Sources
    #define CONN_SRC_NONE               0x0000
    #define CONN_SRC_LFO                0x0001
    #define CONN_SRC_KEYONVELOCITY      0x0002
    #define CONN_SRC_KEYNUMBER          0x0003
    #define CONN_SRC_EG1                0x0004
    #define CONN_SRC_EG2                0x0005
    #define CONN_SRC_PITCHWHEEL         0x0006

    // Midi Controllers 0-127
    #define CONN_SRC_CC1                0x0081      // mod wheel
    #define CONN_SRC_CC7                0x0087      // Channel volume
    #define CONN_SRC_CC10               0x008a      // Pan
    #define CONN_SRC_CC11               0x008b      // Expression.

    // Generic Destinations
    #define CONN_DST_NONE               0x0000
    #define CONN_DST_ATTENUATION        0x0001
    #define CONN_DST_RESERVED           0x0002
    #define CONN_DST_PITCH              0x0003
    #define CONN_DST_PAN                0x0004

    // LFO Destinations
    #define CONN_DST_LFO_FREQUENCY      0x0104
    #define CONN_DST_LFO_STARTDELAY     0x0105

    // EG1 Destinations
    #define CONN_DST_EG1_ATTACKTIME     0x0206
    #define CONN_DST_EG1_DECAYTIME      0x0207
    #define CONN_DST_EG1_RESERVED       0x0208
    #define CONN_DST_EG1_RELEASETIME    0x0209
    #define CONN_DST_EG1_SUSTAINLEVEL   0x020a

    // EG2 Destinations
    #define CONN_DST_EG2_ATTACKTIME     0x030a
    #define CONN_DST_EG2_DECAYTIME      0x030b
    #define CONN_DST_EG2_RESERVED       0x030c
    #define CONN_DST_EG2_RELEASETIME    0x030d
    #define CONN_DST_EG2_SUSTAINLEVEL   0x030e

    // Envelope transform type
    #define CONN_TRN_NONE               0x0000
    #define CONN_TRN_CONCAVE            0x0001

    #define F_RGN_OPTION_SELFNONEXCLUSIVE 0x0001

    typedef enum
    {
        CONN_SRC_FLAG_NONE          = 0x0001,
        CONN_SRC_FLAG_LFO           = 0x0002,
        CONN_SRC_FLAG_KEYONVELOCITY = 0x0004,
        CONN_SRC_FLAG_KEYNUMBER     = 0x0008,
        CONN_SRC_FLAG_EG1           = 0x0010,
        CONN_SRC_FLAG_EG2           = 0x0020,
        CONN_SRC_FLAG_PITCHWHEEL    = 0x0040,
        CONN_SRC_FLAG_MODWHEEL      = 0x0080,
        CONN_SRC_FLAG_CHANNELVOL    = 0x0100,
        CONN_SRC_FLAG_PAN           = 0x0200,
        CONN_SRC_FLAG_EXPRESSION    = 0x0400
    } CONN_SRC_FLAGS;

    #define CONN_SRC_FLAG_COMMON (CONN_SRC_FLAGS)(CONN_SRC_FLAG_NONE |              \
                                                  CONN_SRC_FLAG_KEYONVELOCITY |     \
                                                  CONN_SRC_FLAG_KEYNUMBER |         \
                                                  CONN_SRC_FLAG_PITCHWHEEL |        \
                                                  CONN_SRC_FLAG_MODWHEEL |          \
                                                  CONN_SRC_FLAG_CHANNELVOL |        \
                                                  CONN_SRC_FLAG_PAN |               \
                                                  CONN_SRC_FLAG_EXPRESSION)

    #ifdef FMOD_SUPPORT_PRAGMAPACK
	    #pragma pack(1)
    #endif

    typedef struct
    {
        unsigned int ulBank       FMOD_PACKED_INTERNAL;
        unsigned int ulInstrument FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  DLS_MIDILOCALE;

    typedef struct
    {
        unsigned int    cRegions FMOD_PACKED_INTERNAL;
        DLS_MIDILOCALE  Locale   FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  DLS_INSTRUMENTHEADER;
    

    typedef struct _RGNRANGE
    {
        unsigned short usLow  FMOD_PACKED_INTERNAL;      /* Low Value of Range */
        unsigned short usHigh FMOD_PACKED_INTERNAL;      /* High Value of Range*/
    } FMOD_PACKED  DLS_RGNRANGE;

    typedef struct
    {
        DLS_RGNRANGE    RangeKey      FMOD_PACKED_INTERNAL;
        DLS_RGNRANGE    RangeVelocity FMOD_PACKED_INTERNAL;
        unsigned short  fusOptions    FMOD_PACKED_INTERNAL;
        unsigned short  usKeyGroup    FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  DLS_REGIONHEADER;

    typedef struct
    {
        unsigned short  fusOptions    FMOD_PACKED_INTERNAL;
        unsigned short  usPhaseGroup  FMOD_PACKED_INTERNAL;
        unsigned int    ulChannel     FMOD_PACKED_INTERNAL;
        unsigned int    ulTableIndex  FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  DLS_WAVELINK;            

    typedef struct
    {
        unsigned int    cbSize          FMOD_PACKED_INTERNAL;
        unsigned int    ulLoopType      FMOD_PACKED_INTERNAL;
        unsigned int    ulLoopStart     FMOD_PACKED_INTERNAL;
        unsigned int    ulLoopLength    FMOD_PACKED_INTERNAL; 
    } FMOD_PACKED  DLS_WAVESAMPLELOOP;

    typedef struct
    {
        unsigned int        cbSize          FMOD_PACKED_INTERNAL;
        unsigned short      usUnityNote     FMOD_PACKED_INTERNAL;
        signed short        sFineTune       FMOD_PACKED_INTERNAL;
        signed int          lAttenuation    FMOD_PACKED_INTERNAL;
        unsigned int        fulOptions      FMOD_PACKED_INTERNAL;
        unsigned int        cSampleLoops    FMOD_PACKED_INTERNAL;
        DLS_WAVESAMPLELOOP  loop[1]         FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  DLS_WAVESAMPLE;

    typedef struct
    {
        unsigned short      usSource        FMOD_PACKED_INTERNAL;
        unsigned short      usControl       FMOD_PACKED_INTERNAL;
        unsigned short      usDestination   FMOD_PACKED_INTERNAL;
        unsigned short      usTransform     FMOD_PACKED_INTERNAL;
        int                 lScale          FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  DLS_CONNECTIONBLOCK;

    #ifdef FMOD_SUPPORT_PRAGMAPACK
        #ifdef 	CODEWARRIOR
        #pragma pack(0)
        #else
        #pragma pack()
        #endif
    #endif

    typedef struct
    {
        float mTime;
        float mSrcValue, mDestValue;
    } CodecDLSEnvelopePoint;

    typedef enum
    {
        CODEC_DLS_ENVPOINT_ATTACK,
        CODEC_DLS_ENVPOINT_DECAY,
        CODEC_DLS_ENVPOINT_RELEASE,
        CODEC_DLS_ENVPOINT_MAX
    } CODEC_DLS_ENVPOINT;

    class CodecDLSEnvelope
    {
      public:

        CodecDLSEnvelopePoint   mPoint[CODEC_DLS_ENVPOINT_MAX]; /* Attack, decay, release.  Sustain is the point decay ends.  */

        CODEC_DLS_ENVPOINT      mPosition;
        float                   mTime;
        float                   mSustain;
        float                   mRange;
        bool                    mActive;
    };


    class CodecDLSRegion
    {
      public:
        DLS_REGIONHEADER      mRegionHeader;
        DLS_WAVESAMPLE        mWaveSample;
        DLS_WAVELINK          mWaveLink;
        int                   mNumConnectionBlocks;
        DLS_CONNECTIONBLOCK  *mConnectionBlock;
    };

    class CodecDLSInstrument
    {
      public:
        char                  mName[FMOD_STRING_MAXNAMELEN];
        DLS_INSTRUMENTHEADER  mHeader;
        CodecDLSRegion       *mRegion;
        int                   mNumConnectionBlocks;
        DLS_CONNECTIONBLOCK  *mConnectionBlock;
    };

    class CodecDLSSample
    {
      public:
        char                mName[FMOD_STRING_MAXNAMELEN];
        unsigned int        mDataOffset;
        DLS_WAVESAMPLE      mWaveSample;
    };

    class CodecDLS : public Codec
    {
      public:

        int                 mNumInstruments;
        int                 mInstrumentID;
        CodecDLSInstrument *mInstrument;

        int                 mNumSamples;
        int                 mSampleID;
        CodecDLSSample     *mSample;

        int                 mCurrentIndex;      /* current DLS index */
        unsigned int        mRegionID;          /* This should always be 0-15 with melodic instruments. */

        FMOD_RESULT         parseChunk(char *parentchunk, unsigned int chunksize);

        FMOD_RESULT         openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT         closeInternal();
        FMOD_RESULT         readInternal(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT         setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype);

      public:

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();        
    };
}

#endif  /* FMOD_SUPPORT_DLS */

#endif

