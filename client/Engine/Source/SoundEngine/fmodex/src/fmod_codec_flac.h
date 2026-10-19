#ifndef _FMOD_CODEC_FLAC_H
#define _FMOD_CODEC_FLAC_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_FLAC

#include "fmod_codeci.h"

#include "../lib/flac-1.1.4/include/FLAC/all.h"

namespace FMOD
{
    class CodecFLAC : public Codec
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:
        
        FLAC__StreamDecoder *mDecoder;
        bool                 mFrameReady;   

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

#endif  /* FMOD_SUPPORT_FLAC */

#endif


