#ifndef _FMOD_CHANNEL_REAL_H
#define _FMOD_CHANNEL_REAL_H

#include "fmod_settings.h"

#include "fmod.h"
#include "fmod_channeli.h"
#include "fmod_types.h"

namespace FMOD
{
    class  ChannelPool;
    class  DSPI;
    class  Output;
    class  SoundI;
    class  SystemI;
    struct ChannelRealData;
    class  ReverbI;

    typedef enum
    {
        CHANNELREAL_TYPE_HW3D,
        CHANNELREAL_TYPE_HW2D,
        CHANNELREAL_TYPE_SW,
        CHANNELREAL_TYPE_MAX
    } CHANNELREAL_TYPE;

    #define CHANNELREAL_TYPE_HW CHANNELREAL_TYPE_HW3D

    typedef unsigned int CHANNELREAL_FLAG;

    #define CHANNELREAL_FLAG_ALLOCATED          0x00000010
    #define CHANNELREAL_FLAG_PAUSED             0x00000020
    #define CHANNELREAL_FLAG_PLAYING            0x00000040
    #define CHANNELREAL_FLAG_STOPPED            0x00000080
    #define CHANNELREAL_FLAG_IN_USE             0x00000100
    #define CHANNELREAL_FLAG_PAUSEDFORSETPOS    0x00000200
    #define CHANNELREAL_FLAG_ENDDELAY           0x00000400
    #define CHANNELREAL_FLAG_NOREVERB           0x00000800
    #define CHANNELREAL_FLAG_RESERVED           0x00001000
    #define CHANNELREAL_FLAG_HASPLAYED          0x00002000
    #define CHANNELREAL_FLAG_MAX                0xFFFFFFFF

    typedef enum
    {
        CHANNELREAL_PLAYDIR_FORWARDS,
        CHANNELREAL_PLAYDIR_BACKWARDS,
        CHANNELREAL_PLAYDIR_MAX = 0xFFFFFFFF
    } CHANNELREAL_PLAYDIR;

    /*
        *************************************************************************************
        DON'T IFDEF STUFF OUT THAT MIGHT BE INCLUDED IN FMOD.DLL BUT NOT IN A PLUGIN DLL!
        (staticforplugins vs fmod.dll code sharing conflict)
        *************************************************************************************
    */    
    class ChannelReal
    {
        friend class ChannelPool;
        friend class SystemI;
        friend class ChannelI;
        friend class ChannelStream;
        friend class MusicSong;
        friend class Stream;
        friend class SoundI;
        friend class DSPWaveTable;
        friend class DSPResampler;
        friend class DSPCodec;
        friend class OutputOpenAL;
        friend class AsyncThread;

      protected:

        SystemI                *mSystem;
        int                     mNumRealChannels;
        ChannelReal            *mRealChannel[FMOD_CHANNEL_MAXREALSUBCHANNELS];
        int                     mSubChannelIndex;
        ChannelPool            *mPool;
        ChannelI               *mParent;
        Output                 *mOutput;
        SoundI                 *mSound;
        DSPI                   *mDSP;
        FMOD_MODE               mMode;
        CHANNELREAL_FLAG        mFlags;
        int                     mIndex;
        unsigned int            mPosition;
        int                     mDirection;
        int                     mLoopCount;
        unsigned int            mLoopStart;
        unsigned int            mLoopLength;
        unsigned int            mLength;
        unsigned int            mDelayStart;
        unsigned int            mDelayEnd;
        float                   mMaxFrequency;
        float                   mMinFrequency;

      public:

        ChannelReal();

        virtual FMOD_RESULT  init                 (int index, SystemI *system, Output *output, DSPI *dspmixtarget);
        virtual FMOD_RESULT  set2DFreqVolumePanFor3D();
        virtual bool         isStream             () { return false; }
        virtual FMOD_RESULT  moveChannelGroup     (ChannelGroupI *oldchannelgroup, ChannelGroupI *newchannelgroup);
// VERIFY: 16d01662 auto
        // OFFSET: 0x0093BE70
        virtual ~ChannelReal() { }
        virtual FMOD_RESULT  close                ();
        virtual FMOD_RESULT  alloc                ();
        virtual FMOD_RESULT  alloc                (DSPI *dsp);
        virtual FMOD_RESULT  start                ();
        virtual FMOD_RESULT  update               (int delta);
        virtual FMOD_RESULT  updateStream         ();
        virtual FMOD_RESULT  stop                 (bool ignoreenddelay, bool resetstate);
        virtual FMOD_RESULT  stop                 (bool ignoreenddelay, bool resetstate, bool force);
        virtual FMOD_RESULT  setPaused            (bool paused);
        virtual FMOD_RESULT  getPaused            (bool *paused);
        virtual FMOD_RESULT  setVolume            (float volume);
        virtual FMOD_RESULT  setFrequency         (float frequency);
        virtual FMOD_RESULT  setPan               (float pan, float fbpan = 1);
        virtual FMOD_RESULT  setDelay             (unsigned int startdelay, unsigned int enddelay);
        virtual FMOD_RESULT  setSpeakerMix        (float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright);
        virtual FMOD_RESULT  setSpeakerLevels     (int speaker, float *levels, int numlevels);
        virtual FMOD_RESULT  updateSpeakerLevels  (float volume);
        virtual FMOD_RESULT  setPosition          (unsigned int position, FMOD_TIMEUNIT postype);
        virtual FMOD_RESULT  getPosition          (unsigned int *position, FMOD_TIMEUNIT postype);
        virtual FMOD_RESULT  setLoopPoints        (unsigned int loopstart, unsigned int looplength);
        virtual FMOD_RESULT  setLoopCount         (int loopcount);
        virtual FMOD_RESULT  set3DAttributes      ();
        virtual FMOD_RESULT  set3DMinMaxDistance  ();
        virtual FMOD_RESULT  set3DOcclusion       (float directOcclusion, float reverbOcclusion);
        virtual FMOD_RESULT  setReverbProperties  (const FMOD_REVERB_CHANNELPROPERTIES *prop);
        virtual FMOD_RESULT  getReverbProperties  (FMOD_REVERB_CHANNELPROPERTIES *prop);
        virtual FMOD_RESULT  isPlaying            (bool *isplaying, bool includethreadlatency = false);
        virtual FMOD_RESULT  isVirtual            (bool *isvirtual);
        virtual FMOD_RESULT  getSpectrum          (float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype);
        virtual FMOD_RESULT  getWaveData          (float *wavearray, int numvalues, int channeloffset);
        virtual FMOD_RESULT  getDSPHead           (DSPI **dsp);
        virtual FMOD_RESULT  setMode              (FMOD_MODE mode);

        FMOD_RESULT setReserved(bool reserved) 
        { 
            if (mFlags & (CHANNELREAL_FLAG_ALLOCATED | CHANNELREAL_FLAG_IN_USE)) return FMOD_ERR_CHANNEL_ALLOC; 
            if (reserved) mFlags |= CHANNELREAL_FLAG_RESERVED; else mFlags &= ~CHANNELREAL_FLAG_RESERVED; 
            return FMOD_OK; 
        }
        FMOD_RESULT allowReverb(bool allow) { if (allow) mFlags &= ~CHANNELREAL_FLAG_NOREVERB; else mFlags |= CHANNELREAL_FLAG_NOREVERB; return FMOD_OK; }
        FMOD_RESULT hasPlayed(bool *hasplayed) { if (hasplayed) *hasplayed = ((mFlags & CHANNELREAL_FLAG_HASPLAYED) ? true : false); return FMOD_OK; }
    };
}

#endif


