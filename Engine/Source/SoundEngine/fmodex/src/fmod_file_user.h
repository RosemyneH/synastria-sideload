#ifndef _FMOD_FILE_USER_H
#define _FMOD_FILE_USER_H

#include "fmod_settings.h"
#include "fmod_file.h"

namespace FMOD
{
    class UserFile : public File
    {
        DECLARE_MEMORYTRACKER

      private:

        FMOD_FILE_OPENCALLBACK   mOpenCallback;
        FMOD_FILE_CLOSECALLBACK  mCloseCallback;
        FMOD_FILE_READCALLBACK   mReadCallback;
        FMOD_FILE_SEEKCALLBACK   mSeekCallback;

        void    *mHandle;
        void    *mUserData;

      public:

        static FMOD_FILE_OPENCALLBACK  gOpenCallback;
        static FMOD_FILE_CLOSECALLBACK gCloseCallback;
        static FMOD_FILE_READCALLBACK  gReadCallback;
        static FMOD_FILE_SEEKCALLBACK  gSeekCallback;
        
// VERIFY: 58d56a3d auto
        // OFFSET: 0x008D2770
        UserFile() 
        {
            mHandle = 0;
            mUserData = 0;
            mOpenCallback = 0;
            mCloseCallback = 0;
            mReadCallback = 0;
            mSeekCallback = 0;
            mDeviceType = DEVICE_USER; 
        }

        FMOD_RESULT reallyOpen(const char *name_or_data, unsigned int *filesize);
        FMOD_RESULT reallyClose();
        FMOD_RESULT reallyRead(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT reallySeek(unsigned int pos);

        // OFFSET: 0x008D27B0
        FMOD_RESULT setUserCallbacks(FMOD_FILE_OPENCALLBACK open, FMOD_FILE_CLOSECALLBACK close, FMOD_FILE_READCALLBACK read, FMOD_FILE_SEEKCALLBACK seek)
        {
            if (!open || !close || !read || !seek)
            {
                open = 0;
                close = 0;
                read = 0;
                seek = 0;
            }

            mOpenCallback  = open;
            mCloseCallback = close;
            mReadCallback  = read;
            mSeekCallback  = seek;

            return FMOD_OK;
        }
   };
}

#endif


