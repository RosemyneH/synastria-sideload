#include "fmod_settings.h"

#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_string.h"
#include "fmod_stringw.h"
#include "fmod_thread.h"
#include "fmod_time.h"
#include "fmod_types.h"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_systemi_spu.h"
    #include "fmod_spu_printf.h"
    #include "fmod_file_dma.h"
#else
    #include "fmod_systemi.h"
#endif

#ifdef PLATFORM_PS2_EE
    #include "fmod_cmd.h"
#endif

#ifdef FMOD_SUPPORT_NET
  #include "fmod_file_net.h"
#endif

#ifdef FMOD_SUPPORT_CDDA
  #include "fmod_file_cdda.h"
  #include "fmod_os_cdda.h"
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef PLATFORM_PS3_SPU
extern "C"
{

/*
[API]
[
	[DESCRIPTION]
	Mutex function to synchronize user file reads with FMOD's file reads.  This function tells fmod that you are using the disk so that it will 
    block until you are finished with it.<br>
    This function also blocks if FMOD is already using the disk, so that you cannot do a read at the same time FMOD is reading.

	[PARAMETERS]
    'busy'  1 = you are about to perform a disk access.  0 = you are finished with the disk.
 
	[RETURN_VALUE]

	[REMARKS]
    Use this function as a wrapper around your own file reading functions if you want to do simulatenous file reading while FMOD is also reading.
    ie
    <PRE>
    FMOD_File_SetDiskBusy(1);
    myfread(...);
    FMOD_File_SetDiskBusy(0);
    </PRE>
    Warning! This is a critical section internally.  If you do not match your busy = true with a busy = false your program may hang!<br>
    If you forget to set diskbusy to false it will stop FMOD from reading from the disk.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    File_GetDiskBusy
]
*/
// VERIFY: b93fe9a9 auto
// OFFSET: 0x008E6B10
FMOD_RESULT F_API FMOD_File_SetDiskBusy(int busy)
{
    if (busy)
    {
        FMOD_OS_CriticalSection_Enter(FMOD::File::gFileCrit);
        FMOD::gGlobal->gFileBusy = true;
    }
    else
    {
        FMOD::gGlobal->gFileBusy = false;
        FMOD_OS_CriticalSection_Leave(FMOD::File::gFileCrit);
    }

    return FMOD_OK;
}


/*
[API]
[
	[DESCRIPTION]
	Information function to retreive the state of fmod's disk access.

	[PARAMETERS]
    'busy'      Address of an integer to receive the busy state of the disk at the current time.
 
	[RETURN_VALUE]

	[REMARKS]
    Do not use this function to syncrhonize your own reads with, as due to timing, you might call this function and it says false = it is not busy,
    but the split second after call this function, internally FMOD might set it to busy.  Use File_SetDiskBusy for proper mutual exclusion as it uses semaphores.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    File_SetDiskBusy
]
*/
FMOD_RESULT F_API FMOD_File_GetDiskBusy(int *busy)
{
    if (!busy)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *busy = FMOD::gGlobal->gFileBusy;

    return FMOD_OK;
}

}
#endif

namespace FMOD
{

LinkedListNode           File::gFileThreadHead;
FMOD_OS_CRITICALSECTION *File::gFileCrit          = 0;
bool                     File::gUsesUserCallbacks = false;
int                      File::gBufferSize        = FILE_SECTORSIZE;

#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: dc844bab manual
// OFFSET: 0x008E7D80
void fileThreadFunc(void *data)
{
    FileThread *fileThread = (FileThread *)data;

    fileThread->threadFunc();
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
FMOD_RESULT FileThread::threadFunc()
{
    if (mThreadActive)
    {
        File           *file;

        FMOD_OS_CriticalSection_Enter(mFileListCrit);

        mFileListCurrent = mFileListHead.getNext();

        while (mFileListCurrent != &mFileListHead)
        {            
            mFileListNext = mFileListCurrent->getNext();

            file = SAFE_CAST(File, mFileListCurrent);
            if (file->mFlipping)
            {
                FMOD_OS_CriticalSection_Leave(mFileListCrit);
        
                file->flip(false);
        
                FMOD_OS_CriticalSection_Enter(mFileListCrit);
            }

            mFileListCurrent = mFileListNext;
        }

        FMOD_OS_CriticalSection_Leave(mFileListCrit);
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
// VERIFY: 003d2369 auto
// OFFSET: 0x008E7BC0
FileThread::FileThread()
{
    mFileListCrit    = 0;
    mFileListCurrent = 0;
    mFileListNext    = 0;
    mThreadActive    = false;
    mDeviceType      = 0;
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
// VERIFY: cd4089c1 auto
// OFFSET: 0x008E7D90
FMOD_RESULT FileThread::init(int devicetype, bool owned)
{
    FMOD_RESULT result;

    mDeviceType = devicetype;
    mOwned      = owned;

    result = FMOD_OS_CriticalSection_Create(&mFileListCrit);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mThread.initThread("FMOD file thread", fileThreadFunc, this, FILE_THREADPRIORITY, 0, FILE_STACKSIZE, true, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    mThreadActive = true;

    addAfter(&File::gFileThreadHead);

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
// VERIFY: be1524c1 auto
// OFFSET: 0x008E6B60
FMOD_RESULT FileThread::release()
{
    removeNode();

    mThreadActive = false;

    mThread.closeThread();

    if (mFileListCrit)
    {
        FMOD_OS_CriticalSection_Free(mFileListCrit);
    }

    FMOD_Memory_Free(this);

    return FMOD_OK;
}

#endif  //!PLATFORM_PS3_SPU

#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 0aabb186 manual
// OFFSET: 0x008E7E10
FMOD_RESULT File::getFileThread()
{
    FMOD_RESULT     result;
    LinkedListNode *current;
    int             devicetype;
    bool            found, owned;
    FileThread     *filethread = 0;

    devicetype = DEVICE_DISK;
    owned      = false;

#ifdef FMOD_SUPPORT_NET
    if (!FMOD_strnicmp("http://", mName, 7))
    {
        devicetype = DEVICE_NET;
        owned      = true;
    }
#endif
#ifdef FMOD_SUPPORT_CDDA
    if (FMOD_OS_CDDA_IsDeviceName(mName))
    {
        devicetype = DEVICE_CDDA;
        owned      = true;
    }
#endif

    found = false;
    if (devicetype == DEVICE_DISK)
    {
        current = gFileThreadHead.getNext();
        while (current != &gFileThreadHead)
        {
            filethread = SAFE_CAST(FileThread, current);

            if (filethread->mDeviceType == DEVICE_DISK)
            {
                found = true;
                break;
            }

            current = current->getNext();
        }
    }

    if (!found)
    {
        filethread = FMOD_Object_Alloc(FileThread);
        if (!filethread)
        {
            return FMOD_ERR_MEMORY;
        }

        result = filethread->init(devicetype, owned);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    mFileThread = filethread;

    return FMOD_OK;
}

#endif // !PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: a58d28c1 auto
// OFFSET: 0x008E7C10
File::File()
{
    init(0, gBufferSize);
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
// VERIFY: 614f432f manual
// OFFSET: 0x008E6BC0
FMOD_RESULT File::init(unsigned int filesize, int blocksize)
{
    mBlockSize           = blocksize;
    mBufferIsString      = true;
    mSeekable            = true;
    mBusy                = false;
    mFlipping            = false;
    mFileThread          = 0;
    mRiderHandle         = 0;
    mRiderUserData       = 0;
    mSystem              = 0;
    mBigEndian           = false;
    mStartOffset         = 0;
    mCurrentPosition     = 0;
    mNextPosition        = 0;
    mNextPositionDisplay = 0;
    mBlockOffset         = 0;
    mBuffer              = 0;
    mBufferPos           = 0;
    mBufferSkip          = 0;
    mAsyncError          = FMOD_OK;
    mExit                = false;
    mStarving            = false;
    mEncryptionKeyLength = 0;
    mEncryptionKeyIndex  = 0;
    mSema                = 0;
    mPercentBuffered     = 0;
    mLengthOriginal      = filesize;
    mLength              = filesize;
    mFileSize            = filesize;

    FMOD_memset(mName, 0, FMOD_STRING_MAXNAMELEN);

    return FMOD_OK;
}

#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 65341bb9 auto
// OFFSET: 0x008E6C80
FMOD_RESULT File::shutDown()
{
    LinkedListNode *current, *next;

    current = gFileThreadHead.getNext();
    while (current != &gFileThreadHead)
    {
        FileThread *filethread;

        next = current->getNext();

        filethread = SAFE_CAST(FileThread, current);
        filethread->release();

        current = next;
    }

#ifdef FMOD_SUPPORT_NET
    NetFile::shutDown();
#endif
#ifdef FMOD_SUPPORT_CDDA
    CddaFile::shutDown();
#endif

    if (gFileCrit)
    {
        FMOD_OS_CriticalSection_Free(gFileCrit);
        gFileCrit = 0;
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
// VERIFY: 53c39857 manual
// OFFSET: 0x008E6D20
FMOD_RESULT F_API File::open(const char *name_or_data, unsigned int length, bool unicode, const char *encryptionkey)
{
    FMOD_RESULT result;

    mUnicode             = unicode;
    mBigEndian           = false;
    mStartOffset         = 0;
    mCurrentPosition     = 0;
    mNextPosition        = 0;
    mNextPositionDisplay = 0;
    mBlockOffset         = 0;
    mBuffer              = 0;
    mBufferPos           = 0;
    mBufferSkip          = 0;
    mAsyncError          = FMOD_OK;
    mExit                = false;
    mStarving            = false;
    mEncryptionKeyLength = 0;
    mEncryptionKeyIndex  = 0;
    mLength              = length;
    mFileSize            = length;

    if (encryptionkey)
    {
        mEncryptionKeyLength = FMOD_strlen(encryptionkey);
        if (mEncryptionKeyLength > 32)
        {
            mEncryptionKeyLength = 32;
        }

        FMOD_memset(mEncryptionKey, 0, 32);
        FMOD_strncpy(mEncryptionKey, encryptionkey, mEncryptionKeyLength);
    }

    mBufferSize = mBlockSize;           /* Blocking single buffer */
    if (mBufferSize)
    {
        mBuffer = FMOD_Memory_Calloc(mBufferSize);
        if (!mBuffer)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    result = reallyOpen(name_or_data, &mFileSize);
    if (result != FMOD_OK)
    {
        if (mBuffer)
        {
            FMOD_Memory_Free(mBuffer);
            mBuffer = 0;
        }
        return result;
    }

    if (name_or_data && mBufferIsString)
    {
        FMOD_strncpy(mName, name_or_data, FMOD_STRING_MAXNAMELEN);
        mName[FMOD_STRING_MAXNAMELEN - 1] = 0;
    }

    if (mSystem && mSystem->mOpenCallback)
    {
        mSystem->mOpenCallback(name_or_data, unicode, &mFileSize, &mRiderHandle, &mRiderUserData);
    }

    if (!mLength)
    {
        mLength = mFileSize;
    }
    mLengthOriginal = mLength;

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
// VERIFY: 3a8528d9 auto
// OFFSET: 0x008E7C40
FMOD_RESULT F_API File::close()
{
    FMOD_RESULT result;

    /*
        Cancel any pending file requests.
        This is needed for Wii / GC and to cancel net reads.
    */
    cancel();

    if (mSema && mBusy)
    {
        FMOD_OS_Semaphore_Wait(mSema);
        FMOD_OS_Semaphore_Signal(mSema);
    }

    if (mFileThread)
    {
        FMOD_OS_CriticalSection_Enter(mFileThread->mFileListCrit);

        if (this == mFileThread->mFileListNext)
        {
            mFileThread->mFileListNext = getNext();
        }
        removeNode();

        FMOD_OS_CriticalSection_Leave(mFileThread->mFileListCrit);

        if (mFileThread->mOwned)
        {
            mFileThread->release();
        }

        mFileThread = 0;
    }

    result = reallyClose();

    if (mSema)
    {
        FMOD_OS_Semaphore_Free(mSema);
    }

    if (mSystem && mSystem->mCloseCallback)
    {
        mSystem->mCloseCallback(mRiderHandle, mRiderUserData);
    }

    if (mBuffer)
    {
        FMOD_Memory_Free(mBuffer);
        mBuffer = 0;
    }

    return result;
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
// VERIFY: 99794d3e auto
// OFFSET: 0x008E6F00
FMOD_RESULT F_API File::cancel()
{
    mExit = true;
    return reallyCancel();
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
// VERIFY: 6aff47ad auto
// OFFSET: 0x008E6EE0
bool File::readyForClose()
{
    return !mSema || !mBusy;
}

#endif // !PLATFORM_PS3_SPU


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 04da6811 auto
// OFFSET: 0x008E6F20
FMOD_RESULT File::flip(bool frommainthread)
{
    FMOD_RESULT   result = FMOD_OK;

    if (frommainthread && mSema)
    {
        FMOD_OS_Semaphore_Wait(mSema);
    }

    {
        unsigned int  bytestoread = mBlockSize - mBufferSkip;
        char         *destptr     = (char *)mBuffer + mBlockOffset + mBufferSkip;

        mBusy = true;

        while (bytestoread)
        {
            unsigned int  r = 0;

            result = reallyRead(destptr, bytestoread, &r);
            if (result != FMOD_OK)
            {
                bytestoread = r;

                if ((int)mLength == -1 && result == FMOD_ERR_FILE_EOF)
                {
                    if (mNextPositionDisplay)
                    {
                        mLength = mFileSize = mLengthOriginal = mNextPositionDisplay;
                    }
                }
            }

            if (mSystem && mSystem->mReadCallback)
            {
                mSystem->mReadCallback(mRiderHandle, destptr, r, 0, mRiderUserData);
            }

            if (!r || r > bytestoread)
            {
                break;
            }

            mNextPositionDisplay += r;
            bytestoread          -= r;
            destptr              += r;

            mPercentBuffered = (int)((float)(mNextPositionDisplay - mCurrentPosition) / (float)mBufferSize * 100.0f);
            if (mPercentBuffered < 0)
            {
                mPercentBuffered = 0;
            }

            if (mExit)
            {
                break;
            }
        }

        mBlockOffset += mBlockSize;
        if (mBlockOffset >= mBufferSize)
        {
            mBlockOffset = 0;
        }

        mBufferSkip = 0;
        mAsyncError = result;
        mFlipping = false;
    }

    mBusy = false;

    if (mSema)
    {
        FMOD_OS_Semaphore_Signal(mSema);
    }

    return result;
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
// VERIFY: c8b67a72 auto
// OFFSET: 0x008E70E0
FMOD_RESULT File::seekAndReset()
{
    FMOD_RESULT result;
    unsigned int alignedpos;

    if (mSema && mBusy)
    {
        FMOD_OS_Semaphore_Wait(mSema);
        FMOD_OS_Semaphore_Signal(mSema);
    }

    alignedpos = mCurrentPosition;
    alignedpos /= mBufferSize;      /* We are seeking resetting to the start of the WHOLE buffer */
    alignedpos *= mBufferSize;

    mBufferPos           = mCurrentPosition - alignedpos;
    mBlockOffset         = 0;
    mNextPosition        = alignedpos; /* This will immediately be incremeneted by the flip */
    mNextPositionDisplay = mNextPosition;
    mBufferSkip          = 0;

    result = reallySeek(alignedpos);

    if (mSystem && mSystem->mSeekCallback)
    {
        mSystem->mSeekCallback(mRiderHandle, alignedpos, mRiderUserData);
    }

    return result;
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
// VERIFY: 762430c0 manual
// OFFSET: 0x008E7180
FMOD_RESULT File::checkBufferedStatus()
{
    FMOD_RESULT result;
    int diff;

    if (mAsyncError != FMOD_OK && mAsyncError != FMOD_ERR_FILE_EOF)
    {
        return mAsyncError;
    }

    if (mNextPosition < mCurrentPosition)     /* If the position after the buffer */
    {
        diff = -1;
    }
    else
    {
        mPercentBuffered = (int)((float)(mNextPositionDisplay - mCurrentPosition) / (float)mBufferSize * 100.0f);
        if (mPercentBuffered < 0 || mBufferSkip)
        {
            mPercentBuffered = 0;
        }

        diff = mNextPosition - mCurrentPosition;
        diff += (mBlockSize - 1);
        diff /= (int)mBlockSize;
    }

    if (mBufferSkip)
    {
        if (diff > 2)
        {
            mBufferSkip = 0;
        }
        else
        {
            diff = -1;                          /* We want to do a blocking read here.  Not a background flip. -1 stops it from going into first 2 if statements. */
            mNextPositionDisplay = mBufferSkip;
            mNextPosition        = mCurrentPosition - mBufferPos;
        }
    }

    if (diff != 2)
    {
        mStarving = true;
        if (mBusy)
        {
            FMOD_OS_Semaphore_Wait(mSema);
            FMOD_OS_Semaphore_Signal(mSema);
        }
        mStarving = false;
    }

    /*
        If diff = 1, then we are reading from the front buffer, so non blocking fill the back buffer.
    */
    if (diff == 1 && mBufferSize > mBlockSize)  /* This case is only for double buffered (buffsize > blocksize) */
    {
        mBusy = true;
        mFlipping = true;

        FMOD_OS_Semaphore_Wait(mSema);      /* ENTER */

        mFileThread->mThread.wakeupThread();

        mNextPositionDisplay = mNextPosition;
        mNextPosition += mBlockSize;

        return FMOD_OK;
    }
    /*
        If diff = 2, then we are reading from the front buffer and the fill is more than 1 ahead, so there is no current danger of undderun.
        This means we can just exit.
    */
    else if ((mBufferSize > mBlockSize && diff == 2) || (mBufferSize == mBlockSize && diff == 1)) /* This is the idle case.  If it is doublebuffered the diff must be 2, or if it is single buffered the diff must be 1 */
    {
        return FMOD_OK;
    }
    /*
        If diff = 0, or diff = something large (ie there was a seek), do a blocking read.
    */
    else
    {
        if (diff && mSeekable)  /* was there a seek */
        {
            result = seekAndReset();
            if (result != FMOD_OK)
            {
                return result;
            }
        }

        /*
            Fill the front buffer.
        */
        result = flip(true);
        if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
        {
            return result;
        }

        if (mBufferSize == mBlockSize && result == FMOD_ERR_FILE_EOF && (int)mLength == -1)
        {
            return result;
        }

        mNextPositionDisplay = mNextPosition;
        mNextPosition += mBlockSize;

        /*
            Fill the back buffer.
        */
        if (mBufferPos >= mBlockSize)
        {
            result = flip(true);
            if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
            {
                return result;
            }

            mNextPositionDisplay = mNextPosition;
            mNextPosition += mBlockSize;
        }

        return result;
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
// VERIFY: ed4982e0 manual
// OFFSET: 0x008E73C0
FMOD_RESULT F_API File::read(void *buffer, unsigned int size, unsigned int count, unsigned int *rd)
{
    FMOD_RESULT  result = FMOD_OK;
    unsigned int oldsize = size;
    unsigned int bytesleft;
    unsigned int r = 0;
    bool         eof = false;

    if (!buffer)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if ((int)(size * count) < 0)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    size *= count;

    if ((mCurrentPosition + size) > mStartOffset + mLength)
    {
        if (mCurrentPosition <= mStartOffset + mLength)
        {
            size = (mStartOffset + mLength) - mCurrentPosition;
        }
        else
        {
            return FMOD_ERR_FILE_BAD;
        }
        eof = true;
    }

    bytesleft = size;

    while (bytesleft)
    {
        size = bytesleft;

        /*
            Direct read - single buffered only.
        */
        if (mBlockSize == mBufferSize && size > mBlockSize && !mBufferPos && mSeekable)
        {
            if (mBlockSize)
            {
                if (mCurrentPosition != mNextPosition)
                {
                    result = seekAndReset();
                    if (result != FMOD_OK)
                    {
                        return result;
                    }
                }

                size /= mBlockSize;
                size *= mBlockSize;
            }

            result = reallyRead((char *)buffer + r, size, &size);

            mBusy = false;

            if (mSystem && mSystem->mReadCallback)
            {
                mSystem->mReadCallback(mRiderHandle, (char *)buffer + r, size, 0, mRiderUserData);
            }

            if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
            {
                return result;
            }

            mNextPositionDisplay = mNextPosition;
            mNextPosition += size;

            if (!size)
            {
                result = FMOD_ERR_FILE_EOF;
            }
            if (result == FMOD_ERR_FILE_EOF)
            {
                break;
            }
        }
        /*
            Buffered read - single or double buffered.
        */
        else
        {
            result = checkBufferedStatus();
            if (result == FMOD_ERR_FILE_EOF && !(mBlockSize == mBufferSize && (int)mLength == -1))
            {
                result = FMOD_OK;           /* We dont care about the EOF for the flip status, we determine EOF ourselves. */
            }
            if (result != FMOD_OK)
            {
                break;
            }

            if (size > (mBlockSize - (mBufferPos % mBlockSize)))
            {
                size = mBlockSize - (mBufferPos % mBlockSize);
            }

            FMOD_memcpy((char *)buffer + r, (char *)mBuffer + mBufferPos, size);

            mBufferPos += size;
            if (mBufferPos >= mBufferSize)
            {
                mBufferPos = 0;
            }
        }

        mCurrentPosition += size;
        r += size;
        bytesleft -= size;
    }

    if (oldsize == 2)
    {
        unsigned short *wptr = (unsigned short *)buffer;
        unsigned int i;

        for (i=0; i < (r / oldsize); i++)
        {
            if (mBigEndian)
            {
                wptr[i] = FMOD_SWAPENDIAN_WORD(wptr[i]);
            }
        }
    }
    else if (oldsize == 4)
    {
        unsigned int *dptr = (unsigned int *)buffer;
        unsigned int i;

        for (i=0; i < (r / oldsize); i++)
        {
            if (mBigEndian)
            {
                dptr[i] = FMOD_SWAPENDIAN_DWORD(dptr[i]);
            }
        }
    }

    r /= oldsize;

    if (mEncryptionKeyLength)
    {
        unsigned char *val = (unsigned char *)buffer;
        unsigned int count;

        for (count = 0; count < r; count++)
        {
            FMOD_DECRYPT(val[count], mEncryptionKey[mEncryptionKeyIndex]);

            mEncryptionKeyIndex++;
            if (mEncryptionKeyIndex >= mEncryptionKeyLength)
            {
                mEncryptionKeyIndex = 0;
            }
        }
    }

    if (rd)
    {
        *rd = r;
    }

    if (result == FMOD_OK && eof)
    {
        result = FMOD_ERR_FILE_EOF;
    }

    return result;
}

#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: b03a8b92 auto
// OFFSET: 0x008E77C0
FMOD_RESULT File::getByte(unsigned char *val)
{
    FMOD_RESULT result;
    unsigned int rd;
    unsigned char temp;

	result = read(&temp, 1, 1, &rd);

    if (val)
    {
        *val = temp;
    }

    return result;
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
// VERIFY: c623c72f auto
// OFFSET: 0x008E7760
FMOD_RESULT File::getByte(unsigned short *val)
{
    FMOD_RESULT result;
    unsigned int rd;
    unsigned char temp;

	result = read(&temp, 1, 1, &rd);

    if (val)
    {
        *val = temp;
    }

    return result;
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
// VERIFY: 57cdaa09 auto
// OFFSET: 0x008E7790
FMOD_RESULT File::getByte(unsigned int *val)
{
    FMOD_RESULT result;
    unsigned int rd;
    unsigned char temp;

	result = read(&temp, 1, 1, &rd);
    
    if (val)
    {
        *val = temp;
    }

    return result;
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
FMOD_RESULT File::getByte(signed char *val)
{
    FMOD_RESULT result;
    unsigned int rd;
    signed char temp;

	result = read(&temp, 1, 1, &rd);

    if (val)
    {
        *val = temp;
    }

    return result;
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
FMOD_RESULT File::getByte(signed short *val)
{
    FMOD_RESULT result;
    unsigned int rd;
    signed char temp;

	result = read(&temp, 1, 1, &rd);

    if (val)
    {
        *val = temp;
    }

    return result;
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
// VERIFY: a4ec5777 auto
// OFFSET: 0x008E77F0
FMOD_RESULT File::getByte(signed int *val)
{
    FMOD_RESULT result;
    unsigned int rd;
    signed char temp;

	result = read(&temp, 1, 1, &rd);
    
    if (val)
    {
        *val = temp;
    }

    return result;
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
// VERIFY: 132daf13 auto
// OFFSET: 0x008E7820
FMOD_RESULT File::getWord(unsigned short *val)
{
    FMOD_RESULT result;
    unsigned int rd;
    unsigned short temp;

	result = read(&temp, 2, 1, &rd);

    if (val)
    {
        *val = temp;
    }

    return result;
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
// VERIFY: f2f7e22c auto
// OFFSET: 0x008E7850
FMOD_RESULT File::getWord(unsigned int *val)
{
    FMOD_RESULT result;
    unsigned int rd;
    unsigned short temp;

	result = read(&temp, 2, 1, &rd);

    if (val)
    {
        *val = temp;
    }

    return result;
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
FMOD_RESULT File::getWord(signed short *val)
{
    FMOD_RESULT result;
    unsigned int rd;
    signed short temp;

	result = read(&temp, 2, 1, &rd);

    if (val)
    {
        *val = temp;
    }

    return result;
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
// VERIFY: 7f132b14 auto
// OFFSET: 0x008E7880
FMOD_RESULT File::getWord(signed int *val)
{
    FMOD_RESULT result;
    unsigned int rd;
    signed short temp;

	result = read(&temp, 2, 1, &rd);

    if (val)
    {
        *val = temp;
    }

    return result;
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
// VERIFY: 21eae73e auto
// OFFSET: 0x008E78B0
FMOD_RESULT File::getDword(unsigned int *val)
{    
    FMOD_RESULT result;
    unsigned int rd;
    unsigned int temp;

	result = read(&temp, 4, 1, &rd);

    if (val)
    {
        *val = temp;
    }

    return result;
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
FMOD_RESULT File::getDword(signed int *val)
{    
    FMOD_RESULT result;
    unsigned int rd;
    signed int temp;

	result = read(&temp, 4, 1, &rd);

    if (val)
    {
        *val = temp;
    }

    return result;
}

#endif // !PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: e3dab85b manual
// OFFSET: 0x008E78E0
FMOD_RESULT F_API File::seek(int pos, int mode)
{
    FMOD_RESULT  result = FMOD_OK;
    unsigned int newpos = 0;

	if (mode == SEEK_SET)
    {
        newpos = mStartOffset + pos;
    }
	else if (mode == SEEK_CUR)
    {
        newpos = mCurrentPosition + pos;
    }
	else if (mode == SEEK_END)
    {
        newpos = mStartOffset + mLength + pos;
    }
    else
    {
		return FMOD_ERR_INVALID_PARAM;
    }

    if (newpos > mStartOffset + mLength)
    {
        if (pos >= 0)
        {
            newpos = mStartOffset + mLength;
        }
        else
        {
            newpos = 0;
        }
    }

    /*
        If no read has been done and not seekable and we are trying to seek beyond the first (future) read size, return an error.
    */
    if (!mNextPosition && !mSeekable && newpos >= mBlockSize)
    {
        return FMOD_ERR_FILE_COULDNOTSEEK;
    }

    /*
        Single buffered, not seekable, dont allow seeks outside the current buffer
    */
    if ((mBlockSize == mBufferSize) && !mSeekable && (mNextPosition >= mBlockSize))
    {
        if (mNextPosition && newpos < mNextPosition - mBlockSize)
        {
            return FMOD_ERR_FILE_COULDNOTSEEK;
        }
        if (newpos >= mNextPosition + mBlockSize)
        {
            return FMOD_ERR_FILE_COULDNOTSEEK;
        }
    }

    mCurrentPosition = newpos;

    if (mEncryptionKeyLength)
    {
        mEncryptionKeyIndex = mCurrentPosition % mEncryptionKeyLength;
    }

    if (mBufferSize)
    {
        mBufferPos = mCurrentPosition % mBufferSize;    /* We are seeking resetting to the start of the WHOLE buffer */
    }
    else
    {
        result = reallySeek(newpos);

        if (mSystem && mSystem->mSeekCallback)
        {
            mSystem->mSeekCallback(mRiderHandle, newpos, mRiderUserData);
        }
    }

    return result;
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
// VERIFY: b3ed7065 auto
// OFFSET: 0x008E7A20
FMOD_RESULT F_API File::tell(unsigned int *pos)
{
    FMOD_RESULT result = FMOD_OK;

    if (!pos)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *pos = mCurrentPosition;
    *pos -= mStartOffset;

	return result;
}

#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: d1354aec auto
// OFFSET: 0x008E7A50
FMOD_RESULT File::setStartOffset(unsigned int offset)
{
    mStartOffset = offset;

    mLength = mLengthOriginal;

    if (mStartOffset + mLength > mFileSize)
    {
        mLength = mFileSize - mStartOffset;
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
// VERIFY: fec49d12 auto
// OFFSET: 0x008E7A80
FMOD_RESULT File::getStartOffset(unsigned int *offset)
{
    if (!offset)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *offset = mStartOffset;

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
// VERIFY: 69f93eae manual
// OFFSET: 0x008E7EE0
FMOD_RESULT File::enableDoubleBuffer(unsigned int sizebytes)
{
    FMOD_RESULT result;
    unsigned int oldblocksize;

    if (!mBlockSize)
    {
        return FMOD_OK;
    }

    result = FMOD_OS_Semaphore_Create(&mSema);
    if (result != FMOD_OK)
    {
        return result;
    }
    FMOD_OS_Semaphore_Signal(mSema);    /* Put it up to 1 */

    if (sizebytes < FILE_SECTORSIZE)
    {
        sizebytes = FILE_SECTORSIZE;
    }
    if (sizebytes < mBlockSize)
    {
        sizebytes = mBlockSize;     /* Can't go smaller, we can only go bigger */
    }

    oldblocksize = mBlockSize;

    mBlockSize = sizebytes;
    mBlockSize /= oldblocksize;
    mBlockSize *= oldblocksize;

    mBufferSkip = oldblocksize;     /* This is for the next file flip, it will skip what we already have in the older buffer */
    mBlockOffset = 0;
    mNextPosition = mBlockSize;
    mNextPositionDisplay = mNextPosition;

    mBufferSize = mBlockSize;
    mBufferSize *= 2;              /* Make it a double buffer */

    mBuffer = FMOD_Memory_ReAlloc(mBuffer, mBufferSize);
    if (!mBuffer)
    {
        return FMOD_ERR_MEMORY;
    }

    result = getFileThread();
    if (result != FMOD_OK)
    {
        return result;
    }

    FMOD_OS_CriticalSection_Enter(mFileThread->mFileListCrit);
    addAfter(&mFileThread->mFileListHead);
    FMOD_OS_CriticalSection_Leave(mFileThread->mFileListCrit);

    result = checkBufferedStatus();
    if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
    {
        return result;
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
// VERIFY: b0aaccd0 auto
// OFFSET: 0x008E7AB0
FMOD_RESULT File::getName(char **name)
{
    if (name)
    {
        *name = mName;
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
// VERIFY: 54e01406 auto
// OFFSET: 0x008E7AD0
FMOD_RESULT File::setName(char *name)
{
    if (!name)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_strncpy(mName, name, FMOD_STRING_MAXNAMELEN);
    mName[FMOD_STRING_MAXNAMELEN - 1] = 0; //null terminator

    return FMOD_OK;
}

#endif // !PLATFORM_PS3_SPU





/*
[API]
[
	[DESCRIPTION]
	Callback for opening a file.  

	[PARAMETERS]
	'name'	    This is the filename passed in by the user.  You may treat this as you like.
    'unicode'   Tells the callback if the string being passed in is a double byte unicode string or not.  You may have to support this unless you know the target application will not support unicode.
    'filesize'  The size of the file to be passed back to fmod, in bytes.
    'handle'    This is to store a handle generated by the user.  This will be the handle that gets passed into the other callbacks.  Optional but may be needed.
    'userdata'  This is to store userdata to be passed into the other callbacks.  Optional.
 
	[RETURN_VALUE]

	[REMARKS]
    Return the appropriate error code such as FMOD_ERR_FILE_NOTFOUND if the file fails to open.
    If the callback is from System::attachFileSystem, then the return value is ignored.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::setFileSystem
    System::attachFileSystem
    FMOD_FILE_CLOSECALLBACK
    FMOD_FILE_READCALLBACK
    FMOD_FILE_SEEKCALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_FILE_OPENCALLBACK(const char *name, int unicode, unsigned int *filesize, void **handle, void **userdata);
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/

/*
[API]
[
	[DESCRIPTION]
    Calback for closing a file.

	[PARAMETERS]
	'handle'    This is the handle returned from the open callback to use for your own file routines.
    'userdata'  Userdata initialized in the FMOD_FILE_OPENCALLBACK.

	[RETURN_VALUE]

	[REMARKS]
	Close any user created file handle and perform any cleanup nescessary for the file here.
    If the callback is from System::attachFileSystem, then the return value is ignored.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::setFileSystem
    System::attachFileSystem
    FMOD_FILE_OPENCALLBACK
    FMOD_FILE_READCALLBACK
    FMOD_FILE_SEEKCALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_FILE_CLOSECALLBACK(void *handle, void *userdata);
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/

/*
[API]
[
	[DESCRIPTION]
    Callback for reading from a file.

	[PARAMETERS]
	'handle'    This is the handle you returned from the open callback to use for your own file routines.
	'buffer'	The buffer to read your data into.
	'sizebytes' The number of bytes to read.
    'bytesread' The number of bytes successfully read.
    'userdata'  Userdata initialized in the FMOD_FILE_OPENCALLBACK.

	[RETURN_VALUE]

	[REMARKS]
    If the callback is from System::attachFileSystem, then the return value is ignored.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::setFileSystem
    System::attachFileSystem
    FMOD_FILE_OPENCALLBACK
    FMOD_FILE_CLOSECALLBACK
    FMOD_FILE_SEEKCALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_FILE_READCALLBACK(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata);
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/

/*
[API]
[
	[DESCRIPTION]
    Callback for seeking within a file.

	[PARAMETERS]
	'handle'    This is the handle returned from the open callback to use for your own file routines.
	'pos'       This is the position or offset to seek to in the file in bytes.
    'userdata'  Data initialized in the FMOD_FILE_OPENCALLBACK.

	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::setFileSystem
    FMOD_FILE_OPENCALLBACK
    FMOD_FILE_CLOSECALLBACK
    FMOD_FILE_READCALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_FILE_SEEKCALLBACK(void *handle, unsigned int pos, void *userdata);
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/

}
