#ifndef _FMOD_CHANNEL_DSOUND_H
#define _FMOD_CHANNEL_DSOUND_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_DSOUND

#include "fmod_channel_real.h"

#ifdef PLATFORM_32BIT
    typedef unsigned long DWORD;
    typedef DWORD DWORD_PTR;
#endif

#include <dsound.h>

namespace FMOD
{
    class Output;

    class ChannelDSound : public ChannelReal
    {
        friend class OutputDSound;

      private:

        bool                          mLOCSoftware;

      protected:
        
        OutputDSound                 *mOutputDSound;
        IDirectSoundBuffer8          *mBuffer;
        IDirectSound3DBuffer         *mBuffer3D;
        IKsPropertySet               *mBufferReverb;
        IKsPropertySet               *mBufferQuality;

      public:
        
        // OFFSET: 0x0093AA60
        ChannelDSound() {}

        FMOD_RESULT  init                 (int index, SystemI *system, Output *output, DSPI *dspmixtarget);
        FMOD_RESULT  alloc                ();
        FMOD_RESULT  start                ();

        FMOD_RESULT  stop                 (bool ignoreenddelay, bool resetstate);
        FMOD_RESULT  setVolume            (float volume);
        FMOD_RESULT  setFrequency         (float frequency);
        FMOD_RESULT  setPan               (float pan, float fbpan = 1);
        FMOD_RESULT  setSpeakerLevels     (int speaker, float *levels, int numlevels);
        FMOD_RESULT  setSpeakerMix        (float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright);
        FMOD_RESULT  setPaused            (bool paused);
        FMOD_RESULT  setPosition          (unsigned int position, FMOD_TIMEUNIT postype);
        FMOD_RESULT  getPosition          (unsigned int *position, FMOD_TIMEUNIT postype);
        FMOD_RESULT  set3DAttributes      ();
        FMOD_RESULT  set3DMinMaxDistance  ();
        FMOD_RESULT  set3DOcclusion       (float directOcclusion, float reverbOcclusion);

        FMOD_RESULT  isPlaying            (bool *isplaying, bool includethreadlatency = false);
        FMOD_RESULT  setMode              (FMOD_MODE mode);

        FMOD_RESULT  getBuffer3D          (IDirectSound3DBuffer **buffer3d);
    };
}

#endif /* FMOD_SUPPORT_DSOUND */

#endif