#ifndef _FMOD_FILE_CDDA_H
#define _FMOD_FILE_CDDA_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_CDDA

#include "fmod_file.h"
#include "fmod_linkedlist.h"

namespace FMOD
{
    typedef struct FMOD_CDDA_DEVICE FMOD_CDDA_DEVICE;

    class CddaFile : public File
    {
        DECLARE_MEMORYTRACKER

      private:

        FMOD_CDDA_DEVICE   *mDevice;
        char               *mReadBuf;
        char               *mReadPtr;
        unsigned int        mReadSizebytes;
        unsigned int        mStartSector;
        unsigned int        mCurrentSector;
        unsigned int        mSectorsInChunk;
        unsigned int        mSectorsLeft;
        char               *mJitterBuf;
        unsigned int        mJitterBufSectors;
        bool                mJitterBufEmpty;
        bool                mJitterCorrection;
        unsigned int        mLastTimeAccessed;
        bool                mGotUserToc;
        Metadata            mMetadata;

        int                 mCurrentTrack;
        char              **mTrackFiles;
        void               *mTrackHandle;

      public:

        CddaFile();

        FMOD_RESULT         init(bool force_aspi, bool jitter_correction);

        FMOD_RESULT         reallyOpen(const char *name_or_data, unsigned int *filesize);
        FMOD_RESULT         reallyClose();
        FMOD_RESULT         reallyRead(void *buffer, unsigned int size, unsigned int *read);
        FMOD_RESULT         reallySeek(unsigned int pos);

        FMOD_RESULT         getNumTracks(int *numtracks);
        FMOD_RESULT         getTrackLength(unsigned int track, unsigned int *tracklength);
        FMOD_RESULT         openTrack(unsigned int track);
        FMOD_RESULT         doJitterCorrection(unsigned int sectors_to_read);

        FMOD_RESULT         getMetadata(Metadata **metadata);

      protected:

        friend class File;

        static FMOD_RESULT  shutDown();
   };
}

#endif

#endif


