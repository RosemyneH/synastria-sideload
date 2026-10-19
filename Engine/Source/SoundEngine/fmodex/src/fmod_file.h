#ifndef _FMOD_FILE_H
#define _FMOD_FILE_H

#include "fmod_settings.h"

#include "fmod_linkedlist.h"
#include "fmod_memory.h"
#include "fmod_metadata.h"
#include "fmod_os_misc.h"
#include "fmod_time.h"

//#ifndef PLATFORM_PS3_SPU
    #include "fmod_thread.h"
//#endif

#define FMOD_FILE_QUEUEREAD   0
#define FMOD_FILE_QUEUESEEK   1
#define FMOD_FILE_QUEUEMAX    64            /* Must be a power of 2 */

#ifdef PLATFORM_PS2
    #ifndef SEEK_SET
    #define SEEK_SET	(0)
    #endif
    #ifndef SEEK_CUR
    #define SEEK_CUR	(1)
    #endif
    #ifndef SEEK_END
    #define SEEK_END	(2)
    #endif
#endif

#define PS3_DMAFILE_BLOCKSIZE   256

namespace FMOD
{
    class File;

    const unsigned int FILE_SECTORSIZE  = (2*1024);  /* default.  CD sector size */

    typedef struct
    {
        int                 mType;
        File               *mHandle;
        void               *mReadBuff;
        int                 mReadSize;
        int                 mReadCount;
        int                 mSeekPos;
        signed char         mSeekMode;
    } FileQueue;

    enum
    {
        DEVICE_USER = 0,
        DEVICE_MEMORY,
        DEVICE_RECORD,
        DEVICE_NET,
        DEVICE_CDDA,
        DEVICE_DISK
    };

    #if defined(PLATFORM_WII) || defined(PLATFORM_GC)
        #define FMOD_FILE_ALIGN 32
    #else
        #define FMOD_FILE_ALIGN 0
    #endif

    class FileThread : public LinkedListNode  /* This linked list node entry is for File::gFileThreadHead */
    {
#ifndef PLATFORM_PS3_SPU
      public :

        Thread                   mThread;
        bool                     mThreadActive;
        LinkedListNode           mFileListHead;
        LinkedListNode          *mFileListCurrent;
        LinkedListNode          *mFileListNext;
        FMOD_OS_CRITICALSECTION *mFileListCrit;
        int                      mDeviceType;
        bool                     mOwned;

        FMOD_RESULT              threadFunc();

        FileThread();

        FMOD_RESULT     init(int devicetype, bool owned);
        FMOD_RESULT     release();
#endif
    };

    class File : public LinkedListNode      /* This linked list node entry is for FileThread::mHead */
    {
        DECLARE_MEMORYTRACKER

        friend class FileThread;

      protected:

	    unsigned int             mLength;
        unsigned int             mLengthOriginal;
        unsigned int             mFileSize;
        bool                     mBufferIsString;
        void                    *mRiderUserData;
        void                    *mRiderHandle;
        bool                     mUnicode;
        int                      mDeviceType;

      private:

		char                     mName[FMOD_STRING_MAXNAMELEN];
        char                     mEncryptionKey[32];
        int                      mEncryptionKeyLength;
        int                      mEncryptionKeyIndex;
        unsigned int             mBufferPos;
        unsigned int             mBufferSize;
        unsigned int             mBufferSkip;
        unsigned int             mBlockSize;
        unsigned int             mBlockOffset;
        unsigned int             mCurrentPosition;
        unsigned int             mNextPosition;
        unsigned int             mNextPositionDisplay;
        unsigned int             mStartOffset;
        bool                     mBigEndian;
        bool                     mBusy;
        bool                     mStarving;
        bool                     mExit;
        int                      mPercentBuffered;
        FMOD_RESULT              mAsyncError;
        bool                     mFlipping;
        FileThread              *mFileThread;
        FMOD_OS_SEMAPHORE       *mSema;

        FMOD_RESULT              flip(bool frommainthread);
        FMOD_RESULT              checkBufferedStatus();
        FMOD_RESULT              seekAndReset();
        FMOD_RESULT              getFileThread();

      public:

        bool                     mSeekable;
        SystemI                 *mSystem;
        void                    *mBuffer;

        static LinkedListNode           gFileThreadHead;
        static FMOD_OS_CRITICALSECTION *gFileCrit;
        static bool                     gUsesUserCallbacks;
        static int                      gBufferSize;

        File();

        FMOD_RESULT             init(unsigned int filesize, int blocksize);
        FMOD_RESULT             getByte(unsigned char  *byte = 0);
        FMOD_RESULT             getByte(unsigned short *byte);
        FMOD_RESULT             getByte(unsigned int   *byte);
        FMOD_RESULT             getByte(signed char    *byte);
        FMOD_RESULT             getByte(signed short   *byte);
        FMOD_RESULT             getByte(signed int     *byte);
        FMOD_RESULT             getWord(unsigned short *word = 0);
        FMOD_RESULT             getWord(unsigned int   *word);
        FMOD_RESULT             getWord(signed short   *word);
        FMOD_RESULT             getWord(signed int     *word);
        FMOD_RESULT             getDword(unsigned int  *dword = 0);
        FMOD_RESULT             getDword(signed int    *dword);
        FMOD_RESULT             setStartOffset(unsigned int offset);
        FMOD_RESULT             getStartOffset(unsigned int *offset);
        FMOD_RESULT             enableDoubleBuffer(unsigned int sizebytes);
        FMOD_RESULT             getName(char **name);
        FMOD_RESULT             setName(char *name);
        FMOD_RESULT             setBigEndian(bool bigendian) { mBigEndian = bigendian; return FMOD_OK; }
        FMOD_RESULT             isBusy(bool *isbusy, unsigned int *percent) { if (isbusy) *isbusy = mBusy; if (percent) *percent = mPercentBuffered; return FMOD_OK; }
        FMOD_RESULT             isStarving(bool *starving) { if (!starving) return FMOD_ERR_INVALID_PARAM; *starving = mStarving; return FMOD_OK; }
        bool                    readyForClose();

// VERIFY: 424eafe5 auto
        // OFFSET: 0x008D26C0
        virtual FMOD_RESULT     getMetadata(Metadata **metadata) { return FMOD_ERR_TAGNOTFOUND; }
// VERIFY: fceb7e48 auto
        // OFFSET: 0x008D26D0
        virtual FMOD_RESULT     getSize(unsigned int *size) { *size = mLength; return FMOD_OK; }
        virtual FMOD_RESULT     reallyOpen(const char *name_or_data, unsigned int *filesize) = 0;
        virtual FMOD_RESULT     reallyClose() = 0;
        virtual FMOD_RESULT     reallyRead(void *buffer, unsigned int size, unsigned int *read) = 0;
        virtual FMOD_RESULT     reallySeek(unsigned int pos) = 0;
        virtual FMOD_RESULT     reallyCancel() { return FMOD_OK; }

        FMOD_RESULT F_API       open(const char *name_or_data, unsigned int length, bool unicode = false, const char *encryptionkey = 0);
        FMOD_RESULT F_API       close();
        FMOD_RESULT F_API       cancel();
        FMOD_RESULT F_API       read(void *buffer, unsigned int size, unsigned int count, unsigned int *read = 0);
        FMOD_RESULT F_API       seek(int pos, int mode);
        FMOD_RESULT F_API       tell(unsigned int *pos);

      protected:

        friend class SystemI;

        static FMOD_RESULT shutDown();
   };
}

#endif


