#ifndef _FMOD_CODEC_PLAYLIST_H
#define _FMOD_CODEC_PLAYLIST_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_PLAYLIST

#include "fmod_codeci.h"

namespace FMOD
{
    class CodecPlaylist : public Codec
    {
      private:

        FMOD_RESULT openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT closeInternal();

        FMOD_RESULT readASX();
        FMOD_RESULT readWPL();
        FMOD_RESULT readM3U();
        FMOD_RESULT readPLS();
        FMOD_RESULT readB4S();
        FMOD_RESULT readSimple();

        FMOD_RESULT getNextXMLTag(char *tagname, int *tagnamesize, char *tagdata, int *tagdatasize);
        FMOD_RESULT getPLSToken(char *buffer, int length, int *tokensize);
        FMOD_RESULT readLine(char *buffer, int length, int *linelength);
        FMOD_RESULT skipSimpleComments();
        FMOD_RESULT getQuoteData(const char *tag, char *buffer, int *datasize);
        
        FMOD_RESULT skipWhiteSpace(int *numwhitespaces);
        bool        isNewLine(char c);

        FMOD_CODEC_WAVEFORMAT mWaveFormat;

      public:

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
        static FMOD_RESULT F_CALLBACK soundcreateCallback(FMOD_CODEC_STATE *codec, int subsound, FMOD_SOUND *sound);

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();
    };
}

#endif  /* FMOD_SUPPORT_PLAYLIST */

#endif


