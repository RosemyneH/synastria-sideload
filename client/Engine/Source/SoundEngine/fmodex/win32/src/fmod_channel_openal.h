#ifndef _FMOD_CHANNEL_OPENAL_H
#define _FMOD_CHANNEL_OPENAL_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OPENAL

#include "fmod_channel_real.h"
#include "fmod_output_openal.h"
#include "fmod_dsp_wavetable.h"

namespace FMOD
{
    class ChannelOpenAL : public ChannelReal
    {
		friend class OutputOpenAL;

	  protected:

        DSPI          *mDSPHead;
        DSPFilter      mDSPHeadMemory;
        DSPWaveTable  *mDSPWaveTable;
        DSPWaveTable   mDSPWaveTableMemory;
        DSPResampler  *mDSPResampler;       // Connected between mDSPHead and mDSPWaveTable sometimes
        DSPI          *mDSPLowPass;         // Optional if the user specifies FMOD_INIT_SOFTWARE_OCCLUSION
        DSPCodec      *mDSPCodec;
        DSPConnectionI *mDSPConnection;

		OutputOpenAL  *mOutputOAL;
		int            mNumSources;
		SourceOpenAL  *mSources[16];
		short		  *mBuffer;
		float		  *mTempBuffer;
        bool           mInitialFill;
      
      public:
        
        ChannelOpenAL();

        FMOD_RESULT  init                 (int index, SystemI *system, Output *output, DSPI *dspmixtarget);
        FMOD_RESULT  close                ();
        FMOD_RESULT  alloc                ();
        FMOD_RESULT  alloc                (DSPI *dsp);
        FMOD_RESULT  start                ();
        FMOD_RESULT  stop                 (bool ignoreenddelay, bool resetstate, bool force);
        FMOD_RESULT  setVolume            (float volume);
        FMOD_RESULT  setFrequency         (float frequency);
        FMOD_RESULT  setPan               (float pan, float fbpan = 1);
        FMOD_RESULT  setSpeakerLevels     (int speaker, float *levels, int numlevels);
        FMOD_RESULT  setPaused            (bool paused);
        FMOD_RESULT  setPosition          (unsigned int position, FMOD_TIMEUNIT postype);
        FMOD_RESULT  getPosition          (unsigned int *position, FMOD_TIMEUNIT postype);
        FMOD_RESULT  setLoopPoints        (unsigned int loopstart, unsigned int looplength);
        FMOD_RESULT  setLoopCount         (int loopcount);
        FMOD_RESULT  setMode              (FMOD_MODE mode);      
        FMOD_RESULT  set3DAttributes      ();
        FMOD_RESULT  set3DMinMaxDistance  ();
        FMOD_RESULT  set3DOcclusion       (float directOcclusion, float reverbOcclusion);
        FMOD_RESULT  isPlaying            (bool *isplaying, bool includethreadlatency = false);
        FMOD_RESULT  getSpectrum          (float *spectrumarray, int numentries, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype);
        FMOD_RESULT  getWaveData          (float *wavearray, int numentries, int channeloffset);
// VERIFY: 4b504f5c auto
        // OFFSET: 0x0092A2E0
        FMOD_RESULT  getDSPHead           (DSPI **dsp)      { *dsp = mDSPHead; return FMOD_OK; }
        FMOD_RESULT  setReverbProperties  (const FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT  getReverbProperties  (FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT  setupDSPCodec        (DSPI *dsp);
        FMOD_RESULT  getDSPCodec          (DSPCodec **dsp)  { *dsp = mDSPCodec; return FMOD_OK; }

		virtual FMOD_RESULT	setupChannel  ();
		FMOD_RESULT	 updateChannel		  ();
    };
}

#endif  /* FMOD_SUPPORT_OPENAL */

#endif  /* _FMOD_CHANNEL_OPENAL_H */