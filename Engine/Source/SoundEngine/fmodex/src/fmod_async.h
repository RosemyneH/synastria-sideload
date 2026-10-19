#ifndef _FMOD_ASYNC_H
#define _FMOD_ASYNC_H

#ifndef _FMOD_SETTINGS_H
#include "fmod_settings.h"
#endif

#ifndef _FMOD_LINKEDLIST_H
#include "fmod_linkedlist.h"
#endif
#ifndef _FMOD_THREAD_H
#include "fmod_thread.h"
#endif
#ifndef _FMOD_SOUND_STREAM_H
#include "fmod_sound_stream.h"
#endif

typedef FMOD_RESULT (F_CALLBACK *FMOD_ASYNC_CALLBACK)();

namespace FMOD
{
    class SoundI;
   
    class AsyncThread : public LinkedListNode   /* This linked list node entry is for AsyncThread::gAsyncHead */
    {
        friend class SystemI;
        friend class Stream;
        friend class SoundI;
        friend class ChannelStream;
        friend void asyncThreadFunc(void *data);

      private:

        Thread                          mThread;
        bool                            mThreadActive;
        LinkedListNode                  mHead;
        FMOD_OS_CRITICALSECTION        *mCrit;
        bool                            mOwned;
        bool                            mBusy;
        bool                            mDone;
        LinkedListNode                  mCallbackHead;

        FMOD_RESULT                     threadFunc();
        FMOD_RESULT                     init(bool owned);
        FMOD_RESULT                     reallyRelease();


      public :

        static FMOD_OS_CRITICALSECTION *gAsyncCrit;
        static LinkedListNode           gAsyncHead;

        AsyncThread();

        FMOD_RESULT                     release();
        FMOD_RESULT F_API               wakeupThread();

        static FMOD_RESULT              getAsyncThread(SoundI *sound);
        static FMOD_RESULT              update();
        static FMOD_RESULT              shutDown();
        static FMOD_RESULT F_API        addCallback(FMOD_ASYNC_CALLBACK callback, AsyncThread **asyncthread);
        static FMOD_RESULT F_API        removeCallback(FMOD_ASYNC_CALLBACK callback);
    };

    struct AsyncData
    {
        char                    mName[FMOD_STRING_MAXNAMELEN];
        AsyncThread            *mThread;
        LinkedListNode          mNode;
        void                   *mNameData;
        FMOD_CREATESOUNDEXINFO  mExInfo;
        bool                    mExInfoExists;

        FMOD_RESULT             mResult;
    };
}

#endif
