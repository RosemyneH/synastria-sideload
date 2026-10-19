#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_CDDA

#include "fmod_debug.h"
#include "fmod_file_cdda.h"
#include "fmod_os_cdda.h"
#include "fmod_string.h"
#include "fmod_time.h"

#include <stdio.h>
#include <string.h>


namespace FMOD
{

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: 624ef696 manual
// OFFSET: 0x009219A0
CddaFile::CddaFile()
{
    mDevice           = 0;
    mLastTimeAccessed = 0;
    mGotUserToc       = false;
    mJitterCorrection = false;
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
// VERIFY: 3276bbcc auto
// OFFSET: 0x00921030
FMOD_RESULT CddaFile::init(bool force_aspi, bool jitter_correction)
{
    mJitterCorrection = jitter_correction;
    return FMOD_OS_CDDA_Init(force_aspi);
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
// VERIFY: f18536b7 auto
// OFFSET: 0x00921050
FMOD_RESULT CddaFile::shutDown()
{
    return FMOD_OS_CDDA_Shutdown();
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
// VERIFY: 1a54aeab auto
// OFFSET: 0x00921690
FMOD_RESULT CddaFile::reallyOpen(const char *name_or_data, unsigned int *filesize)
{
    int numtracks;
    FMOD_RESULT result = FMOD_OK;

    mReadBuf          = 0;
    mReadPtr          = 0;
    mReadSizebytes    = 0;
    mStartSector      = 0;
    mCurrentSector    = 0;
    mSectorsLeft      = 0;
    mSectorsInChunk   = 26;
    mJitterBuf        = 0;
    mJitterBufSectors = 7;
    mJitterBufEmpty   = true;
    mGotUserToc       = false;

    result = FMOD_OS_CDDA_OpenDevice((char *)name_or_data, &mDevice);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (!FMOD_OS_CDDA_TestUnitReady(mDevice))
    {
        FMOD_OS_CDDA_DEBUGPRINT("CddaFile::reallyOpen: Device not ready (Make sure there's a CD/DVD in the drive!)");
        return FMOD_ERR_CDDA_NODISC;
    }

    result = FMOD_OS_CDDA_ReadTocRaw(mDevice, &mDevice->toc);
    if (result != FMOD_OK)
    {
        result = FMOD_OS_CDDA_ReadToc(mDevice, &mDevice->toc);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

#ifdef FMOD_OS_CDDA_DUMPTOC
    {
        unsigned int i;
        for (i=0;i < mDevice->toc.num_tracks;i++)
        {
            unsigned int len = (((mDevice->toc.num_sectors[i] * SIZEOF_CDDA_SECTOR) >> 2) / 44100) * 1000;
            FMOD_OS_CDDA_DEBUGPRINT("Track %d.  0x%08lx %d:%d (%d)\n", mDevice->toc.track_number[i], mDevice->toc.start_sector[i], len / 1000 / 60, len / 1000 % 60, *((unsigned char *)&mDevice->toc.flags[i]));
        }
    }
#endif

    mReadBuf = (char *)FMOD_Memory_Alloc(mSectorsInChunk * SIZEOF_CDDA_SECTOR);
    if (!mReadBuf)
    {
        reallyClose();
        return FMOD_ERR_MEMORY;
    }

    FMOD_memset(mReadBuf, 0, mSectorsInChunk * SIZEOF_CDDA_SECTOR);

    if (mJitterCorrection)
    {
        mJitterBuf = (char *)FMOD_Memory_Alloc(SIZEOF_CDDA_SECTOR);
        if (!mJitterBuf)
        {
            reallyClose();
            return FMOD_ERR_MEMORY;
        }

        FMOD_memset(mJitterBuf, 0, SIZEOF_CDDA_SECTOR);
        mJitterBufEmpty = true;
    }

    result = getNumTracks(&numtracks);
    if (result != FMOD_OK)
    {
        return result;
    }
    if (numtracks < 1)
    {
        FMOD_OS_CDDA_DEBUGPRINT("CddaFile::reallyOpen: No audio tracks! (Please insert a CD/DVD that contains one or more audio tracks)\n");
        return FMOD_ERR_CDDA_NOAUDIO;
    }

    result = getTrackLength(0, filesize);
    if (result != FMOD_OK)
    {
        return result;
    }

    *filesize <<= 3;
    mDevice->usertoc.numtracks--;

    FMOD_OS_CDDA_DEBUGPRINT("CddaFile::reallyOpen: ok\n");

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
// VERIFY: 1a8d53e4 auto
// OFFSET: 0x00921060
FMOD_RESULT CddaFile::reallyClose()
{
    FMOD_RESULT result = FMOD_OK;

    if (mDevice)
    {
        result = FMOD_OS_CDDA_CloseDevice(mDevice);
        mDevice = 0;
    }

    if (mReadBuf)
    {
        FMOD_Memory_Free(mReadBuf);
        mReadBuf = 0;
    }

    if (mJitterBuf)
    {
        FMOD_Memory_Free(mJitterBuf);
        mJitterBuf = 0;
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
// VERIFY: d8a5805f auto
// OFFSET: 0x00921850
FMOD_RESULT CddaFile::reallyRead(void *buffer, unsigned int size, unsigned int *rd)
{
    int          bytestocopy, count;
    unsigned int sectors_to_read = 0;
    FMOD_RESULT  result = FMOD_OK;
#ifdef FMOD_OS_CDDA_DEBUG
    unsigned int starttime;
    FMOD_OS_Time_GetMs(&starttime);
#endif

    if (!mDevice)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *rd = 0;

    while (size)
    {
        if (!mReadSizebytes)
        {
            sectors_to_read = (mSectorsLeft < mSectorsInChunk) ? mSectorsLeft : mSectorsInChunk;
            if (!sectors_to_read)
            {
                *rd = 0;
                return FMOD_ERR_FILE_EOF;
            }

            for (count = 0; count < 10; count++)
            {
                result = FMOD_OS_CDDA_ReadSectors(mDevice, mReadBuf, mCurrentSector, sectors_to_read);
                if (result == FMOD_OK)
                {
                    break;
                }
                else
                {
                    FMOD_OS_Time_Sleep(1);
                }
            }

            FMOD_OS_Time_GetMs(&mLastTimeAccessed);

            if (count >= 10)
            {
                return FMOD_ERR_CDDA_READ;
            }

            if (mJitterCorrection)
            {
                doJitterCorrection(sectors_to_read);
            }
            else
            {
                mCurrentSector += sectors_to_read;
                mSectorsLeft   -= sectors_to_read;
                mReadPtr        = mReadBuf;
                mReadSizebytes  = sectors_to_read * SIZEOF_CDDA_SECTOR;
            }

#ifdef FMOD_OS_CDDA_DEBUG
            {
                unsigned int elapsed;
                FMOD_OS_Time_GetMs(&elapsed);
                elapsed -= starttime;
                FMOD_OS_CDDA_AddProfileData((int)elapsed, mReadSizebytes);
            }
#endif
        }

        bytestocopy = (mReadSizebytes < (unsigned int)size) ? mReadSizebytes : size;

        FMOD_memcpy((char *)buffer, mReadPtr, bytestocopy);

        *rd            += bytestocopy;
        mReadPtr       += bytestocopy;
        mReadSizebytes -= bytestocopy;

        buffer         = (void *)((char *)buffer + bytestocopy);
        size           -= bytestocopy;
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
// VERIFY: 1a04310c manual
// OFFSET: 0x009210F0
FMOD_RESULT CddaFile::doJitterCorrection(unsigned int sectors_to_read)
{
    if (!mJitterBufEmpty)
    {
        int offset, skip_bytes;
        int compare_size = ((sectors_to_read < mJitterBufSectors ? sectors_to_read : mJitterBufSectors) - 1) * SIZEOF_CDDA_SECTOR;
        int start_compare = compare_size >> 1;
        char *ptr_forward, *ptr_back;
        bool found = false;

        if ((compare_size <= 0) || (sectors_to_read <= (mJitterBufSectors >> 1)))
        {
            FMOD_memset(mReadBuf, 0, sectors_to_read * SIZEOF_CDDA_SECTOR);
            FMOD_memset(mJitterBuf, 0, SIZEOF_CDDA_SECTOR);
            mCurrentSector += sectors_to_read;
            mSectorsLeft   -= sectors_to_read;
            mReadPtr        = mReadBuf;
            mReadSizebytes  = sectors_to_read * SIZEOF_CDDA_SECTOR;

            return FMOD_OK;
        }

        ptr_forward = ptr_back = (mReadBuf + start_compare);
        skip_bytes = start_compare;

        for (offset=0;offset < start_compare;offset += 4, ptr_forward += 4, ptr_back -= 4)
        {
            if (!memcmp(ptr_forward, mJitterBuf, SIZEOF_CDDA_SECTOR))
            {
                skip_bytes = start_compare + offset + SIZEOF_CDDA_SECTOR;
                found = true;
                break;
            }

            if (!memcmp(ptr_back, mJitterBuf, SIZEOF_CDDA_SECTOR))
            {
                skip_bytes = start_compare - offset + SIZEOF_CDDA_SECTOR;
                found = true;
                break;
            }
        }

        if (!found)
        {
            /*
                Jitter error - couldn't match up this read to the last one - it's gonna sound bad...
            */
            skip_bytes = 0;
        }

        if ((unsigned int)sectors_to_read == mSectorsLeft)
        {
            mCurrentSector += sectors_to_read;
            mSectorsLeft   -= sectors_to_read;
        }
        else
        {
            mCurrentSector += (sectors_to_read - (mJitterBufSectors >> 1) - 1);
            mSectorsLeft   -= (sectors_to_read - (mJitterBufSectors >> 1) - 1);
        }
        mReadPtr       = mReadBuf + skip_bytes;
        mReadSizebytes = (sectors_to_read * SIZEOF_CDDA_SECTOR) - skip_bytes;
    }
    else
    {
        mCurrentSector += sectors_to_read;
        mSectorsLeft   -= sectors_to_read;
        mReadPtr        = mReadBuf;
        mReadSizebytes  = sectors_to_read  * SIZEOF_CDDA_SECTOR;
    }

    FMOD_memcpy(mJitterBuf, mReadBuf + ((sectors_to_read - 1) * SIZEOF_CDDA_SECTOR), SIZEOF_CDDA_SECTOR);
    mJitterBufEmpty = false;

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
// VERIFY: f253f445 auto
// OFFSET: 0x009213D0
FMOD_RESULT CddaFile::reallySeek(unsigned int pos)
{
    unsigned int num_sectors;

    pos /= SIZEOF_CDDA_SECTOR;

    num_sectors = (mCurrentSector - mStartSector) + mSectorsLeft;
    if (pos >= num_sectors)
    {
        return FMOD_ERR_INVALID_PARAM;
    }
    mCurrentSector = mStartSector + pos;
    mSectorsLeft   = num_sectors  - pos;

    FMOD_memset(mReadBuf, 0, mSectorsInChunk * SIZEOF_CDDA_SECTOR);
    mReadPtr        = 0;
    mReadSizebytes  = 0;
    mJitterBufEmpty = true;

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
// VERIFY: 2d073c8c auto
// OFFSET: 0x00921460
FMOD_RESULT CddaFile::getNumTracks(int *numtracks)
{
    if (!numtracks || !mDevice)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *numtracks = mDevice->toc.num_tracks - 1;
    if (*numtracks < 0)
    {
        *numtracks = 0;
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
// VERIFY: d1b03d2c auto
// OFFSET: 0x009214A0
FMOD_RESULT CddaFile::getTrackLength(unsigned int track, unsigned int *tracklength)
{
    if (!tracklength || !mDevice || (track >= (mDevice->toc.num_tracks - 1)))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *tracklength = mDevice->toc.num_sectors[track] * SIZEOF_CDDA_SECTOR;

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
// VERIFY: b8c7a280 manual
// OFFSET: 0x009214F0
FMOD_RESULT CddaFile::openTrack(unsigned int track)
{
    unsigned int starttime, thistime;

    if (track >= (mDevice->toc.num_tracks - 1))       /* -1 for leadout track */
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    mStartSector    = mDevice->toc.start_sector[track];
    mCurrentSector  = mStartSector;
    mSectorsLeft    = mDevice->toc.num_sectors[track];
    mLength         = mSectorsLeft * SIZEOF_CDDA_SECTOR;
    mReadPtr        = 0;
    mReadSizebytes  = 0;
    mJitterBufEmpty = true;
    FMOD_memset(mReadBuf, 0, mSectorsInChunk * SIZEOF_CDDA_SECTOR);

    FMOD_OS_Time_GetMs(&thistime);
    if ((thistime - mLastTimeAccessed) > (FMOD_CDDA_SPINDOWN_TIME * 1000))
    {
        FMOD_OS_CDDA_SetSpeed(mDevice, 4);

        FMOD_OS_Time_GetMs(&starttime);
        for (;;)
        {
            FMOD_OS_Time_GetMs(&thistime);
            if ((thistime - starttime) > (FMOD_CDDA_SPINUP_TIME * 1000))
            {
                break;
            }

            FMOD_OS_CDDA_ReadSectors(mDevice, mReadBuf, mStartSector, 1);
            FMOD_OS_Time_Sleep(20);
        }

        FMOD_OS_Time_GetMs(&mLastTimeAccessed);
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
// VERIFY: d7af3b4d auto
// OFFSET: 0x00921620
FMOD_RESULT CddaFile::getMetadata(Metadata **metadata)
{
    if (!mDevice || !metadata)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (mGotUserToc)
    {
        return FMOD_ERR_TAGNOTFOUND;
    }

    mMetadata.addTag(FMOD_TAGTYPE_FMOD, "CDTOC", &mDevice->usertoc, sizeof(FMOD_CDTOC), FMOD_TAGDATATYPE_CDTOC, true);
    mGotUserToc = true;
    *metadata = &mMetadata;

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

}

#endif