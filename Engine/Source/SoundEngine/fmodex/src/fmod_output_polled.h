#ifndef _FMOD_OUTPUT_POLLED_H
#define _FMOD_OUTPUT_POLLED_H

#include "fmod_settings.h"

#include "fmod_outputi.h"
#include "fmod_thread.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    class Thread;
    class DSP;

    class OutputPolled : public Output, public Thread
    {
        DECLARE_MEMORYTRACKER_NONVIRTUAL

      private:
        
        int             mCursorBlock;

        FMOD_RESULT     threadFunc();

      protected:

        bool                mPolledFromMainThread;     
        FMOD_OS_SEMAPHORE  *mFinishedSema;

      public:
        
        OutputPolled();

        FMOD_RESULT     start();
        FMOD_RESULT     stop();
    };
}

#endif

