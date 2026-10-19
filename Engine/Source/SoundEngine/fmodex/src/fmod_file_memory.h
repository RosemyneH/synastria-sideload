#ifndef _FMOD_FILE_MEMORY_H
#define _FMOD_FILE_MEMORY_H

#include "fmod_settings.h"
#include "fmod_file.h"

namespace FMOD
{
    class MemoryFile : public File
    {
        DECLARE_MEMORYTRACKER

      public:

        unsigned int  mPosition;
        void         *mMem;

// VERIFY: 64a77be1 auto
        // OFFSET: 0x008D26A0
        MemoryFile() { mDeviceType = DEVICE_MEMORY; }

        FMOD_RESULT reallyOpen(const char *name_or_data, unsigned int *filesize);
        FMOD_RESULT reallyClose();
        FMOD_RESULT reallyRead(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT reallySeek(unsigned int pos);
   };
}

#endif


