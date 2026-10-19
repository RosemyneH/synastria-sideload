#ifndef _FMOD_CODEC_TAG_H
#define _FMOD_CODEC_TAG_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_TAGS

#include "fmod_codeci.h"

namespace FMOD
{
    const FMOD_SOUND_TYPE FMOD_SOUND_TYPE_TAG = (FMOD_SOUND_TYPE)1000;

    class CodecTag : public Codec
    {
      private:

        FMOD_RESULT openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT closeInternal();
        FMOD_RESULT readTags();
        FMOD_RESULT readID3v1();
        FMOD_RESULT readID3v2();
        FMOD_RESULT readID3v2FromFooter();
        FMOD_RESULT readASF();

      public:

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
        static FMOD_RESULT F_CALLBACK soundcreateCallback(FMOD_CODEC_STATE *codec, int subsound, FMOD_SOUND *sound);

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();
    };
}

#endif  /* FMOD_SUPPORT_TAGS */

#endif


