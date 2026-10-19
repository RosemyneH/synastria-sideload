#ifndef _FMOD_CHANNEL_STREAM_H
#define _FMOD_CHANNEL_STREAM_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_STREAMING

#include "fmod_channel_real.h"

namespace FMOD
{   
    class ChannelStream : public ChannelReal, public LinkedListNode
    {
      public:

        volatile bool   mFinished;
        unsigned int    mLastPCM;
        unsigned int    mDecodeOffset;
        unsigned int    mSamplesPlayed;
        unsigned int    mSamplesWritten;
        LinkedListNode  mStreamNode;

      public:

        ChannelStream();

        FMOD_RESULT setRealChannel(ChannelReal *realchan);
        bool        isStream() { return true; }

        FMOD_RESULT  set2DFreqVolumePanFor3D();
        FMOD_RESULT  moveChannelGroup    (ChannelGroupI *oldchannelgroup, ChannelGroupI *newchannelgroup);
        FMOD_RESULT  alloc               ();
        FMOD_RESULT  start               ();
        FMOD_RESULT  update              (int delta);
        FMOD_RESULT  updateStream        ();
        FMOD_RESULT  setMode             (FMOD_MODE mode);
        FMOD_RESULT  stop                (bool ignoreenddelay, bool resetstate);
        FMOD_RESULT  stop                (bool ignoreenddelay, bool resetstate, bool force);
        FMOD_RESULT  setPaused           (bool paused);
        FMOD_RESULT  setVolume           (float volume);
        FMOD_RESULT  setFrequency        (float frequency);
        FMOD_RESULT  setPan              (float pan, float fbpan = 1);
        FMOD_RESULT  setDelay            (unsigned int startdelay, unsigned int enddelay);
        FMOD_RESULT  setSpeakerMix       (float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright);
        FMOD_RESULT  setSpeakerLevels    (int speaker, float *levels, int numlevels);
        FMOD_RESULT  setPosition         (unsigned int position, FMOD_TIMEUNIT postype);
        FMOD_RESULT  getPosition         (unsigned int *position, FMOD_TIMEUNIT postype);
        FMOD_RESULT  setLoopPoints       (unsigned int loopstart, unsigned int looplength);
        FMOD_RESULT  setLoopCount        (int loopcount);
        FMOD_RESULT  set3DAttributes     ();
        FMOD_RESULT  set3DMinMaxDistance ();
        FMOD_RESULT  set3DOcclusion      (float directocclusion, float reverbocclusion);
        FMOD_RESULT  setReverbProperties (const FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT  getReverbProperties (FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT  isPlaying           (bool *isplaying, bool includethreadlatency = false);
        FMOD_RESULT  getSpectrum         (float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype);
        FMOD_RESULT  getWaveData         (float *wavearray, int numvalues, int channeloffset);
        FMOD_RESULT  getDSPHead          (DSPI **dsp);
    };
}

#endif

#endif

