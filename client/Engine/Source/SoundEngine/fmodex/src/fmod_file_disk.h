#ifndef _FMOD_FILE_DISK_H
#define _FMOD_FILE_DISK_H

#include "fmod_settings.h"
#include "fmod_file.h"

namespace FMOD
{
    class DiskFile : public File
    {
        DECLARE_MEMORYTRACKER

      private:

	    void *mHandle;

      public:

        // OFFSET: 0x008D2750
        DiskFile() { mDeviceType = DEVICE_DISK; }

        FMOD_RESULT reallyOpen(const char *name_or_data, unsigned int *filesize);
        FMOD_RESULT reallyClose();
        FMOD_RESULT reallyRead(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT reallySeek(unsigned int pos);
        FMOD_RESULT reallyCancel();
   };
}

#endif


