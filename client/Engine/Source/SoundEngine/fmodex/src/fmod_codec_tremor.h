#ifndef _FMOD_CODEC_TREMOR_H
#define _FMOD_CODEC_TREMOR_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_TREMOR

#include "fmod_codeci.h"

#include "../lib/tremor_lowmem/ivorbisfile.h"

namespace FMOD
{
    class SyncPoint;

    class CodecTremor : public Codec
    {
      private:
        
        OggVorbis_File        mVf;
        static bool           gInitialized;
                             
        SyncPoint       *mSyncPoint;
        int                   mNumSyncPoints;
        FMOD_CODEC_WAVEFORMAT mWaveFormat;

        FMOD_RESULT openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT closeInternal();
        FMOD_RESULT readInternal(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype);
        FMOD_RESULT readVorbisComments();

      public:

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();       
    };
}

#endif  /* FMOD_SUPPORT_TREMOR */

#endif

