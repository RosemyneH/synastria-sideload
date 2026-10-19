#ifndef _FMOD_CODEC_IT_H
#define _FMOD_CODEC_IT_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_IT

#include "fmod_music.h"

namespace FMOD
{
    class DSPI;
    class ChannelPool;

    enum FMUSIC_ITCOMMANDS
    {
	    FMUSIC_IT_SETSPEED = 1,			// always
	    FMUSIC_IT_PATTERNJUMP,			// always
	    FMUSIC_IT_PATTERNBREAK,			// always
	    FMUSIC_IT_VOLUMESLIDE,			// always set, foreground channel only execute
	    FMUSIC_IT_PORTADOWN,			// always set, foreground channel only execute
	    FMUSIC_IT_PORTAUP,				// always set, foreground channel only execute
	    FMUSIC_IT_PORTATO,				// always set, foreground channel only execute
	    FMUSIC_IT_VIBRATO,				// always set, foreground channel only execute
	    FMUSIC_IT_TREMOR,				// always set, foreground channel only execute
	    FMUSIC_IT_ARPEGGIO,				// always set, foreground channel only execute
	    FMUSIC_IT_VIBRATOVOLSLIDE,		// always set, foreground channel only execute
	    FMUSIC_IT_PORTATOVOLSLIDE,		// always set, foreground channel only execute
	    FMUSIC_IT_SETCHANNELVOLUME,		// always
	    FMUSIC_IT_CHANNELVOLSLIDE,		// always
	    FMUSIC_IT_SETSAMPLEOFFSET,		// always set, foreground channel only execute
	    FMUSIC_IT_PANSLIDE,	        	// always set, foreground channel only execute
	    FMUSIC_IT_RETRIGVOLSLIDE,		// always set, foreground channel only execute
	    FMUSIC_IT_TREMOLO,				// always set, foreground channel only execute
	    FMUSIC_IT_SPECIAL,				// 
	    FMUSIC_IT_SETTEMPO,				// always
	    FMUSIC_IT_FINEVIBRATO,			// always set, foreground channel only execute
	    FMUSIC_IT_SETGLOBALVOLUME,		// always
	    FMUSIC_IT_GLOBALVOLUMESLIDE,	// always
	    FMUSIC_IT_SETPAN,				// foreground channel only
	    FMUSIC_IT_PANBRELLO,			// foreground channel only
	    FMUSIC_IT_MIDIMACROS			// 
    };

    enum FMUSIC_ITCOMMANDS_SPECIAL
    {
	    FMUSIC_IT_S0,					// 
	    FMUSIC_IT_S1,					// 
	    FMUSIC_IT_S2,					// 
	    FMUSIC_IT_SETVIBRATOWAVE,		//  
	    FMUSIC_IT_SETTREMOLOWAVE,		//  
	    FMUSIC_IT_SETPANBRELLOWAVE,		//  
	    FMUSIC_IT_PATTERNDELAYTICKS,	// 
	    FMUSIC_IT_S7SPECIAL,			// 
	    FMUSIC_IT_SETPANPOSITION16,		// 
	    FMUSIC_IT_S9SPECIAL,		    // 
	    FMUSIC_IT_SETHIGHOFFSET,		// 
	    FMUSIC_IT_PATTERNLOOP,			// 
	    FMUSIC_IT_NOTECUT,				// 
	    FMUSIC_IT_NOTEDELAY,			// 
	    FMUSIC_IT_PATTERNDELAY,			// 
	    FMUSIC_IT_SF
    };

    enum FMUSIC_ITCOMMANDS_S7SPECIAL
    {
	    FMUSIC_IT_PASTNOTECUT,			// 
	    FMUSIC_IT_PASTNOTEOFF,			// 
	    FMUSIC_IT_PASTNOTEFADE,			// 
	    FMUSIC_IT_SETNNACUT,			// 
	    FMUSIC_IT_SETNNACONTINUE,		// 
	    FMUSIC_IT_SETNNANOTEOFF,		// 
	    FMUSIC_IT_SETNNANOTEFADE,		// 
	    FMUSIC_IT_VOLENVELOPEOFF,		// 
	    FMUSIC_IT_VOLENVELOPEON,		// 
	    FMUSIC_IT_S79,
	    FMUSIC_IT_S7A,
	    FMUSIC_IT_S7B,
	    FMUSIC_IT_S7C,
	    FMUSIC_IT_S7D,
	    FMUSIC_IT_S7E,
	    FMUSIC_IT_S7F
    };

    enum FMUSIC_ITCOMMANDS_S9SPECIAL
    {
	    FMUSIC_IT_DISABLESURROUND,	    
	    FMUSIC_IT_ENABLESURROUND,
	    FMUSIC_IT_S92,
	    FMUSIC_IT_S93,
	    FMUSIC_IT_S94,
	    FMUSIC_IT_S95,
	    FMUSIC_IT_S96,
	    FMUSIC_IT_S97,
	    FMUSIC_IT_DISABLEREVERB,
	    FMUSIC_IT_FORCEREVERB,
	    FMUSIC_IT_MONOSURROUND,
	    FMUSIC_IT_QUADSURROUND,
	    FMUSIC_IT_GLOBALFILTER,
	    FMUSIC_IT_LOCALFILTER,
	    FMUSIC_IT_PLAYFORWARD,
	    FMUSIC_IT_PLAYBACKWARD
    };

    #define FMUSIC_ITFLAGS_NONE					0x0
    #define FMUSIC_ITFLAGS_MONOSTEREO			0x1
    #define FMUSIC_ITFLAGS_VOL0OPTIMIZATIONS	0x2
    #define FMUSIC_ITFLAGS_INSTRUMENTS			0x4
    #define FMUSIC_ITFLAGS_LINEARFREQUENCY		0x8
    #define FMUSIC_ITFLAGS_OLD_IT_EFFECTS		0x10
    #define FMUSIC_ITFLAGS_EFFECT_G				0x20
    #define FMUSIC_ITFLAGS_EMBEDMIDICFG         0x80
    #define FMUSIC_ITFLAGS_EXTENDFILTERRANGE    0x1000
    
    #define MAX_MIXPLUGINS		50
   
    typedef struct MODMIDICFG
    {
	    char szMidiGlb[9*32];
	    char szMidiSFXExt[16*32];
	    char szMidiZXXExt[128*32];
    } MODMIDICFG, *LPMODMIDICFG;

    const int IT_MAXROWS    = 256;
    const int IT_MAXSAMPLES = 256;

    class CodecIT : public MusicSong
    {
      private:
        
        MusicSample     **mSample;
        MusicSample       mSampleMem[IT_MAXSAMPLES];
        signed char      *mPatternPtr;
        unsigned int     *mSrcBuffer;                     /* source buffer */
        unsigned int     *mSrcPos;                        /* actual reading position */
        unsigned char     mSrcRemBits;                    /* bits remaining in actual read dword */
        int               mNumRealChannels;
        SNDMIXPLUGIN     *mMixPlugin[MAX_MIXPLUGINS];
        unsigned int      mChannelPlugin[64];
        DSPI             *mDSPFinalHead;                 /* Extra unit that comes before mDSPUnit for submixing purposes. */
        DSPI             *mDSPEffectHead;

        FMOD_RESULT     calculateLength();

        FMOD_RESULT     readBits(unsigned char b, unsigned int *result);	/* reads b bits from the stream */
        FMOD_RESULT     readBlock(signed char **buff);                      /* gets block of compressed data from file */
        FMOD_RESULT     freeBlock();                                        /* frees that block again */
        FMOD_RESULT     decompress8(void **src, void *dst, int len, bool it215, int channels);
        FMOD_RESULT     decompress16(void **src, void *dst, int len, bool it215, int channels);
        FMOD_RESULT     cutOffToFrequency(unsigned int nCutOff, int flt_modifier, float *freq);
        FMOD_RESULT     setupChannelFilter(MusicVirtualChannel *vcptr, bool bReset, int flt_modifier);

        FMOD_RESULT     updateRow(bool audible);                        
        FMOD_RESULT     updateEffects();
        FMOD_RESULT     update(bool audible);
        
        FMOD_RESULT     unpackRow();
        FMOD_RESULT     processEnvelope(MusicEnvelopeState *env, MusicVirtualChannel *vcptr, int Inumpoints, MusicEnvelopeNode *v, int type, int loopstart, int loopend, int susloopstart, int susloopend, unsigned char control);
        FMOD_RESULT     processPitchEnvelope(MusicVirtualChannel *vcptr, MusicInstrument *iptr, int note);
        FMOD_RESULT     sampleVibrato(MusicVirtualChannel *vcptr);
        FMOD_RESULT     play();

        FMOD_RESULT     openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT     closeInternal();
        FMOD_RESULT     readInternal(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT     setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype);

      public:

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();  
        
    };

    class MusicChannelIT : public MusicChannel
    {
      public:
        CodecIT *mModule;

        FMOD_RESULT volumeSlide();
        FMOD_RESULT panSlide();
        FMOD_RESULT portamento();
        FMOD_RESULT vibrato();
        FMOD_RESULT tremolo();
        FMOD_RESULT fineVibrato();
        FMOD_RESULT panbrello();
        FMOD_RESULT processVolumeByte(MusicNote *current, bool newrow);
    };

}

#endif  /* FMOD_SUPPORT_IT */

#endif


