#ifndef _FMOD_SOUND_SAMPLE_H
#define _FMOD_SOUND_SAMPLE_H

#include "fmod_settings.h"

#include "fmod_channeli.h"
#include "fmod_soundi.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    class Sample : public SoundI
    {
        DECLARE_MEMORYTRACKER

      public:

        int           mNumSubSamples;
        Sample       *mSubSample[FMOD_CHANNEL_MAXREALSUBCHANNELS];
        void         *mLockBuffer;
        unsigned int  mLockLength;
        unsigned int  mLockOffset;
        bool          mLockCanRead;

      public:

        Sample();
 
        FMOD_RESULT  release(bool freethis = true);
        FMOD_RESULT  lock(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
        FMOD_RESULT  unlock(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);

// VERIFY: 952e1c12 auto
        // OFFSET: 0x0091AF50
        virtual FMOD_RESULT  lockInternal(unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2) { return FMOD_ERR_SUBSOUNDS; } 
        virtual FMOD_RESULT  unlockInternal(void *ptr1, void *ptr2, unsigned int len1, unsigned int len2) { return FMOD_OK; } 
// VERIFY: 16f70288 auto
        // OFFSET: 0x0091AF60
        virtual FMOD_RESULT  setBufferData(void *data) { return FMOD_ERR_NEEDSSOFTWARE; }
        virtual FMOD_RESULT  setMode(FMOD_MODE mode);

        FMOD_RESULT  setDefaults          (float frequency, float volume, float pan, int priority);
        FMOD_RESULT  setVariations        (float frequencyvar, float volumevar, float panvar);
        FMOD_RESULT  set3DMinMaxDistance  (float min, float max);
        FMOD_RESULT  set3DConeSettings    (float insideconeangle, float outsideconeangle, float outsidevolume);
        FMOD_RESULT  setLoopCount         (int loopcount);
        FMOD_RESULT  setLoopPoints        (unsigned int loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int loopend, FMOD_TIMEUNIT loopendtype);
   };
}

#endif

