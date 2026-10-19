#ifndef _FMOD_SYNCPOINT_H
#define _FMOD_SYNCPOINT_H

#include "fmod_settings.h"

namespace FMOD
{
    class SoundI;

    class SyncPoint : public SortedLinkedListNode
    {
      public:
	    unsigned int    mOffset;
        char            mName[FMOD_STRING_MAXNAMELEN];
        int             mIndex;
    };

}

#endif


