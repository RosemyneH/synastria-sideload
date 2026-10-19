#ifndef _FMOD_CODEC_MIDI_H
#define _FMOD_CODEC_MIDI_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_MIDI

#include "fmod_music.h"

namespace FMOD
{
    class DSPI;
    class ChannelPool;
    class CodecMIDI;
    class CodecMIDITrack;
    class CodecMIDIChannel;

    #ifdef FMOD_SUPPORT_PRAGMAPACK
	    #pragma pack(1)
    #endif

    typedef struct 
    {
       char           mID[4]   FMOD_PACKED_INTERNAL;
       unsigned int   mSize    FMOD_PACKED_INTERNAL; 
    } FMOD_PACKED MIDI_CHUNK;

    typedef struct
    {
       MIDI_CHUNK     mChunk      FMOD_PACKED_INTERNAL;
       unsigned short mFormat     FMOD_PACKED_INTERNAL;
       unsigned short mNumTracks  FMOD_PACKED_INTERNAL;
       unsigned short mDivision   FMOD_PACKED_INTERNAL;
    } FMOD_PACKED MTHD_CHUNK;

    #ifdef FMOD_SUPPORT_PRAGMAPACK
        #ifdef 	CODEWARRIOR
        #pragma pack(0)
        #else
        #pragma pack()
        #endif
    #endif

    typedef enum
    {
        MIDI_FORMAT_SINGLETRACK,
        MIDI_FORMAT_MULTITRACK,
        MIDI_FORMAT_PATTERN,
    } MIDI_FORMAT;

    static const unsigned char MIDI_VOICE_NOTEOFF           = 0x80;
    static const unsigned char MIDI_VOICE_NOTEON            = 0x90;
    static const unsigned char MIDI_VOICE_AFTERTOUCH        = 0xA0;
    static const unsigned char MIDI_VOICE_CONTROLLERCHANGE  = 0xB0;
    static const unsigned char MIDI_VOICE_PROGRAMCHANGE     = 0xC0;
    static const unsigned char MIDI_VOICE_CHANNELKEYPRESSURE= 0xD0;
    static const unsigned char MIDI_VOICE_PITCHBEND         = 0xE0;
    static const unsigned char MIDI_SYSEXESCAPE             = 0xF7;
    static const unsigned char MIDI_SYSEXEVENT              = 0xF0;
    static const unsigned char MIDI_METAEVENT               = 0xFF;

    static const unsigned char MIDI_METAEVENT_SEQNUMBER     = 0x00;
    static const unsigned char MIDI_METAEVENT_TEXT          = 0x01;
    static const unsigned char MIDI_METAEVENT_COPYRIGHT     = 0x02;
    static const unsigned char MIDI_METAEVENT_NAME          = 0x03;
    static const unsigned char MIDI_METAEVENT_INSTRUMENT    = 0x04;
    static const unsigned char MIDI_METAEVENT_LYRIC         = 0x05;
    static const unsigned char MIDI_METAEVENT_MARKER        = 0x06;
    static const unsigned char MIDI_METAEVENT_CUEPOINT      = 0x07;
    static const unsigned char MIDI_METAEVENT_PATCHNAME     = 0x08;
    static const unsigned char MIDI_METAEVENT_PORTNAME      = 0x09;
    static const unsigned char MIDI_METAEVENT_CHANNEL       = 0x20;
    static const unsigned char MIDI_METAEVENT_PORT          = 0x21;
    static const unsigned char MIDI_METAEVENT_ENDOFTRACK    = 0x2F;
    static const unsigned char MIDI_METAEVENT_TEMPO         = 0x51;
    static const unsigned char MIDI_METAEVENT_SMTPOFFSET    = 0x54;
    static const unsigned char MIDI_METAEVENT_TIMESIGNATURE = 0x58;
    static const unsigned char MIDI_METAEVENT_KEYSIGNATURE  = 0x59;
    static const unsigned char MIDI_METAEVENT_PROPRIETARY   = 0x7F;

    static const unsigned char MIDI_CONTROLLERCHANGE_BANKSELECT_MSB                =  0; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_MODULATIONWHEEL_MSB           =  1; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_BREATHCONTROL_MSB             =  2; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_FOOTCONTROLLER_MSB            =  4; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_PORTAMENTOTIME_MSB            =  5; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_DATAENTRY_MSB                 =  6; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_CHANNELVOLUME_MSB             =  7; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_BALANCE_MSB                   =  8; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_PAN_MSB                       = 10; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_EXPRESSIONCONTROLLER_MSB      = 11; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_EFFECTCONTROL1_MSB            = 12; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_EFFECTCONTROL2_MSB            = 13; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER1_MSB = 16; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER2_MSB = 17; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER3_MSB = 18; // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER4_MSB = 19; // 0-127 MSB 

    static const unsigned char MIDI_CONTROLLERCHANGE_BANKSELECT_LSB                = 32; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_MODULATIONWHEEL_LSB           = 33; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_BREATHCONTROL_LSB             = 34; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_FOOTCONTROLLER_LSB            = 36; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_PORTAMENTOTIME_LSB            = 37; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_DATAENTRY_LSB                 = 38; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_CHANNELVOLUME_LSB             = 39; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_BALANCE_LSB                   = 40; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_PAN_LSB                       = 42; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_EXPRESSIONCONTROLLER_LSB      = 43; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_EFFECTCONTROL1_LSB            = 44; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_EFFECTCONTROL2_LSB            = 45; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER1_LSB = 48; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER2_LSB = 49; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER3_LSB = 50; // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER4_LSB = 51; // 0-127 LSB 

    static const unsigned char MIDI_CONTROLLERCHANGE_DAMPERPEDALONOFF              = 64;   // >64=ON 
    static const unsigned char MIDI_CONTROLLERCHANGE_PORTAMENTOONOFF               = 65;   // >64=ON 
    static const unsigned char MIDI_CONTROLLERCHANGE_SUSTENUTOONOFF                = 66;   // >64=ON 
    static const unsigned char MIDI_CONTROLLERCHANGE_SOFTPEDALONOFF                = 67;   // >64=ON 
    static const unsigned char MIDI_CONTROLLERCHANGE_LEGATOFOOTSWITCH              = 68;   // >64=ON 
    static const unsigned char MIDI_CONTROLLERCHANGE_HOLD2                         = 69;   // >64=ON 
    static const unsigned char MIDI_CONTROLLERCHANGE_SOUNDCONTROLLER1              = 70;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_SOUNDCONTROLLER2              = 71;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_SOUNDCONTROLLER3              = 72;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_SOUNDCONTROLLER4              = 73;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_SOUNDCONTROLLER5              = 74;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_SOUNDCONTROLLER6              = 75;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_SOUNDCONTROLLER7              = 76;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_SOUNDCONTROLLER8              = 77;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_SOUNDCONTROLLER9              = 78;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_SOUNDCONTROLLER10             = 79;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER5     = 80;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER6     = 81;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER7     = 82;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_GENERALPURPOSECONTROLLER8     = 83;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_PORTAMENTOCONTROL             = 84;   // -127 SOURCE NOTE 
    static const unsigned char MIDI_CONTROLLERCHANGE_EFFECTS1DEPTH                 = 91;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_EFFECTS2DEPTH                 = 92;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_EFFECTS3DEPTH                 = 93;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_EFFECTS4DEPTH                 = 94;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_EFFECTS5DEPTH                 = 95;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_DATAENTRYINC                  = 96;   // N/A   
    static const unsigned char MIDI_CONTROLLERCHANGE_DATAENTRYDEC                  = 97;   // N/A   
    static const unsigned char MIDI_CONTROLLERCHANGE_NONREGISTEREDPARAMNUMBER_LSB  = 98;   // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_NONREGISTEREDPARAMNUMBER_MSB  = 99;   // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_REGISTEREDPARAMETERNUMBER_LSB = 100;  // 0-127 LSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_REGISTEREDPARAMETERNUMBER_MSB = 101;  // 0-127 MSB 
    static const unsigned char MIDI_CONTROLLERCHANGE_ALLSOUNDOFF                   = 120;
    static const unsigned char MIDI_CONTROLLERCHANGE_RESETALLCONTROLLERS           = 121;
    static const unsigned char MIDI_CONTROLLERCHANGE_LOCALCONTROLONOFF             = 122;   
    static const unsigned char MIDI_CONTROLLERCHANGE_ALLNOTESOFF                   = 123;  
    static const unsigned char MIDI_CONTROLLERCHANGE_OMNIMODEOFF                   = 124; 
    static const unsigned char MIDI_CONTROLLERCHANGE_OMNIMODEON                    = 125;
    static const unsigned char MIDI_CONTROLLERCHANGE_POLYMODEONOFF                 = 126;
    static const unsigned char MIDI_CONTROLLERCHANGE_POLYMODEON                    = 127;

    static const unsigned char MIDI_CONTROLLERCHANGE_RPN_PITCHBENDSENSITIVITY      = 0;

    static const unsigned int MIDI_F_INSTRUMENT_DRUMS = 0x80000000;

    static const int MIDI_DEFAULTSUBCHANNELS        = 32;
    static const int MIDI_MINIMUMSAMPLEGRANULARITY  = 512;      // 10.66ms @ 48khz :S

    class CodecMIDISubChannel : public LinkedListNode
    {
      public:

        CodecMIDI           *mMIDI;
        ChannelI             mChannel;
        CodecDLSInstrument  *mInstrument;
                            
        float                mLFOStartDelay;
        float                mLFOTime;
        float                mLFOFrequency;
        CodecDLSEnvelope     mVolumeEnvelope;
        CodecDLSEnvelope     mPitchEnvelope;
                            
        int                  mCurrentNote;       
        SoundI              *mSound;
        unsigned char        mKeyOnKey;
        unsigned char        mUnityNote;
        int                  mFineTune;
        unsigned char        mKeyOnVelocity;
        int                  mNumArticulators;
        DLS_CONNECTIONBLOCK *mArticulator;
                            
        float                mTremoloScale;
        float                mVibratoScale;
        float                mPanbrelloScale;
                            
        bool                 mKeyOff;
        int                  mMiddleC;
        int                  mKeyGroup;
        float                mSampleAttenuation;
        CodecMIDIChannel    *mParent;
                            
        FMOD_RESULT          displayArticulators();
        FMOD_RESULT          findArticulator(int src, int dest);
        FMOD_RESULT          articulateDest(CONN_SRC_FLAGS srcflags, int dest, int *usDestination);
        FMOD_RESULT          setUpArticulators();
        FMOD_RESULT          stop();
        FMOD_RESULT          updateVolume();
        FMOD_RESULT          updatePitch();
        FMOD_RESULT          updatePan();

        float                getTimeCentsFromlScale(int lScale);
    };

    class CodecMIDIChannel : public LinkedListNode
    {
      public:
        CodecMIDITrack     *mTrack;
        CodecMIDISubChannel mChannelHead;
        unsigned char       mIndex;
        unsigned char       mKeyOffVelocity;
        unsigned char       mKeyOnVelocity;
        unsigned char       mAfterTouchKey;
        unsigned char       mAfterTouchPressure;
        unsigned int        mBank;
        unsigned char       mProgram;
        unsigned int        mRPN;
        unsigned char       mChannelPressure;
        int                 mPitchBend;
        int                 mPitchBendSensitivity;
        bool                mDamperPedal;

        int                 mModWheel;          // cc1
        int                 mVolume;            // cc7
        int                 mPan;               // cc10
        int                 mExpression;        // cc11
        
        FMOD_RESULT         getSound(int key, SoundI **sound, CodecDLSInstrument **instrument, int *unitynote, int *finetune, int *attenuation, bool *duplicateallowed, int *keygroup, int *numarticulators, DLS_CONNECTIONBLOCK **articulators);
        FMOD_RESULT         process(unsigned char event, bool reuse, unsigned char runningdata, bool calledfromopen);
        FMOD_RESULT         update();
    };

    class CodecMIDITrack
    {
      public:
        
        CodecMIDI          *mMIDI;

        unsigned char      *mData;
        unsigned int        mOffset;
        unsigned int        mLength;
        int                 mIndex;
        bool                mReadDelta;
        float               mTick;
        bool                mFinished;
        unsigned char       mPort;
        unsigned char       mEvent;
                                                                                  
        FMOD_RESULT         readVarLen(unsigned int *result);
        FMOD_RESULT         readByte(unsigned char *result);
        FMOD_RESULT         readWord(unsigned short *result);
        FMOD_RESULT         readDWord(unsigned int *result);
        FMOD_RESULT         read(void *buff, int length);
        FMOD_RESULT         addTag(const char *name, int length, bool calledfromopen);
        FMOD_RESULT         process(bool calledfromopen);
    };


    class CodecMIDI : public Codec
    {
        friend class CodecMIDIChannel;
        friend class CodecMIDITrack;

      private:

        DSPI                *mDSPHead;
        ChannelPool         *mChannelPool;
        ChannelSoftware     *mChannelSoftware;      /* Array of FMOD Ex low level real channels.  Each one of these lives in a ChannelI channel.  */
        int                  mNumSubChannels;
        ChannelGroupI        mChannelGroup;
        CodecMIDISubChannel *mMIDISubChannel;
        CodecMIDIChannel     mMIDIChannel[16];
                           
        bool                mFinished;
                           
        int                 mSequenceNumber;
        unsigned int        mMixerSamplesLeft;
        unsigned int        mMixerSamplesPerTick;
        float               mTimingScale;
        unsigned int        mPCMOffset;
                           
        int                 mNumTracks;
        MIDI_FORMAT         mMIDIFormat;
        int                 mDivision;
        unsigned char       mSMTPOffsetHours;
        unsigned char       mSMTPOffsetMinutes;
        unsigned char       mSMTPOffsetSeconds;
        unsigned char       mSMTPOffsetFrames;
        unsigned char       mSMTPOffsetSubFrames;
                           
        unsigned char       mTimeSignatureNumerator;
        unsigned char       mTimeSignatureDenominator;
        unsigned char       mTimeSignatureMetronome;
        unsigned char       mTimeSignatureNotated32nds;
                           
        unsigned char       mKeySignatureSF;
        unsigned char       mKeySignatureMI;

        CodecMIDITrack     *mTrack;
        unsigned int        mTempo;
        float               mTick;
        float               mMillisecondsPerTick;
                           
        SoundI             *mDLSSound;
        CodecDLS           *mDLS;
        bool               *mSampleInclusionList;
                           
        FMOD_RESULT         play();
        FMOD_RESULT         readVarLen(unsigned int *length);      
        FMOD_RESULT         update(bool audible);
                           
        FMOD_RESULT         openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        FMOD_RESULT         closeInternal();
        FMOD_RESULT         readInternal(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT         setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype);
        
      public:

        CodecMIDISubChannel mChannelFreeListHead;

        static FMOD_RESULT F_CALLBACK openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
        static FMOD_RESULT F_CALLBACK closeCallback(FMOD_CODEC_STATE *codec);
        static FMOD_RESULT F_CALLBACK readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread);
        static FMOD_RESULT F_CALLBACK setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

        static FMOD_CODEC_DESCRIPTION_EX *getDescriptionEx();  
        
    };
}

#endif  /* FMOD_SUPPORT_MIDI */

#endif

