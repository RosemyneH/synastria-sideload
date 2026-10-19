#ifndef _FMOD_MUSIC_H
#define _FMOD_MUSIC_H

#include "fmod_settings.h"

#if defined(FMOD_SUPPORT_MOD) || defined(FMOD_SUPPORT_S3M) || defined(FMOD_SUPPORT_XM) || defined(FMOD_SUPPORT_IT)

#include "fmod.h"
#include "fmod_codeci.h"
#include "fmod_channel_real.h"
#include "fmod_channeli.h"
#include "fmod_channelgroupi.h"
#include "fmod_channelpool.h"
#include "fmod_linkedlist.h"
#include "fmod_string.h"
#include "fmod_types.h"

namespace FMOD
{
    class MusicSong;
    class SoundI;
    class ChannelSoftware;

    const int FMUSIC_FLAGS_NORMALVOLUMESLIDES	= 0;
    const int FMUSIC_FLAGS_FASTVOLUMESLIDES		= 1;
    /*const int FMUSIC_FLAGS_LINEARFREQUENCY	= 2; */
    /*const int FMUSIC_FLAGS_AMIGAFREQUENCY		= 4; */

    const int FMUSIC_MAXORDERS					= 256;
    const int FMUSIC_MAXROWS					= 256;

    const int FMUSIC_KEYCUT						= 254;
    const int FMUSIC_KEYOFF						= 255;

    const int FMUSIC_FREQ                       = 0x1;
    const int FMUSIC_VOLUME						= 0x2;
    const int FMUSIC_PAN						= 0x4;
    const int FMUSIC_TRIGGER					= 0x8;
    const int FMUSIC_SURROUND					= 0x10;
    const int FMUSIC_STOP						= 0x20;
    const int FMUSIC_SURROUNDOFF				= 0x40;
    const int FMUSIC_SETPAN                     = 0x80;

    const int FMUSIC_ENVELOPE_OFF				= 0x0;
    const int FMUSIC_ENVELOPE_ON				= 0x1;
    const int FMUSIC_ENVELOPE_SUSTAIN			= 0x2;
    const int FMUSIC_ENVELOPE_LOOP				= 0x4;
    const int FMUSIC_ENVELOPE_CARRY				= 0x8;
    const int FMUSIC_ENVELOPE_FILTER            = 0x10;

    const int FMUSIC_CUT						= 0;
    const int FMUSIC_CONTINUE					= 1;
    const int FMUSIC_OFF						= 2;
    const int FMUSIC_FADEOUT                    = 3;

    const int FMUSIC_FILTER_PRECISION           = 8192;


    #ifndef PLATFORM_GC
        #ifdef FMOD_SUPPORT_PRAGMAPACK
	        #pragma pack(1)
        #endif
    #endif

    typedef struct MusicEnvelopeNode
    {
	    signed char     mValue  FMOD_PACKED_INTERNAL;
	    unsigned short  mTick   FMOD_PACKED_INTERNAL;
    } FMOD_PACKED MusicEnvelopeNode;

    #ifndef PLATFORM_GC
        #ifdef FMOD_SUPPORT_PRAGMAPACK
            #ifdef 	CODEWARRIOR
            #pragma pack(0)
            #else
            #pragma pack()
            #endif
        #endif
    #endif

    typedef struct _SNDMIXPLUGININFO
    {
	    unsigned int dwPluginId1;
	    unsigned int dwPluginId2;
	    unsigned int dwInputRouting;	    // MIXPLUG_INPUTF_XXXX
	    unsigned int dwOutputRouting;	    // 0=mix 0x80+=fx
	    unsigned int dwReserved[4];	        // Reserved for routing info
	    char         szName[32];
	    char         szLibraryName[64];	    // original DLL name
    } SNDMIXPLUGININFO, *PSNDMIXPLUGININFO; // Size should be 128

    typedef struct _SNDMIXPLUGINDATA_ECHO
    {
        float unknown;
        float mWetDryMix;
        float mFeedback;
        float mLeftDelay;
        float mRightDelay;
        float mPanDelay;
    } SNDMIXPLUGINDATA_ECHO;

    typedef struct _SNDMIXPLUGIN
    {
        ChannelGroupI       mChannelGroup;
	    void               *pMixPlugin;
	    SNDMIXPLUGININFO    Info;
    } SNDMIXPLUGIN;

    class MusicSample
    {
      public:
        
        MusicSample() : mSound(0), mMiddleC(8363), mDefaultVolume(255), mLoopStart(0), mLoopLength(0), mRelative(0) { }

        SoundI             *mSound;
        unsigned int        mMiddleC;
        unsigned char       mDefaultVolume;
        unsigned char       mDefaultPan;
        unsigned int        mLoopStart;
        unsigned int        mLoopLength;
        int                 mRelative;
        int                 mFineTune;
        unsigned int        mRawLength;
        unsigned char       mFlags;
    	unsigned char 	    mGlobalVolume;    	/* sample global volume (scalar) */
	    unsigned int 	    mSusLoopBegin;  	/* envelope sustain loop start */
	    unsigned int 	    mSusLoopEnd;    	/* envelope sustain loop length */
	    unsigned char 	    mVibSpeed;		    /* vibrato speed 0-64  */
	    unsigned char	    mVibDepth;		    /* vibrato depth 0-64  */
	    unsigned char	    mVibType;		    /* vibrato type 0=sine, 1=rampdown, 2=square, 3=random */
	    unsigned char	    mVibRate;		    /* vibrato rate 0-64 (like sweep?)  */
    };

    class MusicEnvelopeState
    {
      public:
        int                 mTick;           /* tick counter for envelope position  */
        int                 mPosition;       /* envelope position  */
        int                 mFraction;       /* fractional interpolated envelope volume  */
        int                 mValue;          /* final interpolated envelope volume  */
        int                 mDelta;          /* delta step between points  */
        bool                mStopped;        /* flag to say whether envelope has finished or not  */
    };

    /*
        Channel type - contains information on a virtual child channel 
    */
    class MusicVirtualChannel : public LinkedListNode
    {
      public:                   

        int                 mIndex;
        bool                mAllocated;
        bool                mFlip;              
        ChannelI            mChannel;           /* pointer to FSOUND system mixing channel */
        MusicSample        *mSample;            /* pointer to MusicSample object. */
        MusicSong          *mSong;              /* Pointer to MusicSong object. */
                           
        unsigned char       mLastInstrument;    /* last instrument set in channel  */
        unsigned char       mLastNote;          /* last note set in channel  */
        unsigned char       mLastSample;        /* last sample set in channel  */
                           
        bool                mBackground;        /* flag to say whether channel is a background NNA type channel or not  */
        unsigned char       mNoteControl;       /* flags for FSOUND */
                           
        unsigned char       mNNA;                /* nna type for this channel */
        unsigned char       mVolType;            /* Type of envelope,bit 0:On  */
                                                
        int                 mFrequency;         /* current mod frequency period for this channel  */
        int                 mVolume;            /* current mod volume for this channel  */
        int                 mPan;               /* current mod pan for this channel  */
        int                 mVolumeDelta;       /* delta for volume commands.. tremolo/tremor etc  */
        int                 mFrequencyDelta;    /* delta for frequency commands.. vibrato/arpeggio etc  */
        int                 mPanDelta;          /* delta for pan commands.. panbrello */
        unsigned int        mSampleOffset;      /* sample offset for this channel in SAMPLES */
        int                 mDirection;
                                                
        int                 mSampGlobalVol;    /* this current sample's global volume  */
                
        MusicEnvelopeState  mEnvVolume;
        MusicEnvelopeState  mEnvPan;
        int                 mEnvPitchTick;      /* tick counter for envelope position  */
        int                 mEnvPitchPos;       /* envelope position  */
        int                 mEnvPitchFrac;      /* fractional interpolated envelope pitch  */
        int                 mEnvPitch;          /* final interpolated envelope pitch  */
        int                 mEnvPitchDelta;     /* delta step between points  */
        bool                mEnvPitchStopped;   /* flag to say whether envelope has finished or not  */
                                                
        bool                mFade;              /* flag whether to start fade or not  */
        int                 mFadeOutVolume;     /* volume fade out  */
        int                 mIVibPos;           /* instrument vibrato position */
        int                 mIVibSweepPos;      /* instrument vibrato sweep position */
                                                
        bool                mKeyOff;            /* flag whether keyoff has been hit or not)  */
        bool                mRamping;           /* ramping out instead of stopping */
        int                 mTicksToDie; 
               
        FMOD_RESULT         cleanUp();
    };


    /*
        Multi sample extended instrument
    */
    class MusicInstrument
    {
      public:
	    signed char		mName[28];			    /* instrument name */
	    int				mNumSamples;			/* number of samples in this instrument  */
	    MusicSample     mSample[16];		    /* 16 samples per instrument  */
	    unsigned char	mKeyMap[96];			/* sample keymap assignments  */
										    
	    unsigned char	mVolumeType;			/* Type of envelope,bit 0:On 1:Sustain 2:Loop  */
	    unsigned char	mVolumeNumPoints;		/* Number of volume envelope points  */
	    unsigned short	mVolumePoints[40];		/* Volume envelope points (80 bytes - enough for 25 3 byte IT envelopes) */
	    unsigned char	mVolumeSustain;			/* Volume sustain point  */
	    unsigned char	mVolumeLoopStart;		/* Volume envelope loop start  */
	    unsigned char	mVolumeLoopEnd;			/* Volume envelope loop end  */
	    unsigned char	mVolumeSustainLoopStart;/* Volume envelope sustain loop start  */
	    unsigned char	mVolumeSustainLoopEnd;	/* Volume envelope sustain loop end  */
										    
	    unsigned char	mPanType;			    /* Type of envelope,bit 0:On 1:Sustain 2:Loop  */
	    unsigned char	mPanNumPoints;		    /* Number of panning envelope points  */
	    unsigned short	mPanPoints[40];		    /* Panning envelope points  */
	    unsigned char	mPanSustain;		    /* Panning sustain point  */
	    unsigned char	mPanLoopStart;		    /* Panning envelope loop start  */
	    unsigned char	mPanLoopEnd;		    /* Panning envelope loop end  */
	    unsigned char	mPanSustainLoopStart;	/* Panning envelope sustain loop start  */
	    unsigned char	mPanSustainLoopEnd;		/* Panning envelope sustain loop end  */
                                                
	    unsigned char	mPitchType;			    /* Type of envelope,bit 0:On 1:Sustain 2:Loop  */
	    unsigned char	mPitchNumpoints;	    /* Number of pitch envelope points  */
	    unsigned short	mPitchPoints[40];	    /* Pitch envelope points  */
	    unsigned char	mPitchSustain;		    /* Pitch sustain point  */
	    unsigned char	mPitchLoopStart;	    /* Pitch envelope loop start  */
	    unsigned char	mPitchLoopEnd;		    /* Pitch envelope loop end  */
	    unsigned char	mPitchSustainLoopStart;	/* Pitch envelope sustain loop start  */
	    unsigned char	mPitchSustainLoopEnd;	/* Pitch envelope sustain loop end  */

	    unsigned char	mVibratoType;			/* Instrument Vibrato type  */
	    unsigned char	mVibratoSweep;			/* Time it takes for vibrato to fully kick in  */
	    unsigned char	mVibratoDepth;			/* depth of vibrato  */
	    unsigned char	mVibratoRate;			/* rate of vibrato  */
										    
	    unsigned short	mVolumeFade;			/* fade out value  */

	    unsigned char	mGlobalVolume;			/* global volume (scalar)  */
	    unsigned char	mDefaultPan;			/* default pan  */
	    unsigned char	mNNA;				    /* New Note Action type  */
	    unsigned char	mDupCheckType;		    /* Duplicate Check type  */
	    unsigned char	mDupCheckAction;		/* Duplicate Check action  */
	    unsigned char	mPitchPanSep;		    /* pitch pan seperation  */
	    unsigned char	mPitchPanCenter;		/* pitch pan center  */
	    unsigned char	mVolumeVariation;		/* random volume variation  */
	    unsigned char	mPanVariation;		    /* random panning variation  */
	    unsigned char	mNoteTable[240];		/* keymap / notetable  */

        unsigned int    mFilterCutOff;
        unsigned int    mFilterResonance;

        unsigned char   mMIDIOutput;            /* MIDI Channel / FX Send. */
    };


    class MusicChannel
    {
      public:
        MusicVirtualChannel mVirtualChannelHead;        /* list of child channels for NNA's - mods/s3ms/xms have 1 child  */
        unsigned char       mInstrument;                /* last instrument set in channel  */
        unsigned char       mNote;                      /* last note set in channel  */
        unsigned char       mSample;                    /* last sample set in channel  */
        unsigned char       mRealNote;                  /* last realnote set in channel  */
        int                 mPeriod;                    /* last period set in channel  */
        unsigned char       mRecentEffect;              /* previous row's effect.. used to correct tremolo volume  */
                            
        int                 mVolume;                    /* current mod volume for this channel  */
        int                 mPan;                       /* current mod pan for this channel  */
        int                 mVolumeDelta;               /* delta for volume commands.. tremolo/tremor etc  */

        unsigned int        mSampleOffset;              /* sample offset for this channel in SAMPLES */
                                                        
        int                 mGlobalVolume;              /* global volume for this channel */
        unsigned char       mPortaUpDown;               /* last porta up or down value (S3M / IT) */
        unsigned char       mPortaDown;                 /* last porta down value (XM)  */
        unsigned char       mPortaUp;                   /* last porta up value (XM)  */
        unsigned char       mXtraPortaDown;             /* last porta down value (XM)  */
        unsigned char       mXtraPortaUp;               /* last porta up value (XM)  */
        unsigned char       mVolumeSlide;               /* last volume slide value (XM + S3M) */
        unsigned char       mPanSlide;                  /* pan slide parameter (XM + IT MODPLUG)  */
        unsigned char       mRetrigX;                   /* last retrig volume slide used (XM + S3M) */
        unsigned char       mRetrigY;                   /* last retrig tick count used (XM + S3M) */
        unsigned char       mRetrigCount;               /* retrig timer (IT) */
                            
        int                 mPortaTarget;               /* note to porta to  */
        unsigned char       mPortaSpeed;                /* porta speed */
        unsigned char       mPortaReached;              /* flag for IT to say portamento has been reached */
                            
        signed char         mVibPos;                    /* vibrato position */
        unsigned char       mVibSpeed;                  /* vibrato speed */
        unsigned char       mVibDepth;                  /* vibrato depth */
        unsigned char       mVibType;                   /* vibrato type (IT) */
                            
        signed char         mTremoloPosition;           /* tremolo position */
        unsigned char       mTremoloSpeed;              /* tremolo speed */
        unsigned char       mTremoloDepth;              /* tremolo depth */
                            
        int                 mPanbrelloPos;              /* panbrello position */
        unsigned char       mPanbrelloSpeed;            /* panbrello speed */
        unsigned char       mPanbrelloDepth;            /* panbrello depth */
                            
        unsigned char       mTremorPosition;            /* tremor position (XM + S3M) */
        unsigned char       mTremorOn;                  /* remembered parameters for tremor (XM + S3M)  */
        unsigned char       mTremorOff;                 /* remembered parameters for tremor (XM + S3M)  */
        unsigned char       mArpeggio;                  /* remembered parameters for arpeggio (S3M)  */
        int                 mPatternLoopRow;            
        int                 mPatternLoopNumber;         /* pattern loop variables for effect  E6x */
        unsigned char       mChannelVolumeSlide;        /* global volume slide parameters */
                            
        unsigned char       mSpecialParam;              /* remembered parameter for Sxy */
        unsigned char       mWaveControl;               /* waveform type for vibrato and tremolo (4bits each) */
        unsigned char       mWaveControlVibrato;        /* waveform type for vibrato (IT)  */
        unsigned char       mWaveControlTremolo;        /* waveform type for tremolo (IT) */
        unsigned char       mWaveControlPan;            /* waveform type for panbrello (IT) */
                            
        unsigned char       mFineVolumeSlideDown;       /* parameter for fine volume slide down  */
        unsigned char       mFineVolumeSlideUp;         /* parameter for fine volume slide up */
        unsigned char       mFinePortaUp;               /* parameter for fine porta slide up */
        unsigned char       mFinePortaDown;             /* parameter for fine porta slide down  */
        unsigned char       mHighOffset;                /* high part of sample offset - ie the 'y' part of yxx00 (IT)  */
        unsigned char       mVolumeColumnVolumeSlide;   /* volume column parameter remembered for volume sliding (IT)  */
    };

    
    /*
        Single note type - contains info on 1 note in a pattern
    */
    class MusicNote
    {
      public:
        unsigned char    mNote;                          /* note to play at     (0-133) 132=none,133=keyoff */
        unsigned char    mNumber;                        /* sample being played (0-99) */
        unsigned char    mVolume;                        /* volume column value (0-64)  255=no volume */
        unsigned char    mEffect;                        /* effect number       (0-1Ah) */
        unsigned char    mEffectParam;                   /* effect parameter    (0-255) */
    };

    /*
        Pattern data type
    */
    class MusicPattern
    {
      public:
        int         mRows;
        MusicNote  *mData;
    };

    const int MUSIC_MAXORDERS = 256;
    const int MUSIC_MAXCHANNELS = 64;

    /* 
        Song type - contains info on song
    */
    class MusicSong : public Codec
    {
      private:
        FMOD_RESULT getLengthInternal(unsigned int *length, FMOD_TIMEUNIT lengthtype);
        FMOD_RESULT getPositionInternal(unsigned int *position, FMOD_TIMEUNIT postype);

      public:
        char                 mSongName[FMOD_STRING_MAXNAMELEN];
        MusicPattern        *mPattern;                      /* patterns array for this song */
        DSPI                *mDSPHead;                      /* DSP unit channels usually mix to by default */
        bool                *mVisited;                      /* 256 = maximum rows per pattern */
        unsigned char	     mOrderList[MUSIC_MAXORDERS];   /* pattern playing order list */

        /*
            Channel stuff.
        */
        int                  mNumChannels;          /* number of channels */
        MusicChannel        *mMusicChannel[MUSIC_MAXCHANNELS];    /* Array of music channels.  One for each track in a mod */
        int                  mNumVirtualChannels;
        MusicVirtualChannel *mVirtualChannel;       /* Array of virtual music channels.  One for each track in a mod/s3m/xm, or possibly multiple for each track in a .IT file due to NNA */
        ChannelPool         *mChannelPool;          /* ChannelPool */
        ChannelSoftware     *mChannelSoftware;      /* Array of FMOD Ex low level real channels.  Each one of these lives in a ChannelI channel.  */
        DSPI               **mLowPass;              /* Array of lowpass filters to be used on .IT files */
        ChannelGroupI        mChannelGroup;

        /*
            Mixing stuff
        */
        int             mMixerSamplesLeft;
        int             mMixerSamplesPerTick;
        unsigned int    mPCMOffset;

        /* 
            Defaults 
        */
        int             mDefaultSpeed;
        unsigned int    mDefaultBPM;
        unsigned char   mDefaultPan[64];
        unsigned char   mDefaultVolume[64];
        unsigned char   mDefaultGlobalVolume;

        int             mNumOrders;                     /* number of orders (song length) */
        int             mNumPatterns;                   /* number of physical patterns */
        int             mNumPatternsMem;                /* number of allocated patterns */
        int             mNumInstruments;                /* number of instruments */
        int             mNumSamples;                    /* number of samples */

        MusicInstrument *mInstrument;
        signed char    *mPatternPtr;                    /* current offset into compressed pattern data */
        unsigned char   mLastNote[64];
        unsigned char   mLastNumber[64];
        unsigned char   mLastVolume[64];
        unsigned char   mLastEffect[64];
        unsigned char   mLastEffectParam[64];
        unsigned char   mPreviousMaskVariable[64];
        MusicNote       mNote[64];

        int             mRestart;
        float           mMasterSpeed;
        float           mPanSeparation;                 /* master pan seperation */
        int             mMasterVolume;                  
        int             mGlobalVolume;                  /* global mod volume */
        unsigned char   mGlobalVolumeSlide;             /* global mod volume */
        unsigned short  mMusicFlags;                    /* flags such as linear frequency, format specific quirks etc */
        bool            mPlaying;                       /* song is playing flag */
        bool            mFinished;                      /* song has reached the end     */
        bool            mLooping;                       /* flag to say whether song should loop or not */
        int             mTick;                          /* current mod tick     */
        int             mSpeed;                         /* speed of song in ticks per row */
        int             mBPM;                           /* speed of song in bpm */
        int             mRow;                           /* current row in pattern */
        int             mOrder;                         /* current song order position */
        int             mPatternDelay;                  /* pattern delay counter */
        int             mPatternDelayTicks;             /* pattern delay ticks counter */
        int             mNextRow;                       /* current row in pattern */
        int             mNextOrder;                     /* current song order position */

        inline int      period2HZ(int _per) { return (14317056 / (_per)); }

        FMOD_RESULT     cleanUpVirtualChannel(MusicVirtualChannel *vcptr, MusicVirtualChannel **newvcptr);
        FMOD_RESULT     spawnNewVirtualChannel(MusicChannel *cptr, MusicSample *sptr, MusicVirtualChannel **newvcptr);

        FMOD_RESULT     setBPM(int bpm);
        FMOD_RESULT     play();
        FMOD_RESULT     stop();
        FMOD_RESULT     playSound(MusicSample *sample, MusicVirtualChannel *vcptr, bool addfilter, SNDMIXPLUGIN *plugin = 0);
        FMOD_RESULT     fineTune2Hz(unsigned char ft, unsigned int *hz);

        static FMOD_RESULT F_CALLBACK getLengthCallback(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
        static FMOD_RESULT F_CALLBACK getPositionCallback(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);
    };

    extern unsigned char        gSineTable[];
    extern signed char          gFineSineTable[];
    extern unsigned int         gPeriodTable[];
    extern int                  gITLogPeriodTable[];
#if defined(FMOD_NO_FPU) && defined(FMOD_SUPPORT_IT)
    extern signed short         gITFineTuneTable[];
#endif
    extern MusicSample          gDummySample;
    extern MusicVirtualChannel  gDummyVirtualChannel;
    extern MusicInstrument      gDummyInstrument;
}

#endif

#endif

