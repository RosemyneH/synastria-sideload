#ifndef _FMOD_CODEC_XM_H
#define _FMOD_CODEC_XM_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_XM

#include "fmod_music.h"

namespace FMOD
{
    class DSPI;
    class ChannelPool;

    enum FMUSIC_XMCOMMANDS
    {
	    FMUSIC_XM_ARPEGGIO,
	    FMUSIC_XM_PORTAUP,
	    FMUSIC_XM_PORTADOWN,
	    FMUSIC_XM_PORTATO,
	    FMUSIC_XM_VIBRATO,
	    FMUSIC_XM_PORTATOVOLSLIDE,
	    FMUSIC_XM_VIBRATOVOLSLIDE,
	    FMUSIC_XM_TREMOLO,
	    FMUSIC_XM_SETPANPOSITION,
	    FMUSIC_XM_SETSAMPLEOFFSET,
	    FMUSIC_XM_VOLUMESLIDE,
	    FMUSIC_XM_PATTERNJUMP,
	    FMUSIC_XM_SETVOLUME,
	    FMUSIC_XM_PATTERNBREAK,
	    FMUSIC_XM_SPECIAL,
	    FMUSIC_XM_SETSPEED,
	    FMUSIC_XM_SETGLOBALVOLUME,
	    FMUSIC_XM_GLOBALVOLSLIDE,
	    FMUSIC_XM_I,
	    FMUSIC_XM_J,
	    FMUSIC_XM_KEYOFF,
	    FMUSIC_XM_SETENVELOPEPOS,
	    FMUSIC_XM_M,
	    FMUSIC_XM_N,
	    FMUSIC_XM_O,
	    FMUSIC_XM_PANSLIDE,
	    FMUSIC_XM_Q,
	    FMUSIC_XM_MULTIRETRIG,
	    FMUSIC_XM_S,
	    FMUSIC_XM_TREMOR,
	    FMUSIC_XM_U,
	    FMUSIC_XM_V,
	    FMUSIC_XM_W,
	    FMUSIC_XM_EXTRAFINEPORTA,
	    FMUSIC_XM_Y,
	    FMUSIC_XM_Z
    };


    enum FMUSIC_XMCOMMANDSSPECIAL
    {
	    FMUSIC_XM_SETFILTER,
	    FMUSIC_XM_FINEPORTAUP,
	    FMUSIC_XM_FINEPORTADOWN,
	    FMUSIC_XM_SETGLISSANDO,
	    FMUSIC_XM_SETVIBRATOWAVE,
	    FMUSIC_XM_SETFINETUNE,
	    FMUSIC_XM_PATTERNLOOP,
	    FMUSIC_XM_SETTREMOLOWAVE,
	    FMUSIC_XM_SETPANPOSITION16,
	    FMUSIC_XM_RETRIG,
	    FMUSIC_XM_FINEVOLUMESLIDEUP,
	    FMUSIC_XM_FINEVOLUMESLIDEDOWN,
	    FMUSIC_XM_NOTECUT,
	    FMUSIC_XM_NOTEDELAY,
	    FMUSIC_XM_PATTERNDELAY,
	    FMUSIC_XM_FUNKREPEAT
    };

    #define FMUSIC_XMFLAGS_LINEARFREQUENCY		1

    typedef struct
    {
        unsigned int 	instSize;               /* instrument size */
        signed char		instName[22];           /* instrument filename */
        unsigned char	instType;               /* instrument type (now 0) */
        unsigned short  numSamples;             /* number of samples in instrument */
    } FMUSIC_XM_INSTHEADER;

    typedef struct
    {
        unsigned int 	headerSize;             /* sample header size */
        unsigned char	noteSmpNums[96];        /* sample numbers for notes */
        unsigned short  volEnvelope[2*12];      /* volume envelope points */
        unsigned short  panEnvelope[2*12];      /* panning envelope points */
        unsigned char   numVolPoints;           /* number of volume envelope points */
        unsigned char   numPanPoints;           /* number of panning env. points */
        unsigned char   volSustain;             /* volume sustain point */
        unsigned char   volLoopStart;           /* volume loop start point */
        unsigned char   volLoopEnd;             /* volume loop end point */
        unsigned char   panSustain;             /* panning sustain point */
        unsigned char   panLoopStart;           /* panning loop start point */
        unsigned char   panLoopEnd;             /* panning loop end point */
        unsigned char   volEnvFlags;            /* volume envelope flags */
        unsigned char   panEnvFlags;            /* panning envelope flags */

        unsigned char   vibType;                /* vibrato type */
        unsigned char   vibSweep;               /* vibrato sweep */
        unsigned char   vibDepth;               /* vibrato depth */
        unsigned char   vibRate;                /* vibrato rate */
        unsigned short  volFadeout;             /* volume fadeout */
        unsigned short  reserved;
    } FMUSIC_XM_INSTSAMPLEHEADER;
   
    const int XM_MAXROWS    = 256;
    const int XM_MAXSAMPLES = 256;

    class MusicChannelXM : public MusicChannel
    {
      public:
        FMOD_RESULT     volumeSlide();
        FMOD_RESULT     portamento();        
        FMOD_RESULT     vibrato();
        FMOD_RESULT     tremolo();
        FMOD_RESULT     fineVibrato();
        FMOD_RESULT     processVolumeByte(unsigned char volume);
        FMOD_RESULT     instrumentVibrato(MusicInstrument *iptr);
    };

    class CodecXM : public MusicSong
    {
      private:
        
        MusicSample   **mSample;

        FMOD_RESULT     calculateLength();

        FMOD_RESULT     updateNote(bool audible);                        
        FMOD_RESULT     updateEffects();
        FMOD_RESULT     update(bool audible);

        FMOD_RESULT     spawnNewChannel(MusicChannel *cptr, MusicVirtualChannel *oldvcptr, MusicSample *sptr, MusicVirtualChannel **newvcptr);
        FMOD_RESULT     processEnvelope(MusicEnvelopeState *env, MusicVirtualChannel *vcptr, int Inumpoints, unsigned short *points, int type, int loopstart, int loopend, unsigned char ISustain, unsigned char control);
        FMOD_RESULT     getAmigaPeriod(int note, int finetune, int *period);
        FMOD_RESULT     processNote(MusicNote *current, MusicChannelXM *cptr, MusicVirtualChannel *vcptr, MusicInstrument *iptr, MusicSample *sptr);
        FMOD_RESULT     updateFlags(MusicVirtualChannel *vcptr, MusicSample *sptr);

#ifdef FMOD_SUPPORT_VAG
        void            EncodeVAG_FindPredict( short *samples, float *d_samples, int *predict_nr, int *shift_factor );
        static void     EncodeVAG_pack( float *d_samples, short *four_bit, int predict_nr, int shift_factor );
        int             EncodeVAG_pcm2vag(unsigned char *vag, short *wave, int sample_len, int wavebits);
#endif

        FMOD_RESULT     openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT     closeInternal();
        FMOD_RESULT     readInternal(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT     setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype);

      public:

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();  

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
    };
}

#endif  /* FMOD_SUPPORT_XM */

#endif


