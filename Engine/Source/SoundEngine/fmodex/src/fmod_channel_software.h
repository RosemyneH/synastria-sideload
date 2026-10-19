#ifndef _FMOD_CHANNEL_SOFTWARE_H
#define _FMOD_CHANNEL_SOFTWARE_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_SOFTWARE

#include "fmod_channel_realmanual3d.h"
#include "fmod_dsp_filter.h"
#include "fmod_dsp_wavetable.h"

namespace FMOD
{
    class DSPCodec;
    class ReverbI;
    
    class ChannelSoftware : public ChannelRealManual3D
    {
        friend class DSPCodecPool;
        friend class OutputSoftware;

      protected:

        /*  __________________           __________         _______________  */
        /* |                  |<---..   |          |       |               | */
        /* | channelgroup DSP |<--------| mDSPHead |<------| mDSPWaveTable | */
        /* |__________________|<---..   |__________|       |_______________| */

        DSPI           *mDSPHead;
        DSPFilter       mDSPHeadMemory;
        DSPWaveTable   *mDSPWaveTable;
        DSPWaveTable    mDSPWaveTableMemory;
        DSPResampler   *mDSPResampler;     /* mDSPResampler is connected between mDSPHead and mDSPWaveTable sometimes.  */
        DSPI           *mDSPLowPass;       /* Optional if the user specifies FMOD_INIT_OCCLUSION_LOWPASS. */
        DSPI           *mDSPReverb;        /* Points to the unit the reverb unit is connected to. */
        DSPCodec       *mDSPCodec;
        DSPCodec       *mDSPCodecStopped;
        DSPConnectionI *mDSPConnection;

      public:

        ChannelSoftware();

        FMOD_RESULT  init                 (int index, SystemI *system, Output *output, DSPI *dspmixtarget);
        FMOD_RESULT  close                ();
        FMOD_RESULT  alloc                ();
        FMOD_RESULT  alloc                (DSPI *dsp);
        FMOD_RESULT  start                ();

        FMOD_RESULT  stop                 (bool ignoreenddelay, bool resetstate);
        FMOD_RESULT  stop                 (bool ignoreenddelay, bool resetstate, bool force);
        FMOD_RESULT  setVolume            (float volume);
        FMOD_RESULT  setFrequency         (float frequency);
        FMOD_RESULT  setPan               (float pan, float fbpan = 1);
        FMOD_RESULT  setSpeakerMix        (float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright);
        FMOD_RESULT  setSpeakerLevels     (int speaker, float *levels, int numlevels);
        FMOD_RESULT  setPaused            (bool paused);
        FMOD_RESULT  setPosition          (unsigned int position, FMOD_TIMEUNIT postype);
        FMOD_RESULT  getPosition          (unsigned int *position, FMOD_TIMEUNIT postype);
        FMOD_RESULT  setLoopPoints        (unsigned int loopstart, unsigned int looplength);
        FMOD_RESULT  setLoopCount         (int loopcount);
        FMOD_RESULT  setMode              (FMOD_MODE mode);      
        FMOD_RESULT  set3DOcclusion       (float directOcclusion, float reverbOcclusion);
        FMOD_RESULT  isPlaying            (bool *isplaying, bool includethreadlatency = false);
        FMOD_RESULT  getSpectrum          (float *spectrumarray, int numentries, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype);
        FMOD_RESULT  getWaveData          (float *wavearray, int numentries, int channeloffset);
        FMOD_RESULT  getDSPHead           (DSPI **dsp);
        FMOD_RESULT  setReverbProperties  (const FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT  getReverbProperties  (FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT  updateDirectMix      (float volume);
        FMOD_RESULT  updateReverbMix      (ReverbI* reverb, float volume);
        FMOD_RESULT  addToReverbs         (DSPI *dsp);
        FMOD_RESULT  moveChannelGroup     (ChannelGroupI *oldchannelgroup, ChannelGroupI *newchannelgroup);
        FMOD_RESULT  setupDSPCodec        (DSPI *dsp);
    };
}

#endif

#endif
