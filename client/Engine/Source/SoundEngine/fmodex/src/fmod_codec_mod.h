#ifndef _FMOD_CODEC_MOD_H
#define _FMOD_CODEC_MOD_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_MOD

#include "fmod_music.h"

namespace FMOD
{
    class DSPI;
    class ChannelPool;

    typedef enum
    {
	    FMUSIC_MOD_ARPEGGIO,
	    FMUSIC_MOD_PORTAUP,
	    FMUSIC_MOD_PORTADOWN,
	    FMUSIC_MOD_PORTATO,
	    FMUSIC_MOD_VIBRATO,
	    FMUSIC_MOD_PORTATOVOLSLIDE,
	    FMUSIC_MOD_VIBRATOVOLSLIDE,
	    FMUSIC_MOD_TREMOLO,
	    FMUSIC_MOD_SETPANPOSITION,
	    FMUSIC_MOD_SETSAMPLEOFFSET,
	    FMUSIC_MOD_VOLUMESLIDE,
	    FMUSIC_MOD_PATTERNJUMP,
	    FMUSIC_MOD_SETVOLUME,
	    FMUSIC_MOD_PATTERNBREAK,
	    FMUSIC_MOD_SPECIAL,
	    FMUSIC_MOD_SETSPEED
    } MODCOMMANDS;

    typedef enum
    {
	    FMUSIC_MOD_SETFILTER,
	    FMUSIC_MOD_FINEPORTAUP,
	    FMUSIC_MOD_FINEPORTADOWN,
	    FMUSIC_MOD_SETGLISSANDO,
	    FMUSIC_MOD_SETVIBRATOWAV,
	    FMUSIC_MOD_SETFINETUNE,
	    FMUSIC_MOD_PATTERNLOOP,
	    FMUSIC_MOD_SETTREMOLOWAV,
	    FMUSIC_MOD_SETPANPOSITION16,
	    FMUSIC_MOD_RETRIG,
	    FMUSIC_MOD_FINEVOLUMESLIDEUP,
	    FMUSIC_MOD_FINEVOLUMESLIDEDOWN,
	    FMUSIC_MOD_NOTECUT,
	    FMUSIC_MOD_NOTEDELAY,
	    FMUSIC_MOD_PATTERNDELAY,
	    FMUSIC_MOD_FUNKREPEAT
    } MODCOMMANDSSPECIAL;

    const int MOD_MAXROWS = 64;
    const int MOD_MAXSAMPLES = 31;

    class MusicChannelMOD : public MusicChannel
    {
      public:
        FMOD_RESULT portamento();
        FMOD_RESULT vibrato();        
        FMOD_RESULT tremolo();
    };

    class CodecMOD : public MusicSong
    {
      private:
        
        MusicSample      mSample[MOD_MAXSAMPLES];

        FMOD_RESULT      calculateLength();
        inline int       getAmigaPeriod(int note, int middlec);

        FMOD_RESULT      updateNote(bool audible);                        
        FMOD_RESULT      updateEffects();
        FMOD_RESULT      update(bool audible);

        FMOD_RESULT      openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT      closeInternal();
        FMOD_RESULT      readInternal(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT      setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype);

      public:

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();  
        
    };
}

#endif  /* FMOD_SUPPORT_MOD */

#endif

