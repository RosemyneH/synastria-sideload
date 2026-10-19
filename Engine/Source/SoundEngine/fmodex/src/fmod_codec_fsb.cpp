#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_FSB

#include "fmod.h"
#include "fmod_async.h"
#include "fmod_codec_fsb.h"
#include "fmod_debug.h"
#include "fmod_dsp_codec.h"
#include "fmod_file.h"
#include "fmod_metadata.h"
#include "fmod_outputi.h"
#include "fmod_soundi.h"
#include "fmod_speakermap.h"
#include "fmod_string.h"

#ifdef PLATFORM_PS3_SPU
    #include "fmod_common_spu.h"
    #include "fmod_spu_printf.h"
    #include "fmod_systemi_spu.h"
    #include <cell/dma.h>
#else
    #include "fmod_systemi.h"
#endif

#ifdef FMOD_SUPPORT_IMAADPCM
    #include "fmod_codec_wav_imaadpcm.h"
#endif

#ifdef FMOD_SUPPORT_GCADPCM
    #include "fmod_codec_gcadpcm.h"
#endif

#ifdef FMOD_SUPPORT_XMA
    #include "fmod_codec_xma.h"
#endif

#ifdef FMOD_SUPPORT_MPEG
    #include "fmod_codec_mpeg.h"
#endif

#ifdef FMOD_SUPPORT_VAG
    #include "fmod_codec_swvag.h"
#endif


namespace FMOD
{

#ifndef PLATFORM_PS3_SPU

FMOD_CODEC_DESCRIPTION_EX fsbcodec;


#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_CODEC_DESCRIPTION_EX * F_API FMODGetCodecDescriptionEx()
    {
        return CodecFSB::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


#ifdef FMOD_FSB_USEHEADERCACHE
CodecFSBCache CodecFSB::gCacheHead;
#endif


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: d89166f7 manual
// OFFSET: 0x00912EC0
FMOD_CODEC_DESCRIPTION_EX *CodecFSB::getDescriptionEx()
{
    FMOD_memset(&fsbcodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    fsbcodec.name          = "FMOD FSB Codec";
    fsbcodec.version       = 0x00010100;
    fsbcodec.timeunits     = FMOD_TIMEUNIT_PCM | FMOD_TIMEUNIT_RAWBYTES;
    fsbcodec.open          = &CodecFSB::openCallback;
    fsbcodec.close         = &CodecFSB::closeCallback;
    fsbcodec.read          = &CodecFSB::readCallback;
    fsbcodec.setposition   = &CodecFSB::setPositionCallback;
    fsbcodec.getposition   = &CodecFSB::getPositionCallback;
    fsbcodec.soundcreate   = &CodecFSB::soundcreateCallback;
    fsbcodec.getwaveformat = &CodecFSB::getWaveFormatCallback;
    fsbcodec.reset         = &CodecFSB::resetCallback;
    fsbcodec.canpoint      = &CodecFSB::canPointCallback;

    fsbcodec.mType         = FMOD_SOUND_TYPE_FSB;
    fsbcodec.mSize         = sizeof(CodecFSB);

    return &fsbcodec;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 0cec473c manual
// OFFSET: 0x00911FD0
FMOD_RESULT CodecFSB::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    unsigned int    rd;
    int             count;
    char           *shdrblock = 0;
    FMOD_RESULT     result = FMOD_OK;
    unsigned int    offset, soffset;
    CodecFSBCache  *current = 0;
    bool            found = false;

    init(FMOD_SOUND_TYPE_FSB);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecFSB::openInternal", "attempting to open as FSB..\n"));

    result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mFile->read(&mHeader, 1, sizeof(FMOD_FSB_HEADER), &rd);
    if (result != FMOD_OK)
    {
        return result;
    }
    if (rd != sizeof(FMOD_FSB_HEADER))
    {
        return FMOD_ERR_FILE_BAD;
    }

    if (!FMOD_strncmp(mHeader.id, "FSB3", 4))
    {
        result = mFile->seek((int)sizeof(FMOD_FSB3_HEADER) - (int)sizeof(FMOD_FSB_HEADER), SEEK_CUR);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    else if (FMOD_strncmp(mHeader.id, "FSB4", 4))
    {
        return FMOD_ERR_FORMAT;
    }

    if (mHeader.version < FMOD_FSB_VERSION_3_1)
    {
        return FMOD_ERR_VERSION;
    }

    mFlags |= FMOD_CODEC_FROMFSB;
    mSrcDataOffset = 0;

    if (mHeader.numsamples < 1)
    {
        return FMOD_ERR_FILE_BAD;
    }

    if (mHeader.mode & FMOD_FSB_SOURCE_FORMAT)
    {
        return FMOD_ERR_FORMAT;
    }

    if (mHeader.version >= FMOD_FSB_VERSION_4_0)
    {
        for (current = (CodecFSBCache *)gCacheHead.getNext(); current != &gCacheHead; current = (CodecFSBCache *)current->getNext())
        {
            if (!memcmp(&mHeader, &current->mHeader, sizeof(FMOD_FSB_HEADER)))
            {
                found = true;
                break;
            }
        }
    }

    if (found)
    {
        result = mFile->seek(mHeader.shdrsize, SEEK_CUR);
        if (result != FMOD_OK)
        {
            return result;
        }

        mShdr       = current->mShdr;
        mShdrb      = current->mShdrb;
        shdrblock   = current->mShdrData;
        current->mShareCount++;
        mCacheEntry = current;
    }
    else
    {
        if (mHeader.version >= FMOD_FSB_VERSION_4_0)
        {
            mCacheEntry = FMOD_Object_Alloc(CodecFSBCache);
            if (!mCacheEntry)
            {
                return FMOD_ERR_MEMORY;
            }

            mCacheEntry->addBefore(&gCacheHead);
            mCacheEntry->mShareCount = 0;
            FMOD_memcpy(&mCacheEntry->mHeader, &mHeader, sizeof(FMOD_FSB_HEADER));
        }

        if (mHeader.mode & FMOD_FSB_SOURCE_BASICHEADERS)
        {
            mShdrb = (FMOD_FSB_SAMPLE_HEADER_BASIC **)FMOD_Memory_Calloc(sizeof(void *) * mHeader.numsamples);
            if (!mShdrb)
            {
                return FMOD_ERR_MEMORY;
            }
        }
        else
        {
            mShdr = (FMOD_FSB_SAMPLE_HEADER **)FMOD_Memory_Calloc(sizeof(void *) * mHeader.numsamples);
            if (!mShdr)
            {
                return FMOD_ERR_MEMORY;
            }
        }

        shdrblock = (char *)FMOD_Memory_Calloc(mHeader.shdrsize);
        if (!shdrblock)
        {
            return FMOD_ERR_MEMORY;
        }

        if (mCacheEntry)
        {
            mCacheEntry->mShdr     = mShdr;
            mCacheEntry->mShdrb    = mShdrb;
            mCacheEntry->mShdrData = shdrblock;
        }

        result = mFile->read(shdrblock, 1, mHeader.shdrsize, &rd);
        if (result != FMOD_OK)
        {
            FMOD_Memory_Free(shdrblock);
            return result;
        }
        if (rd != mHeader.shdrsize)
        {
            FMOD_Memory_Free(shdrblock);
            return FMOD_ERR_FILE_BAD;
        }
    }

    mSrcDataOffset = mHeader.shdrsize + (mHeader.version >= FMOD_FSB_VERSION_4_0 ? sizeof(FMOD_FSB_HEADER) : sizeof(FMOD_FSB3_HEADER));

    mDataOffset = (unsigned int *)FMOD_Memory_Calloc(sizeof(unsigned int) * mHeader.numsamples);
    if (!mDataOffset)
    {
        FMOD_Memory_Free(shdrblock);
        return FMOD_ERR_MEMORY;
    }

    waveformat   = 0;
    offset       = mSrcDataOffset;
    mFirstSample = 0;

    if ((usermode & (FMOD_SOFTWARE | FMOD_HARDWARE)) && !(mOriginalMode & (FMOD_SOFTWARE | FMOD_HARDWARE)))
    {
        usermode &= ~(FMOD_SOFTWARE | FMOD_HARDWARE);
    }
    if ((mMode & (FMOD_SOFTWARE | FMOD_HARDWARE)) && !(mOriginalMode & (FMOD_SOFTWARE | FMOD_HARDWARE)))
    {
        mMode &= ~(FMOD_SOFTWARE | FMOD_HARDWARE);
    }

    mUserMode = usermode;

    for (soffset = 0, count = 0; count < mHeader.numsamples; count++)
    {
        if ((mHeader.mode & FMOD_FSB_SOURCE_BASICHEADERS) && count)
        {
            mShdrb[count] = (FMOD_FSB_SAMPLE_HEADER_BASIC *)(shdrblock + soffset);
            mDataOffset[count] = offset;

            offset  += mShdrb[count]->lengthcompressedbytes;
            soffset += sizeof(FMOD_FSB_SAMPLE_HEADER_BASIC);

            mChannels = mFirstSample->numchannels;

            if (mFirstSample->mode & FSOUND_GCADPCM || mFirstSample->mode & FSOUND_XMA)
            {
                return FMOD_ERR_FORMAT;
            }

            if (mFirstSample->mode & FSOUND_IMAADPCM)
            {
                mReadBufferLength     = 36 * (mFirstSample->numchannels < 2 ? 2 : mFirstSample->numchannels);
                mPCMBufferLength      = 64;
                mPCMBufferLengthBytes = mPCMBufferLength * sizeof(signed short) * (mFirstSample->numchannels < 2 ? 2 : mFirstSample->numchannels);
            }
            else if (mFirstSample->mode & FSOUND_MPEG && mFirstSample->numchannels > 2 && !(usermode & FMOD_CREATESTREAM))
            {
                return FMOD_ERR_TOOMANYCHANNELS;
            }

            if (userexinfo && userexinfo->speakermap)
            {
                if (userexinfo->speakermap == FMOD_SPEAKERMAPTYPE_ALLMONO)
                {
                    mFirstSample->mode |= FSOUND_CHANNELMODE_ALLMONO;
                }
                else if (userexinfo->speakermap == FMOD_SPEAKERMAPTYPE_ALLSTEREO)
                {
                    mFirstSample->mode |= FSOUND_CHANNELMODE_ALLSTEREO;
                }
            }
        }
        else
        {
            FMOD_FSB_SAMPLE_HEADER *s = (FMOD_FSB_SAMPLE_HEADER *)(shdrblock + soffset);
            FMOD_FSB_SAMPLE_HEADER tmpsample;

            if (!count)
            {
                mFirstSample = s;
            }

            if (!(mHeader.mode & FMOD_FSB_SOURCE_BASICHEADERS))
            {
                mShdr[count] = s;
                s = mShdr[count];
            }

            if (s->name[0])
            {
                mDataOffset[count] = offset;
            }
            else
            {
                if (!count)
                {
                    return FMOD_ERR_FILE_BAD;
                }
                mDataOffset[count] = mDataOffset[count - 1];
            }

            FMOD_memcpy(&tmpsample, s, sizeof(FMOD_FSB_SAMPLE_HEADER));
            tmpsample.varvol <<= 2;
            tmpsample.varpan <<= 2;
            FMOD_memcpy(s, &tmpsample, sizeof(FMOD_FSB_SAMPLE_HEADER));

            offset  += (s->name[0] ? s->lengthcompressedbytes : 0);
            soffset += s->size;

            if (s->numchannels > mChannels)
            {
                mChannels = s->numchannels;
            }

            if (s->mode & FSOUND_XMA && !(usermode & FMOD_HARDWARE))
            {
                return FMOD_ERR_FORMAT;
            }

            if (s->mode & FSOUND_IMAADPCM)
            {
                mReadBufferLength     = 36 * (mChannels < 2 ? 2 : mChannels);
                mPCMBufferLength      = 64;
                mPCMBufferLengthBytes = mPCMBufferLength * sizeof(signed short) * (mChannels < 2 ? 2 : mChannels);
            }
            else if (s->mode & FSOUND_MPEG && s->numchannels > 2 && !(usermode & FMOD_CREATESTREAM))
            {
                return FMOD_ERR_TOOMANYCHANNELS;
            }

            if (userexinfo && userexinfo->speakermap)
            {
                if (userexinfo->speakermap == FMOD_SPEAKERMAPTYPE_ALLMONO)
                {
                    s->mode |= FSOUND_CHANNELMODE_ALLMONO;
                }
                else if (userexinfo->speakermap == FMOD_SPEAKERMAPTYPE_ALLSTEREO)
                {
                    s->mode |= FSOUND_CHANNELMODE_ALLSTEREO;
                }
            }
        }
    }

    if (mFirstSample->mode & FSOUND_GCADPCM || mFirstSample->mode & FSOUND_XMA)
    {
        return FMOD_ERR_FORMAT;
    }

    if (mFirstSample->mode & FSOUND_IMAADPCM)
    {
        if (usermode & FMOD_CREATECOMPRESSEDSAMPLE)
        {
            mDecodeADPCM = true;
        }

        if (mDecodeADPCM)
        {
            mADPCM = FMOD_Object_Calloc(CodecWav);
            if (!mADPCM)
            {
                return FMOD_ERR_MEMORY;
            }

            FMOD_memcpy(&mADPCM->mDescription, CodecWav::getDescriptionEx(), sizeof(FMOD_CODEC_DESCRIPTION_EX));
            mADPCM->mDescription.getwaveformat = &Codec::defaultGetWaveFormat;
            mADPCM->mFile = mFile;

            mADPCM->mWaveFormatMemory = (FMOD_CODEC_WAVEFORMAT *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_WAVEFORMAT));
            if (!mADPCM->mWaveFormatMemory)
            {
                return FMOD_ERR_MEMORY;
            }
            mADPCM->waveformat = mADPCM->mWaveFormatMemory;
            mADPCM->mPCMBufferLengthBytes = mPCMBufferLengthBytes;
        }
    }
    else if (mFirstSample->mode & FSOUND_MPEG)
    {
        mMPEG = FMOD_Object_Calloc(CodecMPEG);
        if (!mMPEG)
        {
            return FMOD_ERR_MEMORY;
        }

        if (!CodecMPEG::gInitialized)
        {
            CodecMPEG::initAll();
            CodecMPEG::gInitialized = true;
        }

        FMOD_memcpy(&mMPEG->mDescription, CodecMPEG::getDescriptionEx(), sizeof(FMOD_CODEC_DESCRIPTION_EX));
        mMPEG->mDescription.getwaveformat = &Codec::defaultGetWaveFormat;
        mMPEG->mFile = mFile;

        mMPEG->mWaveFormatMemory = (FMOD_CODEC_WAVEFORMAT *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_WAVEFORMAT));
        if (!mMPEG->mWaveFormatMemory)
        {
            return FMOD_ERR_MEMORY;
        }
        mMPEG->waveformat = mMPEG->mWaveFormatMemory;
        mMPEG->mSystem    = mSystem;

        mPCMBufferLength             = 1152;
        mMPEG->mPCMBufferLength      = 1152;
        mMPEG->mPCMBufferLengthBytes = 1152 * sizeof(signed short) * (mChannels < 2 ? 2 : mChannels);
        mMPEG->mPCMFrameLengthBytes  = 1152 * sizeof(signed short) * (mChannels < 2 ? 2 : mChannels);
        mMPEG->mChannels             = 0;
        mMPEG->mFlags               |= FMOD_CODEC_FROMFSB;

        if (!(usermode & FMOD_CREATECOMPRESSEDSAMPLE))
        {
            mMPEG->mMemoryBlockMemory = (CodecMPEG_MemoryBlock *)FMOD_Memory_Calloc(sizeof(CodecMPEG_MemoryBlock) * (mChannels > 2 ? mChannels : 1) + 16);
            if (!mMPEG->mMemoryBlockMemory)
            {
                return FMOD_ERR_MEMORY;
            }
            mMPEG->mMemoryBlock = (CodecMPEG_MemoryBlock *)FMOD_ALIGNPOINTER(mMPEG->mMemoryBlockMemory, 16);

            mMPEG->mPCMBufferMemory = (unsigned char *)FMOD_Memory_Calloc(mMPEG->mPCMBufferLengthBytes);
            if (!mMPEG->mPCMBufferMemory)
            {
                return FMOD_ERR_MEMORY;
            }
            mMPEG->mPCMBuffer = mMPEG->mPCMBufferMemory;

            if (mChannels > 2)
            {
                mMPEG->mChannels = mChannels;
            }

            mMPEG->resetFrame();

            for (count = 0; count < (mChannels > 2 ? mChannels : 1); count++)
            {
                mMPEG->mMemoryBlock[count].mHasXingNumFrames = false;
                mMPEG->mMemoryBlock[count].mHasXingToc       = false;
                mMPEG->mMemoryBlock[count].mFrameSizeOld     = -1;
                mMPEG->mMemoryBlock[count].mSynthBo          = 1;
                mMPEG->mMemoryBlock[count].mLayer            = 0;
            }
        }
    }

    if (!(mHeader.mode & FMOD_FSB_SOURCE_BASICHEADERS))
    {
        for (soffset = 0, count = 0; count < mHeader.numsamples; count++)
        {
            FMOD_FSB_SAMPLE_HEADER *s = (FMOD_FSB_SAMPLE_HEADER *)(shdrblock + soffset);

            if (s->mode & FSOUND_SYNCPOINTS)
            {
                char *syncpointdata = (char *)s + sizeof(FMOD_FSB_SAMPLE_HEADER);

                if (!FMOD_strncmp(syncpointdata, "SYNC", 4))
                {
                    SYNCDATA *syncdata;

                    syncpointdata += 4;

                    if (!mNumSyncPoints)
                    {
                        mNumSyncPoints = (int *)FMOD_Memory_Calloc(sizeof(int) * mHeader.numsamples);
                        if (!mNumSyncPoints)
                        {
                            return FMOD_ERR_MEMORY;
                        }

                        mSyncPoint = (SyncPoint **)FMOD_Memory_Calloc(sizeof(SyncPoint *) * mHeader.numsamples);
                        if (!mSyncPoint)
                        {
                            return FMOD_ERR_MEMORY;
                        }
                    }

                    mNumSyncPoints[count] = *((int *)syncpointdata);
                    syncdata = (SYNCDATA *)(syncpointdata + 4);

                    if (mNumSyncPoints)
                    {
                        int count2;

                        mSyncPoint[count] = (SyncPoint *)FMOD_Memory_Calloc(sizeof(SyncPoint) * mNumSyncPoints[count]);
                        if (!mSyncPoint[count])
                        {
                            return FMOD_ERR_MEMORY;
                        }

                        for (count2 = 0; count2 < mNumSyncPoints[count]; count2++)
                        {
                            FMOD_strncpy(mSyncPoint[count][count2].mName, syncdata[count2].name, FMOD_STRING_MAXNAMELEN);
                            mSyncPoint[count][count2].mOffset = syncdata[count2].offset;
                        }
                    }
                }
            }

            soffset += s->size;
        }
    }

    if (!(usermode & FMOD_CREATECOMPRESSEDSAMPLE))
    {
        if (mReadBufferLength)
        {
            mReadBuffer = (unsigned char *)FMOD_Memory_Calloc(mReadBufferLength);
            if (!mReadBuffer)
            {
                return FMOD_ERR_MEMORY;
            }
        }

        if (mPCMBufferLengthBytes)
        {
            mPCMBuffer = (unsigned char *)FMOD_Memory_Calloc(mPCMBufferLengthBytes);
            if (!mPCMBuffer)
            {
                return FMOD_ERR_MEMORY;
            }
        }
    }

    numsubsounds = mHeader.numsamples;

    if (usermode & FMOD_CREATECOMPRESSEDSAMPLE)
    {
        if (mADPCM)
        {
            if (mChannels > 2)
            {
                return FMOD_ERR_TOOMANYCHANNELS;
            }

            mADPCM->mReadBufferLength     = mReadBufferLength;
            mADPCM->mReadBuffer           = 0;
            mADPCM->mSamplesPerADPCMBlock = 64;

            getWaveFormatInternal(0, &mADPCM->waveformat[0]);

            if (!mSystem->mDSPCodecPool_ADPCM.mNumDSPCodecs)
            {
                result = mSystem->mDSPCodecPool_ADPCM.init(FMOD_DSP_CATEGORY_DSPCODECADPCM, 64, mSystem->mAdvancedSettings.maxADPCMcodecs ? mSystem->mAdvancedSettings.maxADPCMcodecs : FMOD_ADVANCEDSETTINGS_MAXADPCMCODECS);
                if (result != FMOD_OK)
                {
                    return result;
                }

                for (count = 0; count < mSystem->mDSPCodecPool_ADPCM.mNumDSPCodecs; count++)
                {
                    DSPCodec *dspcodec = SAFE_CAST(DSPCodec, mSystem->mDSPCodecPool_ADPCM.mPool[count]);
                    CodecWav *wav      = (CodecWav *)dspcodec->mCodec;

                    wav->mSrcDataOffset = 0;
                    wav->mSrcFormat     = &wav->mSrcFormatMemory;
                    wav->mSrcFormat->Format.wFormatTag = WAVE_FORMAT_IMA_ADPCM;
                }
            }
        }
        else if (mMPEG)
        {
            if (mChannels > 2)
            {
                return FMOD_ERR_TOOMANYCHANNELS;
            }

            mMPEG->mReadBufferLength = mReadBufferLength;
            mMPEG->mReadBuffer       = mReadBuffer;

            getWaveFormatInternal(0, &mMPEG->waveformat[0]);

            if (!mSystem->mDSPCodecPool_MPEG.mNumDSPCodecs)
            {
                result = mSystem->mDSPCodecPool_MPEG.init(FMOD_DSP_CATEGORY_DSPCODECMPEG, 1152, mSystem->mAdvancedSettings.maxMPEGcodecs ? mSystem->mAdvancedSettings.maxMPEGcodecs : FMOD_ADVANCEDSETTINGS_MAXMPEGCODECS);
                if (result != FMOD_OK)
                {
                    return result;
                }

                for (count = 0; count < mSystem->mDSPCodecPool_MPEG.mNumDSPCodecs; count++)
                {
                    DSPCodec  *dspcodec = SAFE_CAST(DSPCodec, mSystem->mDSPCodecPool_MPEG.mPool[count]);
                    CodecMPEG *mpeg     = (CodecMPEG *)dspcodec->mCodec;

                    mpeg->mSrcDataOffset    = 0;
                    mpeg->mWaveFormatMemory = 0;
                    mpeg->resetFrame();
                }
            }
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

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 94003586 manual
// OFFSET: 0x009106F0
FMOD_RESULT CodecFSB::closeInternal()
{
    if (mCacheEntry)
    {
        if (mCacheEntry->mShareCount)
        {
            mCacheEntry->mShareCount--;
        }
        else
        {
            if (mFirstSample)
            {
                FMOD_Memory_Free(mFirstSample);
                mFirstSample = 0;
            }
            if (mShdr)
            {
                FMOD_Memory_Free(mShdr);
                mShdr = 0;
            }
            if (mShdrb)
            {
                FMOD_Memory_Free(mShdrb);
                mShdrb = 0;
            }

            mCacheEntry->removeNode();
            FMOD_Memory_Free(mCacheEntry);
            mCacheEntry = 0;
        }
    }
    else
    {
        if (mFirstSample)
        {
            FMOD_Memory_Free(mFirstSample);
            mFirstSample = 0;
        }
        if (mShdr)
        {
            FMOD_Memory_Free(mShdr);
            mShdr = 0;
        }
        if (mShdrb)
        {
            FMOD_Memory_Free(mShdrb);
            mShdrb = 0;
        }
    }

    if (mNumSyncPoints)
    {
        FMOD_Memory_Free(mNumSyncPoints);
        mNumSyncPoints = 0;
    }

    if (mSyncPoint)
    {
        int count;

        for (count = 0; count < mHeader.numsamples; count++)
        {
            if (mSyncPoint[count])
            {
                FMOD_Memory_Free(mSyncPoint[count]);
            }
        }

        FMOD_Memory_Free(mSyncPoint);
        mSyncPoint = 0;
    }

    if (mDataOffset)
    {
        FMOD_Memory_Free(mDataOffset);
        mDataOffset = 0;
    }

    if (mPCMBuffer)
    {
        FMOD_Memory_Free(mPCMBuffer);
        mPCMBuffer = 0;
    }
    mPCMBufferLengthBytes = 0;

    if (mReadBuffer)
    {
        FMOD_Memory_Free(mReadBuffer);
        mReadBuffer = 0;
    }
    mReadBufferLength = 0;

    if (mADPCM)
    {
        mADPCM->mReadBuffer = 0;
        mADPCM->mFile = 0;
        mADPCM->release();
    }

    if (mMPEG)
    {
        mMPEG->mReadBuffer = 0;
        mMPEG->mFile = 0;
        mMPEG->release();
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: deccf956 manual
// OFFSET: 0x00911210
FMOD_RESULT CodecFSB::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    FMOD_RESULT result = FMOD_OK;
    unsigned int bytesreadinternal;
    unsigned int srcmode = mHeader.mode & FMOD_FSB_SOURCE_BASICHEADERS ? mFirstSample->mode : mShdr[mCurrentIndex]->mode;
    FMOD_CODEC_WAVEFORMAT waveformat;

    getWaveFormatInternal(mCurrentIndex, &waveformat);

    if (mMPEG)
    {
        result = mMPEG->read(buffer, mChannels > 2 ? sizebytes : sizebytes * waveformat.channels / mChannels, &bytesreadinternal);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    else if (srcmode & FSOUND_IMAADPCM && waveformat.format == FMOD_SOUND_FORMAT_PCM16)
    {
        unsigned char readbuff[36 * 16];
        int readbufflength = 36 * waveformat.channels;
        int blockalign = 36 * waveformat.channels;

        result = mFile->read(readbuff, 1, blockalign, 0);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (waveformat.channels == 1)
        {
            result = IMAAdpcm_DecodeM16(readbuff, (signed short *)mPCMBuffer, 1, blockalign, 64, 1);
        }
        else if (srcmode & FSOUND_IMAADPCMSTEREO && waveformat.channels == 2)
        {
            result = IMAAdpcm_DecodeS16(readbuff, (signed short *)mPCMBuffer, 1, blockalign, 64);
        }
        else
        {
            int count = 0;

            blockalign /= waveformat.channels;

            for (count = 0; count < waveformat.channels; count++)
            {
                signed short tempin[4096];
                int count2;

                for (count2 = 0; count2 < (int)readbufflength / waveformat.channels; count2++)
                {
                    tempin[count2] = ((signed short *)readbuff)[(count2 * waveformat.channels) + count];
                }

                result = IMAAdpcm_DecodeM16((unsigned char *)tempin, (signed short *)mPCMBuffer + count, 1, blockalign, 64, waveformat.channels);
            }
        }

        bytesreadinternal = 64 * sizeof(signed short) * waveformat.channels;
    }
    else
    {
        result = mFile->read(buffer, 1, sizebytes / mChannels * waveformat.channels, &bytesreadinternal);
        if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
        {
            return result;
        }

        /*
            Convert data from unsigned to signed that fmod requires.
        */
        if (waveformat.format == FMOD_SOUND_FORMAT_PCM8)
        {
            unsigned int len;
            unsigned char *src = (unsigned char *)buffer;

            len = bytesreadinternal >> 3;
            while (len)
            {
                src[0] ^= 128;
                src[1] ^= 128;
                src[2] ^= 128;
                src[3] ^= 128;
                src[4] ^= 128;
                src[5] ^= 128;
                src[6] ^= 128;
                src[7] ^= 128;
                src+=8;
                len--;
            }
            len = bytesreadinternal & 7;
            while (len)
            {
                src[0] ^= 128;
                src++;
                len--;
            }
        }

        if (mHeader.mode & FMOD_FSB_SOURCE_BIGENDIANPCM)
        {
            if (waveformat.format == FMOD_SOUND_FORMAT_PCM16)
            {
                unsigned int count;
                signed short *wptr = (signed short *)buffer;

                for (count=0; count < bytesreadinternal >> 1; count++)
                {
                    wptr[count] = FMOD_SWAPENDIAN_WORD(wptr[count]);
                }
            }
            else if (waveformat.format == FMOD_SOUND_FORMAT_PCMFLOAT)
            {
                unsigned int count;
                unsigned int *wptr = (unsigned int *)buffer;

                for (count=0; count < bytesreadinternal >> 1; count++)
                {
                    wptr[count] = FMOD_SWAPENDIAN_DWORD(wptr[count]);
                }
            }
        }
    }

    /*
        Upscale to the stream channel format if the incoming data is lower. (just duplicate channel data)
    */
    if (waveformat.channels < mChannels && !(srcmode & FSOUND_MPEG && mChannels > 2))
    {
        int count, block;
        int blocksize, numblocks;
        char *srcptr, *destptr;

        if (srcmode & FSOUND_IMAADPCM && waveformat.format == FMOD_SOUND_FORMAT_IMAADPCM && mChannels == 2)
        {
            blocksize = 4;
        }
        else if (waveformat.format == FMOD_SOUND_FORMAT_GCADPCM)
        {
            blocksize = 2;
        }
        else
        {
            blocksize = waveformat.blockalign / waveformat.channels;
        }

        srcptr  = (char *)buffer + (bytesreadinternal                                  ) - (blocksize * waveformat.channels);
        destptr = (char *)buffer + (bytesreadinternal / waveformat.channels * mChannels) - (blocksize * mChannels);
        numblocks = bytesreadinternal / (blocksize * waveformat.channels);

        if (blocksize == 1)
        {
            for (block = 0; block < numblocks; block++)
            {
                int srcchannel = waveformat.channels - 1;

                for (count = mChannels - 1; count >= 0; count--)
                {
                    destptr[count] = srcptr[srcchannel];
                    srcchannel--;
                    if (srcchannel < 0)
                    {
                        srcchannel = waveformat.channels - 1;
                    }
                }
                srcptr  -= waveformat.channels;
                destptr -= mChannels;
            }
        }
        else if (blocksize == 2)
        {
            signed short *srcptrw  = (signed short *)srcptr;
            signed short *destptrw = (signed short *)destptr;

            if (waveformat.channels == 1)
            {
                for (block = 0; block < numblocks; block++)
                {
                    for (count = mChannels - 1; count >= 0; count--)
                    {
                        destptrw[count] = srcptrw[0];
                    }
                    srcptrw  --;
                    destptrw -= mChannels;
                }
            }
            else
            {
                for (block = 0; block < numblocks; block++)
                {
                    int srcchannel = waveformat.channels - 1;

                    for (count = mChannels - 1; count >= 0; count--)
                    {
                        destptrw[count] = srcptrw[srcchannel];
                        srcchannel--;
                        if (srcchannel < 0)
                        {
                            srcchannel = waveformat.channels - 1;
                        }
                    }
                    srcptrw  -= waveformat.channels;
                    destptrw -= mChannels;
                }
            }
        }
        else if (blocksize == 4)
        {
            int *srcptrd  = (int *)srcptr;
            int *destptrd = (int *)destptr;

            if (waveformat.channels == 1)
            {
                for (block = 0; block < numblocks; block++)
                {
                    for (count = mChannels - 1; count >= 0; count--)
                    {
                        destptrd[count] = srcptrd[0];
                    }
                    srcptrd  --;
                    destptrd -= mChannels;
                }
            }
            else
            {
                for (block = 0; block < numblocks; block++)
                {
                    int srcchannel = waveformat.channels - 1;

                    for (count = mChannels - 1; count >= 0; count--)
                    {
                        destptrd[count] = srcptrd[srcchannel];
                        srcchannel--;
                        if (srcchannel < 0)
                        {
                            srcchannel = waveformat.channels - 1;
                        }
                    }
                    srcptrd  -= waveformat.channels;
                    destptrd -= mChannels;
                }
            }
        }
        else
        {
            for (block = 0; block < numblocks; block++)
            {
                int srcchannel = waveformat.channels - 1;

                for (count = mChannels - 1; count >= 0; count--)
                {
                    FMOD_memcpy(destptr + (count * blocksize), srcptr + (srcchannel * blocksize), blocksize);
                    srcchannel--;
                    if (srcchannel < 0)
                    {
                        srcchannel = waveformat.channels - 1;
                    }
                }
                srcptr  -= (blocksize * waveformat.channels);
                destptr -= (blocksize * mChannels);
            }
        }

        *bytesread = bytesreadinternal * mChannels / waveformat.channels;
    }
    else
    {
        *bytesread = bytesreadinternal;
    }

    return result;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 2a323545 manual
// OFFSET: 0x009117B0
FMOD_RESULT CodecFSB::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result = FMOD_OK;

    if (subsound < 0 || (numsubsounds && subsound >= numsubsounds))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (mFile->mSeekable)
    {
        unsigned int pos = 0, srcmode;
        FMOD_CODEC_WAVEFORMAT waveformat;

        if (subsound != mCurrentIndex)
        {
            mCurrentIndex = subsound;
        }

        getWaveFormatInternal(mCurrentIndex, &waveformat);

        srcmode = mHeader.mode & FMOD_FSB_SOURCE_BASICHEADERS ? mFirstSample->mode : mShdr[mCurrentIndex]->mode;

        if (postype == FMOD_TIMEUNIT_RAWBYTES)
        {
            pos = mDataOffset[mCurrentIndex] + position;

            result = mFile->seek(pos, SEEK_SET);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        else if (srcmode & FSOUND_MPEG && waveformat.format == FMOD_SOUND_FORMAT_PCM16)
        {
            mMPEG->mSrcDataOffset = mDataOffset[mCurrentIndex];
            mMPEG->mFile          = mFile;
            FMOD_memcpy(mMPEG->mWaveFormatMemory, &waveformat, sizeof(FMOD_CODEC_WAVEFORMAT));

            result = mMPEG->setPositionInternal(subsound, position, postype);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        else if (srcmode & FSOUND_IMAADPCM && waveformat.format == FMOD_SOUND_FORMAT_PCM16)
        {
            unsigned int pcm = position;
            unsigned int pcmaligned = pcm;
            unsigned int excessbytes = 0;

            pcmaligned /= 64;
            pcmaligned *= 64;

            SoundI::getBytesFromSamples(pcmaligned, &pos, waveformat.channels, FMOD_SOUND_FORMAT_IMAADPCM);

            pos += mDataOffset[mCurrentIndex];

            result = mFile->seek(pos, SEEK_SET);
            if (result != FMOD_OK)
            {
                return result;
            }

            SoundI::getBytesFromSamples(pcm - pcmaligned, &excessbytes, waveformat.channels, waveformat.format);

            while (excessbytes)
            {
                static char buff[1000];
                unsigned int read = 0, toread = 1000;

                if (toread > excessbytes)
                {
                    toread = excessbytes;
                }

                result = Codec::read(buff, toread, &read);
                if (result != FMOD_OK)
                {
                    break;
                }

                excessbytes -= read;
            }
        }
        else
        {
            result = SoundI::getBytesFromSamples(position, &pos, waveformat.channels, waveformat.format);
            if (result != FMOD_OK)
            {
                return result;
            }

            pos += mDataOffset[mCurrentIndex];

            result = mFile->seek(pos, SEEK_SET);
            if (result != FMOD_OK)
            {
                return result;
            }
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

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: bd168eea auto
// OFFSET: 0x00911C70
FMOD_RESULT CodecFSB::getPositionInternal(unsigned int *position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result;

    unsigned int pos;
    FMOD_CODEC_WAVEFORMAT waveformat;

    getWaveFormatInternal(mCurrentIndex, &waveformat);

    result = mFile->tell(&pos);
    if (result != FMOD_OK)
    {
        return result;
    }

    pos -= mDataOffset[mCurrentIndex];

    if (postype == FMOD_TIMEUNIT_PCM && waveformat.format == FMOD_SOUND_FORMAT_GCADPCM)
    {
        SoundI::getSamplesFromBytes(pos, position, waveformat.channels, waveformat.format);
    }
    else
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 1011e525 auto
// OFFSET: 0x00911D00
FMOD_RESULT CodecFSB::soundcreateInternal(int subsound, FMOD_SOUND *sound)
{
    FMOD_RESULT            result;
    SoundI                *s = (SoundI *)sound;
    FMOD_CODEC_WAVEFORMAT  waveformat;

    getWaveFormatInternal(subsound, &waveformat);

    if (!(mMode & FMOD_CREATESTREAM))
    {
        mChannels = waveformat.channels;
    }

    {
        FMOD_FSB_SAMPLE_HEADER *shdr = (FMOD_FSB_SAMPLE_HEADER *)(mHeader.mode & FMOD_FSB_SOURCE_BASICHEADERS ? mFirstSample : mShdr[subsound]);
        FMOD_FSB_SAMPLE_HEADER tmp;

        FMOD_memcpy(&tmp, shdr, sizeof(FMOD_FSB_SAMPLE_HEADER));   /* FMOD_memcpy'd instead of pointed to for alignment reasons */

        result = s->setDefaults((float)tmp.deffreq, (float)tmp.defvol / 255.0f, tmp.defpan == 0 ? -1.0f : tmp.defpan == 255 ? 1.0f : tmp.defpan == 128 ? 0.0f : ((float)tmp.defpan / 255.0f * 2.0f) - 1.0f, tmp.defpri);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = s->setVariations((float)tmp.varfreq, (float)tmp.varvol / 255.0f, (float)tmp.varpan / 255.0f);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = s->set3DMinMaxDistance(tmp.mindistance, tmp.maxdistance);
        if ((result != FMOD_OK) && (result != FMOD_ERR_NEEDS3D))
        {
            return result;
        }
    }

    if (mNumSyncPoints && mSyncPoint && mNumSyncPoints[subsound])
    {
        int count;

        for (count = 0; count < mNumSyncPoints[subsound]; count++)
        {
            s->addSyncPoint(mSyncPoint[subsound][count].mOffset, FMOD_TIMEUNIT_PCM, mSyncPoint[subsound][count].mName, 0);
        }

        FMOD_Memory_Free(mSyncPoint[subsound]);
        mSyncPoint[subsound] = 0;
    }

    return FMOD_OK;
}

#endif //!PLATFORM_PS3_SPU


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 8c4a66d3 manual
// OFFSET: 0x00910B00
FMOD_RESULT CodecFSB::getWaveFormatInternal(int index, FMOD_CODEC_WAVEFORMAT *waveformat_out)
{
    FMOD_memset(waveformat_out, 0, sizeof(FMOD_CODEC_WAVEFORMAT));

    if (mHeader.mode & FMOD_FSB_SOURCE_BASICHEADERS)
    {
        unsigned int mode = 0;

        if (mFirstSample->mode & FSOUND_HW3D || mFirstSample->mode & FSOUND_HW2D)
        {
            mode |= FMOD_HARDWARE;
            if (mFirstSample->mode & FSOUND_HW2D)
            {
                mode |= FMOD_2D;
            }
            if (mFirstSample->mode & FSOUND_HW3D)
            {
                mode |= FMOD_3D;
            }
        }
        else
        {
            mode |= FMOD_SOFTWARE;
            if (mFirstSample->mode & FSOUND_2D)
            {
                mode |= FMOD_2D;
            }
            if (mFirstSample->mode & FSOUND_3D)
            {
                mode |= FMOD_3D;
            }
        }

        waveformat_out->mode = (FMOD_MODE)mode;

        FMOD_strncpy(waveformat_out->name, mFirstSample->name, FMOD_STRING_MAXNAMELEN);
        waveformat_out->channels      = mFirstSample->numchannels;
        waveformat_out->frequency     = mFirstSample->deffreq;

        /*
            Set FMOD Ex format based on fmod3 flags.
        */
        if (mFirstSample->mode & FSOUND_8BITS)
        {
            waveformat_out->format = FMOD_SOUND_FORMAT_PCM8;
        }
        else if (mFirstSample->mode & FSOUND_16BITS)
        {
            waveformat_out->format = FMOD_SOUND_FORMAT_PCM16;
        }
        else if (mFirstSample->mode & FSOUND_32BITS)
        {
            waveformat_out->format = FMOD_SOUND_FORMAT_PCMFLOAT;
        }
        else if (mFirstSample->mode & FSOUND_IMAADPCM)
        {
            if (mADPCM)
            {
                waveformat_out->format  = FMOD_SOUND_FORMAT_IMAADPCM;
            }
            else
            {
                waveformat_out->format  = FMOD_SOUND_FORMAT_PCM16;
            }
        }
        else if (mFirstSample->mode & FSOUND_VAG)
        {
            waveformat_out->format = FMOD_SOUND_FORMAT_VAG;
        }
        else if (mFirstSample->mode & FSOUND_GCADPCM)
        {
            waveformat_out->format = FMOD_SOUND_FORMAT_GCADPCM;
        }
        else if (mFirstSample->mode & FSOUND_MPEG)
        {
            if (mUserMode & FMOD_CREATECOMPRESSEDSAMPLE && (waveformat_out->mode & FMOD_SOFTWARE || mUserMode & FMOD_SOFTWARE) && !(mUserMode & FMOD_CREATESTREAM))
            {
                waveformat_out->format = FMOD_SOUND_FORMAT_MPEG;
                waveformat_out->mode |= FMOD_CREATECOMPRESSEDSAMPLE;
            }
            else
            {
                waveformat_out->format = FMOD_SOUND_FORMAT_PCM16;
            }
        }

        /*
            Block size override based on machine specific block sizes.
        */
        if (!waveformat_out->blockalign)
        {
            if (waveformat_out->format == FMOD_SOUND_FORMAT_VAG)
            {
                waveformat_out->blockalign = 16 * waveformat_out->channels;
            }
            else if (waveformat_out->format == FMOD_SOUND_FORMAT_GCADPCM)
            {
                waveformat_out->blockalign = 32 * waveformat_out->channels;
            }
            else if (waveformat_out->format == FMOD_SOUND_FORMAT_XMA)
            {
                waveformat_out->blockalign = 2048;
            }
            else if (waveformat_out->format == FMOD_SOUND_FORMAT_MPEG)
            {
                waveformat_out->blockalign = 1152 * sizeof(signed short) * waveformat_out->channels;
            }
            else
            {
                SoundI::getBytesFromSamples(1, (unsigned int *)&waveformat_out->blockalign, waveformat_out->channels, waveformat_out->format);
            }
        }

        if (index)
        {
            waveformat_out->lengthbytes = mShdrb[index]->lengthcompressedbytes;
            waveformat_out->lengthpcm   = mShdrb[index]->lengthsamples;
        }
        else
        {
            waveformat_out->lengthpcm   = mFirstSample->lengthsamples;
            waveformat_out->lengthbytes = mFirstSample->lengthcompressedbytes;
        }

        mode = mFirstSample->mode & FSOUND_CHANNELMODE_MASK;
        if (mode == FSOUND_CHANNELMODE_ALLMONO)
        {
            waveformat_out->channelmask = SPEAKER_ALLMONO;
        }
        else if (mode == FSOUND_CHANNELMODE_ALLSTEREO)
        {
            waveformat_out->channelmask = SPEAKER_ALLSTEREO;
        }
    }
    else
    {
        unsigned int mode = 0;

        if (mShdr[index]->mode & FSOUND_HW3D || mShdr[index]->mode & FSOUND_HW2D)
        {
            mode |= FMOD_HARDWARE;
            if (mShdr[index]->mode & FSOUND_HW2D)
            {
                mode |= FMOD_2D;
            }
            if (mShdr[index]->mode & FSOUND_HW3D)
            {
                mode |= FMOD_3D;
            }
        }
        else
        {
            mode |= FMOD_SOFTWARE;
            if (mShdr[index]->mode & FSOUND_2D)
            {
                mode |= FMOD_2D;
            }
            if (mShdr[index]->mode & FSOUND_3D)
            {
                mode |= FMOD_3D;
            }
        }

        if (mShdr[index]->mode & FSOUND_LOOP_NORMAL)
        {
            mode |= FMOD_LOOP_NORMAL;
        }
        else if (mShdr[index]->mode & FSOUND_LOOP_BIDI)
        {
            mode |= FMOD_LOOP_BIDI;
        }

        waveformat_out->mode = (FMOD_MODE)mode;

        FMOD_strncpy(waveformat_out->name, mShdr[index]->name, FMOD_STRING_MAXNAMELEN);
        waveformat_out->channels    = mShdr[index]->numchannels;
        waveformat_out->frequency   = mShdr[index]->deffreq;
        waveformat_out->lengthpcm   = mShdr[index]->lengthsamples;
        waveformat_out->loopstart   = mShdr[index]->loopstart;
        waveformat_out->loopend     = mShdr[index]->loopend;
        waveformat_out->lengthbytes = mShdr[index]->lengthcompressedbytes;

        if (mShdr[index]->mode & FSOUND_8BITS)
        {
            waveformat_out->format = FMOD_SOUND_FORMAT_PCM8;
        }
        else if (mShdr[index]->mode & FSOUND_16BITS)
        {
            waveformat_out->format = FMOD_SOUND_FORMAT_PCM16;
        }
        else if (mShdr[index]->mode & FSOUND_32BITS)
        {
            waveformat_out->format = FMOD_SOUND_FORMAT_PCMFLOAT;
        }
        else if (mShdr[index]->mode & FSOUND_IMAADPCM)
        {
            if (mDecodeADPCM)
            {
                waveformat_out->format = FMOD_SOUND_FORMAT_IMAADPCM;
            }
            else
            {
                waveformat_out->format = FMOD_SOUND_FORMAT_PCM16;
            }
        }
        else if (mShdr[index]->mode & FSOUND_VAG)
        {
            waveformat_out->format = FMOD_SOUND_FORMAT_VAG;
        }
        else if (mShdr[index]->mode & FSOUND_GCADPCM)
        {
            waveformat_out->format = FMOD_SOUND_FORMAT_GCADPCM;
        }
        else if (mShdr[index]->mode & FSOUND_MPEG)
        {
            if (mUserMode & FMOD_CREATECOMPRESSEDSAMPLE && !(mUserMode & FMOD_CREATESTREAM))
            {
                waveformat_out->format = FMOD_SOUND_FORMAT_MPEG;
                waveformat_out->mode |= FMOD_CREATECOMPRESSEDSAMPLE;
            }
            else
            {
                waveformat_out->format = FMOD_SOUND_FORMAT_PCM16;
            }
        }

        /*
            Block size override based on machine specific block sizes.
        */
        if (!waveformat_out->blockalign)
        {
            if (waveformat_out->format == FMOD_SOUND_FORMAT_VAG)
            {
                waveformat_out->blockalign = 16 * waveformat_out->channels;
            }
            else if (waveformat_out->format == FMOD_SOUND_FORMAT_GCADPCM)
            {
                waveformat_out->blockalign = 32 * waveformat_out->channels;
            }
            else if (waveformat_out->format == FMOD_SOUND_FORMAT_XMA)
            {
                waveformat_out->blockalign = 2048;
            }
            else if (waveformat_out->format == FMOD_SOUND_FORMAT_MPEG)
            {
                waveformat_out->blockalign = 1152 * sizeof(signed short) * waveformat_out->channels;
            }
            else
            {
                SoundI::getBytesFromSamples(1, (unsigned int *)&waveformat_out->blockalign, waveformat_out->channels, waveformat_out->format);
            }
        }

        mode = mShdr[index]->mode & FSOUND_CHANNELMODE_MASK;
        if (mode == FSOUND_CHANNELMODE_ALLMONO)
        {
            waveformat_out->channelmask = SPEAKER_ALLMONO;
        }
        else if (mode == FSOUND_CHANNELMODE_ALLSTEREO)
        {
            waveformat_out->channelmask = SPEAKER_ALLSTEREO;
        }
    }

    if (!waveformat_out->channelmask)
    {
        if (waveformat_out->channels == 4)
        {
            waveformat_out->channelmask = SPEAKER_QUAD;
        }
        else if (waveformat_out->channels == 6)
        {
            waveformat_out->channelmask = SPEAKER_5POINT1;
        }
    }

    return FMOD_OK;
}




#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 18d9d68f auto
// OFFSET: 0x009109D0
FMOD_RESULT CodecFSB::resetInternal()
{
    if (mADPCM)
    {
        mADPCM->reset();
    }

    if (mMPEG)
    {
        mMPEG->reset();
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT CodecFSB::canPointInternal()
{
    if (mADPCM)
    {
        return FMOD_ERR_MEMORY_CANTPOINT;
    }

    if (mMPEG)
    {
        return FMOD_ERR_MEMORY_CANTPOINT;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: cf4148c0 manual
// OFFSET: 0x00912E80
FMOD_RESULT F_CALLBACK CodecFSB::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecFSB *fsb = (CodecFSB *)codec;    

    return fsb->openInternal(usermode, userexinfo);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: f913560c manual
// OFFSET: 0x00910A60
FMOD_RESULT F_CALLBACK CodecFSB::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecFSB *fsb = (CodecFSB *)codec;    

    return fsb->closeInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: ce2b80f9 auto
// OFFSET: 0x00911EF0
FMOD_RESULT F_CALLBACK CodecFSB::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecFSB *fsb = (CodecFSB *)codec;    

    return fsb->readInternal(buffer, sizebytes, bytesread);
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 470487a2 auto
// OFFSET: 0x00911F20
FMOD_RESULT F_CALLBACK CodecFSB::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecFSB *fsb = (CodecFSB *)codec;    

    return fsb->setPositionInternal(subsound, position, postype);
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 5d21033d manual
// OFFSET: 0x00911F50
FMOD_RESULT F_CALLBACK CodecFSB::getPositionCallback(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype)
{
    CodecFSB *fsb = (CodecFSB *)codec;    

    return fsb->getPositionInternal(position, postype);
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 1275cedf manual
// OFFSET: 0x00911F90
FMOD_RESULT F_CALLBACK CodecFSB::soundcreateCallback(FMOD_CODEC_STATE *codec, int subsound, FMOD_SOUND *sound)
{
    CodecFSB *fsb = (CodecFSB *)codec;    

    return fsb->soundcreateInternal(subsound, sound);
}

#endif //!PLATFORM_PS3_SPU


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: b2937076 manual
// OFFSET: 0x009111D0
FMOD_RESULT F_CALLBACK CodecFSB::getWaveFormatCallback(FMOD_CODEC_STATE *codec, int subsound, FMOD_CODEC_WAVEFORMAT *waveformat)
{
    CodecFSB *fsb = (CodecFSB *)codec;    

    return fsb->getWaveFormatInternal(subsound, waveformat);
}

#ifndef PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 355eec6f manual
// OFFSET: 0x00910A90
FMOD_RESULT F_CALLBACK CodecFSB::resetCallback(FMOD_CODEC_STATE *codec)
{
    CodecFSB *fsb = (CodecFSB *)codec;    

    return fsb->resetInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: c3819788 manual
// OFFSET: 0x00910AC0
FMOD_RESULT F_CALLBACK CodecFSB::canPointCallback(FMOD_CODEC_STATE *codec)
{
    CodecFSB *fsb = (CodecFSB *)codec;    

    return fsb->canPointInternal();
}




#endif //!PLATFORM_PS3_SPU

}

#endif

