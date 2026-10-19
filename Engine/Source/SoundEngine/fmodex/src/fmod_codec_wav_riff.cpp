#include "fmod_settings.h"

#if defined(FMOD_SUPPORT_WAV) || defined(FMOD_SUPPORT_MPEG)

#include "fmod_codec_wav.h"
#include "fmod_file.h"
#include "fmod_syncpoint.h"

namespace FMOD
{

/*
[
    [DESCRIPTION]

    [PARAMETERS]
 
    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
// VERIFY: a65f7065 manual
// OFFSET: 0x00932440
FMOD_RESULT CodecWav::parseChunk(unsigned int chunksize)
{
    unsigned int    offset, fileoffset;
    FMOD_RESULT     result;
    bool            done = false;

    result = mFile->tell(&fileoffset);
    if (result != FMOD_OK)
    {
        return result;
    }

    offset = 4;
    fileoffset -= sizeof(WAVE_CHUNK);

    /*
        Decode chunks
    */
    do 
    {
        WAVE_CHUNK chunk;

        result = mFile->seek(fileoffset + sizeof(WAVE_CHUNK), SEEK_SET);
        if (result != FMOD_OK)
        {
            break;
        }

        result = mFile->read(&chunk, 1, sizeof(WAVE_CHUNK), 0);
        if (result != FMOD_OK)
        {
            break;
        }

        #ifdef PLATFORM_ENDIAN_BIG
        chunk.size = FMOD_SWAPENDIAN_DWORD(chunk.size);
        #endif

        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecWav::parseRIFF","chunk : id %c%c%c%c size %d\n", chunk.id[0],chunk.id[1],chunk.id[2],chunk.id[3], chunk.size));

        /*
            FORMAT CHUNK
        */
        if (!FMOD_strncmp((const char *)chunk.id, "fmt ", 4))
        {
            mSrcFormat = (WAVE_FORMATEXTENSIBLE *)FMOD_Memory_Calloc(chunk.size < sizeof(WAVE_FORMATEXTENSIBLE) ? sizeof(WAVE_FORMATEXTENSIBLE) : chunk.size);
            if (!mSrcFormat)
            {
                return FMOD_ERR_MEMORY;
            }

            result = mFile->read(mSrcFormat, 1, chunk.size, 0);
            if (result != FMOD_OK)
            {
                break;
            }

            #ifdef PLATFORM_ENDIAN_BIG
            mSrcFormat->Format.wFormatTag       = FMOD_SWAPENDIAN_WORD(mSrcFormat->Format.wFormatTag);      
            mSrcFormat->Format.nChannels        = FMOD_SWAPENDIAN_WORD(mSrcFormat->Format.nChannels);       
            mSrcFormat->Format.nSamplesPerSec   = FMOD_SWAPENDIAN_DWORD(mSrcFormat->Format.nSamplesPerSec);  
            mSrcFormat->Format.nAvgBytesPerSec  = FMOD_SWAPENDIAN_DWORD(mSrcFormat->Format.nAvgBytesPerSec); 
            mSrcFormat->Format.nBlockAlign      = FMOD_SWAPENDIAN_WORD(mSrcFormat->Format.nBlockAlign);     
            mSrcFormat->Format.wBitsPerSample   = FMOD_SWAPENDIAN_WORD(mSrcFormat->Format.wBitsPerSample);
            mSrcFormat->Format.cbSize           = FMOD_SWAPENDIAN_WORD(mSrcFormat->Format.cbSize);

            mSrcFormat->Samples.wValidBitsPerSample = FMOD_SWAPENDIAN_WORD(mSrcFormat->Samples.wValidBitsPerSample);
            mSrcFormat->dwChannelMask           = FMOD_SWAPENDIAN_DWORD(mSrcFormat->dwChannelMask);
            
            if (mSrcFormat->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
            {
                mSrcFormat->SubFormat.Data1 = FMOD_SWAPENDIAN_DWORD(mSrcFormat->SubFormat.Data1);
                mSrcFormat->SubFormat.Data2 = FMOD_SWAPENDIAN_WORD(mSrcFormat->SubFormat.Data2);
                mSrcFormat->SubFormat.Data3 = FMOD_SWAPENDIAN_WORD(mSrcFormat->SubFormat.Data3);
            }
            
            #endif

        }

        /*
            CUE CHUNK
        */
        else if (!FMOD_strncmp((const char *)chunk.id, "cue ", 4))
        {
            result = mFile->read(&mNumSyncPoints, 4, 1, 0);
            if (mNumSyncPoints)
            {
                int count;

                if (mSyncPoint)
                {
                    FMOD_Memory_Free(mSyncPoint);
                }

                mSyncPoint = (SyncPoint *)FMOD_Memory_Calloc(mNumSyncPoints * sizeof(SyncPoint));
                if (!mSyncPoint)
                {
                    return FMOD_ERR_MEMORY;
                }

                for (count=0; count < mNumSyncPoints; count++)
                {
                    WAVE_CUEPOINT    cue;
                    SyncPoint  *point = &mSyncPoint[count];

                    result = mFile->read(&cue, 1, sizeof(WAVE_CUEPOINT), 0);
                    if (result != FMOD_OK)
                    {
                        break;
                    }


                    #ifdef PLATFORM_ENDIAN_BIG
                    cue.dwSampleOffset = FMOD_SWAPENDIAN_DWORD(cue.dwSampleOffset);
                    cue.dwIdentifier = FMOD_SWAPENDIAN_DWORD(cue.dwIdentifier);
                    #endif

                    point->mOffset = cue.dwSampleOffset;
                    point->mIndex = cue.dwIdentifier;
                }
            }
        }

        /*
            FACT CHUNK
        */
        else if (!FMOD_strncmp((const char *)chunk.id, "fact", 4))
        {
            unsigned int fact;

            result = mFile->read(&fact, 4, 1, 0);
            if (result != FMOD_OK)
            {
                break;
            }

            waveformat[0].lengthpcm = fact; 
        }

        /*
            LIST CHUNK
        */
        else if (!FMOD_strncmp((const char *)chunk.id, "LIST", 4))
        {
            char listid[4];

            result = mFile->read(listid, 1, 4, 0);
            if (result != FMOD_OK)
            {
                break;
            }

            result = parseChunk(chunk.size);
            if (result != FMOD_OK)
            {
                break;
            }
        }

        /*
            LABL CHUNK
        */
        else if (!FMOD_strncmp((const char *)chunk.id, "labl", 4))
        {
            if (mSyncPoint)
            {
                int id, count;

                result = mFile->read(&id, 4, 1, 0);
                if (result != FMOD_OK)
                {
                    break;
                }

                for (count = 0; count < mNumSyncPoints; count++)
                {
                    SyncPoint *p = &mSyncPoint[count];
                    
                    if (p->mIndex == id)
                    {
                        result = mFile->read(p->mName, 1, chunk.size-4);
                        if (result != FMOD_OK)
                        {
                            break;
                        }
                        break;
                    }
                }
            }
        }

        /*
            SAMPLER CHUNK
        */
        else if (!FMOD_strncmp((const char *)chunk.id, "smpl", 4))
        {
            WAVE_SMPLHEADER smpl;
            unsigned int rd;

            result = mFile->read(&smpl, 1, sizeof(WAVE_SMPLHEADER), &rd);
            if (result != FMOD_OK)
            {
                break;
            }

            if (rd == sizeof(WAVE_SMPLHEADER))
            {
                #ifdef PLATFORM_ENDIAN_BIG
                smpl.Manufacturer   = FMOD_SWAPENDIAN_DWORD(smpl.Manufacturer);
                smpl.Product        = FMOD_SWAPENDIAN_DWORD(smpl.Product);
                smpl.SamplePeriod   = FMOD_SWAPENDIAN_DWORD(smpl.SamplePeriod);
                smpl.Note           = FMOD_SWAPENDIAN_DWORD(smpl.Note);
                smpl.FineTune       = FMOD_SWAPENDIAN_DWORD(smpl.FineTune);
                smpl.SMPTEFormat    = FMOD_SWAPENDIAN_DWORD(smpl.SMPTEFormat);
                smpl.SMPTEOffset    = FMOD_SWAPENDIAN_DWORD(smpl.SMPTEOffset);
                smpl.Loops          = FMOD_SWAPENDIAN_DWORD(smpl.Loops);
                smpl.SamplerData    = FMOD_SWAPENDIAN_DWORD(smpl.SamplerData);
                
                smpl.Loop.Identifier = FMOD_SWAPENDIAN_DWORD(smpl.Loop.Identifier);
                smpl.Loop.Type       = FMOD_SWAPENDIAN_DWORD(smpl.Loop.Type);
                smpl.Loop.Start      = FMOD_SWAPENDIAN_DWORD(smpl.Loop.Start);
                smpl.Loop.End        = FMOD_SWAPENDIAN_DWORD(smpl.Loop.End);
                smpl.Loop.Fraction   = FMOD_SWAPENDIAN_DWORD(smpl.Loop.Fraction);
                smpl.Loop.Count      = FMOD_SWAPENDIAN_DWORD(smpl.Loop.Count);       
                #endif

                if (smpl.Loops)
                {
                    mLoopPoints[0] = smpl.Loop.Start;
                    mLoopPoints[1] = smpl.Loop.End;
                }
            }
        }


        /*
            DATA CHUNK
        */
        else if (!FMOD_strncmp((const char *)chunk.id, "data", 4))
        {
            if (mSrcDataOffset == (unsigned int)-1)
            {
                waveformat[0].lengthbytes = chunk.size;

                result = mFile->tell(&mSrcDataOffset);
                if (result != FMOD_OK)
                {
                    break;
                }
            }

            if (mFile->mSeekable)
            {
                result = mFile->seek(chunk.size, SEEK_CUR);
                if (result != FMOD_OK)
                {
                    break;
                }
            }
            else
            {
                done = true;
            }
        }
        else
        {
            mFile->seek(chunk.size, SEEK_CUR);
            if (result != FMOD_OK)
            {
                break;
            }
        }

        offset     += (chunk.size+sizeof(WAVE_CHUNK));
        fileoffset += (chunk.size+sizeof(WAVE_CHUNK));

        if (chunk.size & 1)
        {
            offset++;
            fileoffset++;
        }

        if (chunk.size < 0)
        {
            break;
        }

        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecWav::parseRIFF", "offset = %d / %d\n", offset, chunksize));

    } while (offset < chunksize && offset > 0 && !done);


    /*
        There is a data chunk, but we had a truncated wav.. we will still load what's there.
    */
    if (result == FMOD_ERR_FILE_EOF)
    {
        result = FMOD_OK;
    }


    return result;
}

}

#endif

