#ifndef _FMOD_SOUND_STREAM_H
#define _FMOD_SOUND_STREAM_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_STREAMING

#ifndef _FMOD_SOUNDI_H
    #include "fmod_soundi.h"
#endif

namespace FMOD
{
    class Sample;
    class ChannelStream;

    class Stream : public SoundI
    {
        DECLARE_MEMORYTRACKER

      public:

        LinkedListNode  mStreamNode;
        ChannelStream  *mChannel;   /* Each stream has its own unique channel. */
        Sample         *mSample;
        unsigned int    mLastPos;
        int             mBlockSize;
        int             mLoopCountCurrent;
        int             mInitialPosition;

        bool            isStream() { return true; }

      public:

        Stream();

        FMOD_RESULT     fill(unsigned int offset, unsigned int length);
        FMOD_RESULT     flush();
        FMOD_RESULT     setPosition(unsigned int position, FMOD_TIMEUNIT postype);
        FMOD_RESULT     getPosition(unsigned int *position, FMOD_TIMEUNIT postype);
        FMOD_RESULT     setLoopCount(int loopcount);
    };
}

#endif

#endif

