#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_NONBLOCKING

#include "fmod_async.h"
#include "fmod_dspi.h"
#include "fmod_soundi.h"
#include "fmod_systemi.h"

namespace FMOD
{

FMOD_OS_CRITICALSECTION *AsyncThread::gAsyncCrit = 0;
LinkedListNode           AsyncThread::gAsyncHead;


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 1108165e auto
// OFFSET: 0x00921FC0
void asyncThreadFunc(void *data)
{
    AsyncThread *asyncThread = (AsyncThread *)data;
    asyncThread->threadFunc();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: c17661ce manual
// OFFSET: 0x00921FD0
AsyncThread::AsyncThread()
{
    mCrit         = 0;
    mThreadActive = false;
    mBusy         = false;
    mDone         = false;

    if (!gAsyncCrit)
    {
        FMOD_OS_CriticalSection_Create(&gAsyncCrit);
    }
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 905f0f62 auto
// OFFSET: 0x00922040
FMOD_RESULT AsyncThread::shutDown()
{
    if (gAsyncCrit)
    {
        AsyncThread *asyncthread;
        LinkedListNode *current, *next;

        FMOD_OS_CriticalSection_Enter(gAsyncCrit);
        {
            current = gAsyncHead.getNext();
            while (current != &gAsyncHead)
            {
                next = current->getNext();

                asyncthread = SAFE_CAST(AsyncThread, current);
                asyncthread->reallyRelease();

                current = next;
            }
        }
        FMOD_OS_CriticalSection_Leave(gAsyncCrit);

        if (gAsyncCrit)
        {
            FMOD_OS_CriticalSection_Free(gAsyncCrit);
            gAsyncCrit = 0;
        }
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
FMOD_RESULT AsyncThread::addCallback(FMOD_ASYNC_CALLBACK callback, AsyncThread **asyncthread)
{
    FMOD_RESULT result;

    if (asyncthread)
    {
        *asyncthread = 0;
    }

    LinkedListNode *node = (LinkedListNode *)FMOD_Object_Alloc(LinkedListNode);
    if (!node)
    {
        return FMOD_ERR_MEMORY;
    }

    node->setData((void *)callback);

    /*
        Make sure there's a thread for us
    */
    result = getAsyncThread(0);
    if (result != FMOD_OK)
    {
        return result;
    }

    LinkedListNode *threadnode = gAsyncHead.getNext();
    if (threadnode != &gAsyncHead)
    {
        AsyncThread *thread = SAFE_CAST(AsyncThread, threadnode);
        FMOD_OS_CriticalSection_Enter(gAsyncCrit);
        {
            node->addBefore(&thread->mCallbackHead);
        }
        FMOD_OS_CriticalSection_Leave(gAsyncCrit);

        if (asyncthread)
        {
            *asyncthread = thread;
        }
    }
    else
    {
        return FMOD_ERR_INTERNAL;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
FMOD_RESULT AsyncThread::removeCallback(FMOD_ASYNC_CALLBACK callback)
{
    LinkedListNode *threadnode = gAsyncHead.getNext();

    if (threadnode != &gAsyncHead)
    {
        AsyncThread *asyncthread = SAFE_CAST(AsyncThread, threadnode);

        FMOD_OS_CriticalSection_Enter(gAsyncCrit);

        LinkedListNode *callbacknode = asyncthread->mCallbackHead.getNext();
        for (; callbacknode != &asyncthread->mCallbackHead; callbacknode = callbacknode->getNext())
        {
            if (callbacknode->getData() == (void *)callback)
            {
                callbacknode->removeNode();
                FMOD_Memory_Free(callbacknode);
                break;
            }
        }

        FMOD_OS_CriticalSection_Leave(gAsyncCrit);
    }
    else
    {
        return FMOD_ERR_INTERNAL;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 06b548ac auto
// OFFSET: 0x009220A0
FMOD_RESULT AsyncThread::init(bool owned)
{
    FMOD_RESULT result;

    mOwned = owned;

    result = FMOD_OS_CriticalSection_Create(&mCrit);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mThread.initThread("FMOD thread for FMOD_NONBLOCKING", asyncThreadFunc, this, ASYNC_THREADPRIORITY, 0, ASYNC_STACKSIZE, true, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    mThreadActive = true;

    FMOD_OS_CriticalSection_Enter(gAsyncCrit);
    addBefore(&gAsyncHead);
    FMOD_OS_CriticalSection_Leave(gAsyncCrit);

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
FMOD_RESULT AsyncThread::release()
{
    if (mOwned)
    {
        mDone = true;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 13745afd auto
// OFFSET: 0x00921C60
FMOD_RESULT AsyncThread::reallyRelease()
{
    FMOD_OS_CriticalSection_Enter(mCrit);
    {
        LinkedListNode *next, *current = mCallbackHead.getNext();
        while (current != &mCallbackHead)
        {
            next = current->getNext();
            current->removeNode();
            FMOD_Memory_Free(current);
            current = next;
        }
    }
    FMOD_OS_CriticalSection_Leave(mCrit);

    removeNode();

    mThreadActive = false;
    mThread.closeThread();

    if (mCrit)
    {
        FMOD_OS_CriticalSection_Free(mCrit);
    }

    FMOD_Memory_Free(this);

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
FMOD_RESULT AsyncThread::wakeupThread()
{
    return mThread.wakeupThread();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 9532e982 auto
// OFFSET: 0x00921D30
FMOD_RESULT AsyncThread::update()
{
    if (gAsyncCrit)
    {
        AsyncThread *asyncthread;
        LinkedListNode *current, *next;

        FMOD_OS_CriticalSection_Enter(gAsyncCrit);
        {
            current = gAsyncHead.getNext();
            while (current != &gAsyncHead)
            {
                next = current->getNext();

                asyncthread = SAFE_CAST(AsyncThread, current);
                if (asyncthread->mDone)
                {
                    asyncthread->reallyRelease();
                }

                current = next;
            }
        }
        FMOD_OS_CriticalSection_Leave(gAsyncCrit);
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: b3a96161 auto
// OFFSET: 0x00921D80
FMOD_RESULT AsyncThread::threadFunc()
{
    FMOD_RESULT     result = FMOD_OK;
    SoundI         *sound = 0;
    LinkedListNode *current;

    if (!mThreadActive)
    {
        return FMOD_OK;
    }

    FMOD_OS_CriticalSection_Enter(mCrit);
    {
        current = mHead.getNext();
        if (current != &mHead)
        {
            sound = (SoundI *)current->getData();
            current->removeNode();
            mBusy = true;
        }
    }
    FMOD_OS_CriticalSection_Leave(mCrit);

    if (sound)
    {
        SystemI *system = sound->mSystem;

        if (sound->mOpenState == FMOD_OPENSTATE_LOADING)
        {
            if (sound->mMode & (FMOD_OPENMEMORY | FMOD_OPENMEMORY_POINT))
            {
                result = system->createSoundInternal((const char *)sound->mAsyncData->mNameData, sound->mMode, sound->mAsyncData->mExInfoExists ? &sound->mAsyncData->mExInfo : 0, true, &sound);
            }
            else
            {
                result = system->createSoundInternal(sound->mAsyncData->mName, sound->mMode, sound->mAsyncData->mExInfoExists ? &sound->mAsyncData->mExInfo : 0, true, &sound);
            }
        }
#ifdef FMOD_SUPPORT_STREAMING
        else if (sound->mOpenState == FMOD_OPENSTATE_SEEKING)
        {
            Stream *stream = SAFE_CAST(Stream, sound);

            result = stream->updateSubSound(stream->mSubSoundIndex, false);
            if (result == FMOD_OK)
            {
                result = stream->Stream::setPosition(0, FMOD_TIMEUNIT_PCM);
                if (result == FMOD_OK)
                {
                    result = stream->flush();
                }
            }
        }
#endif

        sound->mAsyncData->mThread = 0;
        sound->mAsyncData->mResult = result;
        sound->mFlags             |= FMOD_SOUND_FLAG_DONOTRELEASE;                                               /* Dissalow release... */
        sound->mOpenState          = (result == FMOD_OK) ? FMOD_OPENSTATE_READY : FMOD_OPENSTATE_ERROR;     /* .. but allow everything else. */

        if (sound->mSubSoundParent)
        {
            sound->mSubSoundParent->mOpenState = sound->mOpenState;
        }

        mBusy = false;

        if (sound->mAsyncData->mExInfoExists && sound->mAsyncData->mExInfo.nonblockcallback)
        {
            sound->mUserData = sound->mAsyncData->mExInfo.userdata;
            sound->mAsyncData->mExInfo.nonblockcallback((FMOD_SOUND *)sound, result);
        }

        sound->mFlags &= ~FMOD_SOUND_FLAG_DONOTRELEASE;

        /*
            Release this thread if it's owned
        */
        release();
    }

    /*
        Call any callbacks that have been added
    */
    LinkedListNode *node;

    FMOD_OS_CriticalSection_Enter(mCrit);
    {
        node = mCallbackHead.getNext();
    }
    FMOD_OS_CriticalSection_Leave(mCrit);

    while (node != &mCallbackHead)
    {
        FMOD_ASYNC_CALLBACK callback = (FMOD_ASYNC_CALLBACK)node->getData();

        result = callback();
        if (result != FMOD_OK)
        {
            return result;
        }

        FMOD_OS_CriticalSection_Enter(mCrit);
        {
            node = node->getNext();
        }
        FMOD_OS_CriticalSection_Leave(mCrit);
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: d5e037a5 auto
// OFFSET: 0x00922130
FMOD_RESULT AsyncThread::getAsyncThread(SoundI *sound)
{
    FMOD_RESULT     result;
    LinkedListNode *current;
    bool            found;
    AsyncThread    *asyncthread = 0;

    found = false;

    if (!gAsyncCrit)
    {
        result = FMOD_OS_CriticalSection_Create(&gAsyncCrit);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    FMOD_OS_CriticalSection_Enter(gAsyncCrit);
    {
        current = gAsyncHead.getNext();
        if (current != &gAsyncHead)
        {
            asyncthread = SAFE_CAST(AsyncThread, current);

            FMOD_OS_CriticalSection_Enter(asyncthread->mCrit);
            {
                found = true;       /* The first asyncthread isn't doing anything so use it */
            }
            FMOD_OS_CriticalSection_Leave(asyncthread->mCrit);
        }
    }
    FMOD_OS_CriticalSection_Leave(gAsyncCrit);

    if (!found)
    {
        asyncthread = FMOD_Object_Alloc(AsyncThread);
        if (!asyncthread)
        {
            return FMOD_ERR_MEMORY;
        }

        result = asyncthread->init(false);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    if (sound)
    {
        sound->mAsyncData->mThread = asyncthread;
    }

    return FMOD_OK;
}


}

#endif
