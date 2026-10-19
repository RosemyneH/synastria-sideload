#ifndef _FMOD_CHANNELGROUPI_H
#define _FMOD_CHANNELGROUPI_H

#include "fmod_settings.h"

#include "fmod.hpp"
#include "fmod_dsp_filter.h"
#include "fmod_linkedlist.h"
#include "fmod_string.h"
#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    class ChannelI;
    class DSPI;
    class DSPConnectionI;
    class SystemI;

    class ChannelGroupI : public LinkedListNode      /* This linked list node entry is for System::mChannelGroupHead */
    {
        DECLARE_MEMORYTRACKER

      public:
   
        static FMOD_RESULT validate(ChannelGroup *channelgroup, ChannelGroupI **channelgroupi);
        
        SystemI        *mSystem;
        DSPI           *mDSPHead;
        DSPI           *mDSPMixTarget;        /* By default this just points to mDSPHead unless dsp effects are added. */
        void           *mUserData;
        ChannelGroupI  *mParent;
        ChannelGroupI  *mGroupHead;
        LinkedListNode  mChannelHead;
        int             mNumChannels;
        char           *mName;
        float           mVolume, mRealVolume;
        float           mDirectOcclusion, mReverbOcclusion;
        float           mRealDirectOcclusionVolume, mRealReverbOcclusionVolume;
        float           mPitch;
        float           mRealPitch;

        bool            mMute;
        bool            mPaused;

      public:

// VERIFY: e156f30b auto
        // OFFSET: 0x008D51C0
        ChannelGroupI() 
        {
            mVolume = mRealVolume = 1;
            mPitch = mRealPitch = 1;
            mDirectOcclusion = mReverbOcclusion = 0;
            mRealDirectOcclusionVolume = mRealReverbOcclusionVolume = 1;
        }
                               
        FMOD_RESULT     release                ();
        FMOD_RESULT     releaseInternal        ();
        FMOD_RESULT     getSystemObject        (System **system);

        // Channelgroup scale values.  (changes attributes relative to the channels, doesn't overwrite them)
        FMOD_RESULT     setVolume              (float volume);
        FMOD_RESULT     setVolumeInternal      ();
        FMOD_RESULT     getVolume              (float *volume);
        FMOD_RESULT     setPitch               (float pitch);
        FMOD_RESULT     setPitchInternal       ();
        FMOD_RESULT     getPitch               (float *pitch);
		FMOD_RESULT	    set3DOcclusion         (float directocclusion, float reverbocclusion);
		FMOD_RESULT	    set3DOcclusionInternal ();
		FMOD_RESULT     get3DOcclusion         (float *directocclusion, float *reverbocclusion);
        FMOD_RESULT     setPaused              (bool paused, bool setpausedflag = true);
        FMOD_RESULT     getPaused              (bool *paused);
        FMOD_RESULT     setMute                (bool mute, bool setmuteflag = true);
        FMOD_RESULT     getMute                (bool *mute);

        // Channelgroup override values.  (recursively overwrites whatever settings the channels had)
        FMOD_RESULT     stop                    ();
        FMOD_RESULT     overrideVolume          (float volume);
        FMOD_RESULT     overrideFrequency       (float frequency);
        FMOD_RESULT     overridePan             (float pan);
        FMOD_RESULT     overrideReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES *prop);
        FMOD_RESULT     override3DAttributes    (const FMOD_VECTOR *pos, const FMOD_VECTOR *vel);
        FMOD_RESULT     overrideSpeakerMix      (float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright);

        // Nested channel groups.
        FMOD_RESULT     addGroup               (ChannelGroupI *group);
        FMOD_RESULT     getNumGroups           (int *numgroups);
        FMOD_RESULT     getGroup               (int index, ChannelGroupI **group);
        FMOD_RESULT     getParentGroup         (ChannelGroupI **group);

        // DSP functionality only for channel groups playing sounds created with FMOD_SOFTWARE.
        FMOD_RESULT     getDSPHead             (DSPI **dsp);
        FMOD_RESULT     addDSP                 (DSPI *dsp);

        // Information only functions.
        FMOD_RESULT     getName                (char *name, int namelen);
        FMOD_RESULT     getNumChannels         (int *numchannels);
        FMOD_RESULT     getChannel             (int index, Channel **channel);
        FMOD_RESULT     getSpectrum            (float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype);
        FMOD_RESULT     getWaveData            (float *wavearray, int numvalues, int channeloffset);

        // Userdata set/get.
        FMOD_RESULT     setUserData            (void *userdata);
        FMOD_RESULT     getUserData            (void **userdata);

    };

}

#endif


