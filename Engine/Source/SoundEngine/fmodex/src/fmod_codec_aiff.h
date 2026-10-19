#ifndef _FMOD_CODEC_AIFF_H
#define _FMOD_CODEC_AIFF_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_AIFF

#include "fmod_codeci.h"
#include "fmod_types.h"

namespace FMOD
{

    #ifdef FMOD_SUPPORT_PRAGMAPACK
        #pragma pack(1)
    #endif

    typedef struct
    {
        signed char id[4]   FMOD_PACKED_INTERNAL;
        int         size    FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  AIFF_CHUNK;

    typedef struct
    { 
        short           numChannels     FMOD_PACKED_INTERNAL;
        unsigned int    numSampleFrames FMOD_PACKED_INTERNAL;
        short           sampleSize      FMOD_PACKED_INTERNAL;
        unsigned char   eSampleRate[10] FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  AIFF_COMMONCHUNK;

    typedef struct
    {
        short           numChannels             FMOD_PACKED_INTERNAL;
        unsigned int    numSampleFrames         FMOD_PACKED_INTERNAL;
        short           sampleSize              FMOD_PACKED_INTERNAL;
        unsigned char   eSampleRate[10]         FMOD_PACKED_INTERNAL;
        char            compressionid[4];       FMOD_PACKED_INTERNAL;
        unsigned char   compressionName[256];   FMOD_PACKED_INTERNAL;
        unsigned char   compressionNameLength;  FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  AIFC_COMMONCHUNK;

    typedef struct
    { 
        unsigned int offset             FMOD_PACKED_INTERNAL;
        unsigned int blockSize          FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  AIFF_SOUNDDATACHUNK; 

    typedef struct
    { 
        short           id           FMOD_PACKED_INTERNAL;
        unsigned int    position     FMOD_PACKED_INTERNAL;
        unsigned char   markerlength FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  AIFF_MARKER; 

    typedef struct
    { 
        short           playMode  FMOD_PACKED_INTERNAL;
        short           beginLoop FMOD_PACKED_INTERNAL;
        short           endLoop   FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  AIFF_LOOP;

    typedef struct
    { 
        char            baseNote     FMOD_PACKED_INTERNAL;
        char            detune       FMOD_PACKED_INTERNAL;
        char            lowNote      FMOD_PACKED_INTERNAL;
        char            highNote     FMOD_PACKED_INTERNAL;
        char            lowVelocity  FMOD_PACKED_INTERNAL;
        char            highVelocity FMOD_PACKED_INTERNAL;
        short           gain         FMOD_PACKED_INTERNAL;
        AIFF_LOOP       sustainLoop  FMOD_PACKED_INTERNAL;
        AIFF_LOOP       releaseLoop  FMOD_PACKED_INTERNAL;
    } FMOD_PACKED  AIFF_INSTRUMENTCHUNK;

    #ifdef FMOD_SUPPORT_PRAGMAPACK
        #ifdef 	CODEWARRIOR
        #pragma pack(0)
        #else
        #pragma pack()
        #endif
    #endif

    class CodecAIFF : public Codec
    {
      private:
      
        bool mIsAIFC;
        bool mLittleEndian;
        
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

#endif  /* FMOD_SUPPORT_AIFF */

#endif


