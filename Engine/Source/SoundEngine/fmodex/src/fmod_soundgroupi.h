#ifndef _FMOD_SOUNDGROUPI_H
#define _FMOD_SOUNDGROUPI_H

#include "fmod_settings.h"

#include "fmod.hpp"
#include "fmod_linkedlist.h"
#include "fmod_string.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    class SoundI;
    class SystemI;

    class SoundGroupI : public LinkedListNode      /* This linked list node entry is for System::mSoundGroupHead */
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      public:
   
        static FMOD_RESULT validate(SoundGroup *soundgroup, SoundGroupI **soundgroupi);
        
        SystemI                  *mSystem;
        void                     *mUserData;
        LinkedListNode            mSoundHead;
        char                     *mName;
        int                       mMaxAudible;
        FMOD_SOUNDGROUP_BEHAVIOR  mMaxAudibleBehavior;

        int                       mPlayCount;
        int                       mNumPlaying;
        float                     mFadeSpeed;
        float                     mVolume;

      public:

        SoundGroupI();
                               
        FMOD_RESULT release              ();
        FMOD_RESULT releaseInternal      ();
        FMOD_RESULT getSystemObject      (System **system);
                                         
        // SoundGroup control functions. 
        FMOD_RESULT setMaxAudible        (int maxaudible);
        FMOD_RESULT getMaxAudible        (int *maxaudible);
        FMOD_RESULT setMaxAudibleBehavior(FMOD_SOUNDGROUP_BEHAVIOR behavior);
        FMOD_RESULT getMaxAudibleBehavior(FMOD_SOUNDGROUP_BEHAVIOR *behavior);
        FMOD_RESULT setMuteFadeSpeed     (float speed);
        FMOD_RESULT getMuteFadeSpeed     (float *speed);
        FMOD_RESULT setVolume            (float volume);
        FMOD_RESULT getVolume            (float *volume);
        FMOD_RESULT stop                 ();
                                         
        // Information only functions.   
        FMOD_RESULT getName              (char *name, int namelen);
        FMOD_RESULT getNumSounds         (int *numsounds);
        FMOD_RESULT getSound             (int index, Sound **sound);
        FMOD_RESULT getNumPlaying        (int *numplaying);

        // Userdata set/get.
        FMOD_RESULT setUserData          (void *userdata);
        FMOD_RESULT getUserData          (void **userdata);

    };
}

#endif


