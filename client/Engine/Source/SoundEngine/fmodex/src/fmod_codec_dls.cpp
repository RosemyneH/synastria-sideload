#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_DLS

#include "fmod.h"
#include "fmod_codec_dls.h"
#include "fmod_codec_wav.h"
#include "fmod_soundi.h"

#include <stdio.h>

namespace FMOD
{

FMOD_CODEC_DESCRIPTION_EX dlscodec;


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
        return CodecDLS::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


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
// VERIFY: b211a1c8 manual
// OFFSET: 0x008F9F60
FMOD_CODEC_DESCRIPTION_EX *CodecDLS::getDescriptionEx()
{
    FMOD_memset(&dlscodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    dlscodec.name        = "FMOD DLS Codec";
    dlscodec.version     = 0x00010100;
    dlscodec.timeunits   = FMOD_TIMEUNIT_PCM;
    dlscodec.open        = &CodecDLS::openCallback;
    dlscodec.close       = &CodecDLS::closeCallback;
    dlscodec.read        = &CodecDLS::readCallback;
    dlscodec.setposition = &CodecDLS::setPositionCallback;

    dlscodec.mType       = FMOD_SOUND_TYPE_DLS;
    dlscodec.mSize       = sizeof(CodecDLS);

    return &dlscodec;
}


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
// VERIFY: 206165d0 manual
// OFFSET: 0x008F8E40
FMOD_RESULT CodecDLS::parseChunk(char *parentchunk, unsigned int chunksize)
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
            return result;
        }

		result = mFile->read(&chunk, 1, sizeof(WAVE_CHUNK), 0);
        if (result != FMOD_OK)
        {
            return result;
        }

        #ifdef PLATFORM_ENDIAN_BIG
        chunk.size = FMOD_SWAPENDIAN_DWORD(chunk.size);
        #endif

        //FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecDLS::parseRIFF","chunk : id %c%c%c%c size %d\n", chunk.id[0],chunk.id[1],chunk.id[2],chunk.id[3], chunk.size));

		/*
            CHUNKS WE DONT CARE ABOUT
        */
		if (!FMOD_strncmp((const char *)chunk.id, "vers", 4) ||
            !FMOD_strncmp((const char *)chunk.id, "msyn", 4) ||
		    !FMOD_strncmp((const char *)chunk.id, "dlid", 4))
		{
		}

		/*
            NUMBER OF INSTRUMENTS CHUNK
        */
		else if (!FMOD_strncmp((const char *)chunk.id, "colh", 4))
		{
            result = mFile->read(&mNumInstruments, 4, 1, 0);
            if (result != FMOD_OK)
            {
                return result;
            }

            mInstrument = (CodecDLSInstrument *)FMOD_Memory_Calloc(mNumInstruments * sizeof(CodecDLSInstrument));
            if (!mInstrument)
            {
                return FMOD_ERR_MEMORY;
            }
		}
		/*
            POOL TABLE CHUNK
        */
		else if (!FMOD_strncmp((const char *)chunk.id, "ptbl", 4))
		{
            unsigned int size;

            result = mFile->read(&size, 4, 1, 0);
            if (result != FMOD_OK)
            {
                return result;
            }
            result = mFile->read(&mNumSamples, 4, 1, 0);
            if (result != FMOD_OK)
            {
                return result;
            }

            /*
                Fill out base class members, also pointing to or allocating storage for them.
            */
            waveformat = (FMOD_CODEC_WAVEFORMAT *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_WAVEFORMAT) * mNumSamples);
            if (!waveformat)
            {
                return FMOD_ERR_MEMORY;
            }

            mSample = (CodecDLSSample *)FMOD_Memory_Calloc(mNumSamples *sizeof(CodecDLSSample));
            if (!mSample)
            {
                return FMOD_ERR_MEMORY;
            }
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
                return result;
            }
            
            result = parseChunk(listid, chunk.size);
            if (result != FMOD_OK)
            {
                return result;
            }

            if (!FMOD_strncmp((const char *)listid, "wave", 4))
            {
                mSampleID++;
            }
            else if (!FMOD_strncmp((const char *)listid, "ins ", 4))
            {
                mInstrumentID++;
                if (mInstrumentID >= mNumInstruments)
                {
                    mInstrumentID = mInstrumentID;
                }
            }
            else if (!FMOD_strncmp((const char *)listid, "rgn ", 4))
            {
                mRegionID++;
            }
        }
        else if (!FMOD_strncmp((const char *)chunk.id, "dlid", 4))
        {
            FMOD_GUID dlsid;
            
		    result = mFile->read(&dlsid, 1, sizeof(FMOD_GUID), 0);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        else if (!FMOD_strncmp((const char *)chunk.id, "insh", 4))
        {
		    result = mFile->read(&mInstrument[mInstrumentID].mHeader, 1, sizeof(DLS_INSTRUMENTHEADER), 0);
            if (result != FMOD_OK)
            {
                return result;
            }

            #ifdef PLATFORM_ENDIAN_BIG
            mInstrument[mInstrumentID].mHeader.cRegions            = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mHeader.cRegions);
            mInstrument[mInstrumentID].mHeader.Locale.ulBank       = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mHeader.Locale.ulBank);
            mInstrument[mInstrumentID].mHeader.Locale.ulInstrument = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mHeader.Locale.ulInstrument);
            #endif

            mInstrument[mInstrumentID].mRegion = (CodecDLSRegion *)FMOD_Memory_Calloc(sizeof(CodecDLSRegion) * mInstrument[mInstrumentID].mHeader.cRegions);
            if (!mInstrument[mInstrumentID].mRegion)
            {
                return FMOD_ERR_MEMORY;
            }
            mRegionID = 0;
        }
        else if (!FMOD_strncmp((const char *)chunk.id, "rgnh", 4))
        {
            result = mFile->read(&mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader, 1, sizeof(DLS_REGIONHEADER), 0);
            if (result != FMOD_OK)
            {
                return result;
            }

            #ifdef PLATFORM_ENDIAN_BIG
            mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.RangeKey.usLow       = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.RangeKey.usLow);
            mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.RangeKey.usHigh      = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.RangeKey.usHigh);
            mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.RangeVelocity.usLow  = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.RangeVelocity.usLow);
            mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.RangeVelocity.usHigh = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.RangeVelocity.usHigh);
            mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.fusOptions           = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.fusOptions);
            mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.usKeyGroup           = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mRegionHeader.usKeyGroup);
            #endif
        }
		else if (!FMOD_strncmp((const char *)chunk.id, "wsmp", 4))
		{
            unsigned int size = sizeof(DLS_WAVESAMPLE);

            if (chunk.size < size)
            {
                size = chunk.size;  
            }

            if (!FMOD_strncmp(parentchunk, "wave", 4))
            {
    		    result = mFile->read(&mSample[mSampleID].mWaveSample, 1, size, 0);
                if (result != FMOD_OK)
                {
                    return result;
                } 
                
                #ifdef PLATFORM_ENDIAN_BIG
                mSample[mSampleID].mWaveSample.cbSize           = FMOD_SWAPENDIAN_DWORD(mSample[mSampleID].mWaveSample.cbSize);
                mSample[mSampleID].mWaveSample.usUnityNote      = FMOD_SWAPENDIAN_WORD(mSample[mSampleID].mWaveSample.usUnityNote);
                mSample[mSampleID].mWaveSample.sFineTune        = FMOD_SWAPENDIAN_WORD(mSample[mSampleID].mWaveSample.sFineTune);
                mSample[mSampleID].mWaveSample.lAttenuation     = FMOD_SWAPENDIAN_DWORD(mSample[mSampleID].mWaveSample.lAttenuation);
                mSample[mSampleID].mWaveSample.fulOptions       = FMOD_SWAPENDIAN_DWORD(mSample[mSampleID].mWaveSample.fulOptions);
                #endif               

                if (mSample[mSampleID].mWaveSample.cSampleLoops)
                {
                    #ifdef PLATFORM_ENDIAN_BIG
                    mSample[mSampleID].mWaveSample.loop[0].cbSize       = FMOD_SWAPENDIAN_DWORD(mSample[mSampleID].mWaveSample.loop[0].cbSize);
                    mSample[mSampleID].mWaveSample.loop[0].ulLoopType   = FMOD_SWAPENDIAN_DWORD(mSample[mSampleID].mWaveSample.loop[0].ulLoopType);
                    mSample[mSampleID].mWaveSample.loop[0].ulLoopStart  = FMOD_SWAPENDIAN_DWORD(mSample[mSampleID].mWaveSample.loop[0].ulLoopStart);
                    mSample[mSampleID].mWaveSample.loop[0].ulLoopLength = FMOD_SWAPENDIAN_DWORD(mSample[mSampleID].mWaveSample.loop[0].ulLoopLength);
                    #endif
                
                    waveformat[mSampleID].mode      = FMOD_LOOP_NORMAL;
                    waveformat[mSampleID].loopstart = mSample[mSampleID].mWaveSample.loop[0].ulLoopStart;
                    waveformat[mSampleID].loopend   = mSample[mSampleID].mWaveSample.loop[0].ulLoopStart + 
                                                      mSample[mSampleID].mWaveSample.loop[0].ulLoopLength - 1;
                }
            }
            else if (!FMOD_strncmp(parentchunk, "rgn ", 4))
            {
    		    result = mFile->read(&mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample, 1, size, 0);
                if (result != FMOD_OK)
                {
                    return result;
                }   
                
                #ifdef PLATFORM_ENDIAN_BIG
                mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.cbSize           = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.cbSize);
                mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.usUnityNote      = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.usUnityNote);
                mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.sFineTune        = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.sFineTune);
                mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.lAttenuation     = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.lAttenuation);
                mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.fulOptions       = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.fulOptions);

                if (mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.cSampleLoops)
                {
                    mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.loop[0].cbSize       = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.loop[0].cbSize);
                    mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.loop[0].ulLoopType   = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.loop[0].ulLoopType);
                    mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.loop[0].ulLoopStart  = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.loop[0].ulLoopStart);
                    mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.loop[0].ulLoopLength = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveSample.loop[0].ulLoopLength);
                }
                #endif
            }
        }
        else if (!FMOD_strncmp((const char *)chunk.id, "wlnk", 4))
        {
            result = mFile->read(&mInstrument[mInstrumentID].mRegion[mRegionID].mWaveLink, 1, sizeof(DLS_WAVELINK), 0);
            if (result != FMOD_OK)
            {
                return result;
            }
            
            #ifdef PLATFORM_ENDIAN_BIG
            mInstrument[mInstrumentID].mRegion[mRegionID].mWaveLink.fusOptions    = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveLink.fusOptions);
            mInstrument[mInstrumentID].mRegion[mRegionID].mWaveLink.usPhaseGroup  = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveLink.usPhaseGroup);
            mInstrument[mInstrumentID].mRegion[mRegionID].mWaveLink.ulChannel     = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveLink.ulChannel);
            mInstrument[mInstrumentID].mRegion[mRegionID].mWaveLink.ulTableIndex  = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mRegion[mRegionID].mWaveLink.ulTableIndex);
            #endif            
        }
        else if (!FMOD_strncmp((const char *)chunk.id, "art1", 4))
        {
            struct
            {        
                unsigned int        cbSize;
                unsigned int        cConnectionBlocks;
            } articulator;

            result = mFile->read(&articulator, 1, sizeof(articulator), 0);
            if (result != FMOD_OK)
            {
                return result;
            }
            
            #ifdef PLATFORM_ENDIAN_BIG
            articulator.cbSize            = FMOD_SWAPENDIAN_DWORD(articulator.cbSize);
            articulator.cConnectionBlocks = FMOD_SWAPENDIAN_DWORD(articulator.cConnectionBlocks);
            #endif
            
            if (articulator.cbSize > sizeof(articulator))
            {
                mFile->seek(articulator.cbSize - sizeof(articulator), SEEK_CUR);
            }

            if (mRegionID < mInstrument[mInstrumentID].mHeader.cRegions)
            {
                mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock = (DLS_CONNECTIONBLOCK *)FMOD_Memory_Calloc(sizeof(DLS_CONNECTIONBLOCK) * articulator.cConnectionBlocks);
                if (!mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock)
                {
                    return FMOD_ERR_MEMORY;
                }

                mInstrument[mInstrumentID].mRegion[mRegionID].mNumConnectionBlocks = articulator.cConnectionBlocks;

                result = mFile->read(mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock, 1, sizeof(DLS_CONNECTIONBLOCK) * articulator.cConnectionBlocks, 0);
                if (result != FMOD_OK)
                {
                    return result;
                }
                
                #ifdef PLATFORM_ENDIAN_BIG
                {
                    unsigned int count;
                 
                    for (count = 0; count < articulator.cConnectionBlocks; count++)
                    {
                        mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock[count].usSource      = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock[count].usSource);
                        mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock[count].usControl     = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock[count].usControl);
                        mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock[count].usDestination = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock[count].usDestination);
                        mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock[count].usTransform   = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock[count].usTransform);
                        mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock[count].lScale        = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mRegion[mRegionID].mConnectionBlock[count].lScale);
                    }
                }
                #endif
            }
            else
            {
                mInstrument[mInstrumentID].mConnectionBlock = (DLS_CONNECTIONBLOCK *)FMOD_Memory_Calloc(sizeof(DLS_CONNECTIONBLOCK) * articulator.cConnectionBlocks);
                if (!mInstrument[mInstrumentID].mConnectionBlock)
                {
                    return FMOD_ERR_MEMORY;
                }

                mInstrument[mInstrumentID].mNumConnectionBlocks = articulator.cConnectionBlocks;

                result = mFile->read(mInstrument[mInstrumentID].mConnectionBlock, 1, sizeof(DLS_CONNECTIONBLOCK) * articulator.cConnectionBlocks, 0);
                if (result != FMOD_OK)
                {
                    return result;
                }
                
                #ifdef PLATFORM_ENDIAN_BIG
                unsigned int count;
                
                for (count = 0; count < articulator.cConnectionBlocks; count++)
                {
                    mInstrument[mInstrumentID].mConnectionBlock[count].usSource      = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mConnectionBlock[count].usSource);
                    mInstrument[mInstrumentID].mConnectionBlock[count].usControl     = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mConnectionBlock[count].usControl);
                    mInstrument[mInstrumentID].mConnectionBlock[count].usDestination = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mConnectionBlock[count].usDestination);
                    mInstrument[mInstrumentID].mConnectionBlock[count].usTransform   = FMOD_SWAPENDIAN_WORD(mInstrument[mInstrumentID].mConnectionBlock[count].usTransform);
                    mInstrument[mInstrumentID].mConnectionBlock[count].lScale        = FMOD_SWAPENDIAN_DWORD(mInstrument[mInstrumentID].mConnectionBlock[count].lScale);
                }
                #endif                
            }
        }
		/*
            FORMAT CHUNK
        */
		else if (!FMOD_strncmp((const char *)chunk.id, "fmt ", 4))
		{
            WAVE_FORMATEXTENSIBLE srcformat;
            unsigned int          size = chunk.size;

            /* Reduced allocs here and used a local instance of waveformatextensible instead */
            if (size > sizeof(WAVE_FORMATEXTENSIBLE))
            {
                size = sizeof(WAVE_FORMATEXTENSIBLE);
            }

            FMOD_memset(&srcformat, 0, sizeof(WAVE_FORMATEXTENSIBLE));
			result = mFile->read(&srcformat, 1, size, 0);
            if (result != FMOD_OK)
            {
                return result;
            }

            if (chunk.size > sizeof(WAVE_FORMATEXTENSIBLE))
            {
                result = mFile->seek(chunk.size - sizeof(WAVE_FORMATEXTENSIBLE), SEEK_CUR);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }

            #ifdef PLATFORM_ENDIAN_BIG
            srcformat.Format.wFormatTag            = FMOD_SWAPENDIAN_WORD (srcformat.Format.wFormatTag);      
            srcformat.Format.nChannels             = FMOD_SWAPENDIAN_WORD (srcformat.Format.nChannels);       
            srcformat.Format.nSamplesPerSec        = FMOD_SWAPENDIAN_DWORD(srcformat.Format.nSamplesPerSec);  
            srcformat.Format.nAvgBytesPerSec       = FMOD_SWAPENDIAN_DWORD(srcformat.Format.nAvgBytesPerSec); 
            srcformat.Format.nBlockAlign           = FMOD_SWAPENDIAN_WORD (srcformat.Format.nBlockAlign);     
            srcformat.Format.wBitsPerSample        = FMOD_SWAPENDIAN_WORD (srcformat.Format.wBitsPerSample);
            srcformat.Format.cbSize                = FMOD_SWAPENDIAN_WORD (srcformat.Format.cbSize);
            srcformat.Samples.wValidBitsPerSample  = FMOD_SWAPENDIAN_WORD (srcformat.Samples.wValidBitsPerSample);
            srcformat.dwChannelMask                = FMOD_SWAPENDIAN_DWORD(srcformat.dwChannelMask);
            #endif

            switch (srcformat.Format.wBitsPerSample)
            {
                case 8:
                {
                    waveformat[mSampleID].format =  FMOD_SOUND_FORMAT_PCM8;
                    break;
                }
                case 16:
                {
                    waveformat[mSampleID].format =  FMOD_SOUND_FORMAT_PCM16;
                    break;
                }
                case 24:
                {
                    waveformat[mSampleID].format =  FMOD_SOUND_FORMAT_PCM24;
                    break;
                }
                case 32:     
                {
                    if (srcformat.Format.wFormatTag == WAVE_FORMAT_PCM)
                    {
                        waveformat[mSampleID].format =  FMOD_SOUND_FORMAT_PCM32;
                    }
                    else if (srcformat.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
                    {
                        waveformat[mSampleID].format =  FMOD_SOUND_FORMAT_PCMFLOAT;
                    }
                    break;
                }
            }
            waveformat[mSampleID].channels    = srcformat.Format.nChannels;
            waveformat[mSampleID].frequency   = srcformat.Format.nSamplesPerSec;
            waveformat[mSampleID].blockalign  = srcformat.Format.nBlockAlign;
		}
		/*
            DATA CHUNK
        */
		else if (!FMOD_strncmp((const char *)chunk.id, "data", 4))
		{
            SoundI::getSamplesFromBytes(chunk.size, &waveformat[mSampleID].lengthpcm, waveformat[mSampleID].channels, waveformat[mSampleID].format);

			result = mFile->tell(&mSample[mSampleID].mDataOffset);
            if (result != FMOD_OK)
            {
                return result;
            }
		}
		else if (!FMOD_strncmp((const char *)chunk.id, "INAM", 4))
		{   
            if (mInstrumentID < mNumInstruments)
            {
                FMOD_memset(mInstrument[mInstrumentID].mName, 0, FMOD_STRING_MAXNAMELEN);

			    result = mFile->read(mInstrument[mInstrumentID].mName, 1, chunk.size, 0);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
            else if (mSampleID < mNumSamples)
            {
                FMOD_memset(mSample[mSampleID].mName, 0, FMOD_STRING_MAXNAMELEN);

			    result = mFile->read(mSample[mSampleID].mName, 1, chunk.size, 0);
                if (result != FMOD_OK)
                {
                    return result;
                }

                FMOD_strncpy(waveformat[mSampleID].name, mSample[mSampleID].mName, FMOD_STRING_MAXNAMELEN);
            }
		}
		else if (!FMOD_strncmp((const char *)chunk.id, "IARL", 4) ||
	             !FMOD_strncmp((const char *)chunk.id, "IART", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "ICMS", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "ICMT", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "ICOP", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "ICRD", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "IENG", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "IGNR", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "IKEY", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "IMED", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "IPRD", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "ISBJ", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "ISFT", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "ISRC", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "ISRF", 4) ||
                 !FMOD_strncmp((const char *)chunk.id, "ITCH", 4))
        {
            result = result;
        }
        else
		{
			mFile->seek(chunk.size, SEEK_CUR);
            if (result != FMOD_OK)
            {
                return result;
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

		//FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecDLS::parseRIFF", "offset = %d / %d\n", offset, chunksize));

	} while (offset < chunksize && offset > 0 && !done);

    return FMOD_OK;
}


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
// VERIFY: a71fc969 auto
// OFFSET: 0x008F9C00
FMOD_RESULT CodecDLS::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    FMOD_RESULT     result = FMOD_OK;
	WAVE_CHUNK      chunk;
    char            dls[4];

    init(FMOD_SOUND_TYPE_DLS);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecDLS::openInternal", "attempting to open as DLS..\n"));

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

	mSrcDataOffset = 0;
    mNumInstruments = 0;
    mNumSamples = 0;

    result = mFile->read(&chunk, 1, sizeof(WAVE_CHUNK), 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (FMOD_strncmp((const char *)chunk.id, "RIFF", 4))
    {
        return FMOD_ERR_FORMAT;
    }
        
    result = mFile->read(dls, 1, 4, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (FMOD_strncmp(dls, "DLS ", 4))
    {
        return FMOD_ERR_FORMAT;
    }

    #ifdef PLATFORM_ENDIAN_BIG
    chunk.size = FMOD_SWAPENDIAN_DWORD(chunk.size);
    #endif

    mSrcDataOffset = 0;
    mSampleID = 0;

    result = parseChunk(dls, chunk.size);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (mNumInstruments <= 0)
    {
        return FMOD_ERR_FORMAT;
    }

    numsubsounds = mNumSamples;

    return result;
}


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
// VERIFY: 3409d2b6 manual
// OFFSET: 0x008F8BE0
FMOD_RESULT CodecDLS::closeInternal()
{
    if (waveformat)
    {
        FMOD_Memory_Free(waveformat);
        waveformat = 0;
    }

    if (mInstrument)
    {
        int count;

        for (count = 0; count < mNumInstruments; count++)
        {
            if (mInstrument[count].mRegion)
            {
                unsigned int region;

                /*
                    Look for special drum articulators.
                */
                for (region = 0; region < mInstrument[count].mHeader.cRegions; region++)
                {
                    if (mInstrument[count].mRegion[region].mConnectionBlock)
                    {
                        FMOD_Memory_Free(mInstrument[count].mRegion[region].mConnectionBlock);
                    }
                }

                FMOD_Memory_Free(mInstrument[count].mRegion);
            }
            if (mInstrument[count].mConnectionBlock)
            {
                FMOD_Memory_Free(mInstrument[count].mConnectionBlock);
            }
        }

        FMOD_Memory_Free(mInstrument);
        mInstrument = 0;
    }

    if (mSample)
    {
        FMOD_Memory_Free(mSample);
        mSample = 0;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 62b8add7 auto
// OFFSET: 0x008F8D70
FMOD_RESULT CodecDLS::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    FMOD_RESULT result = FMOD_OK;

    result = mFile->read(buffer, 1, sizebytes, bytesread);
	if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
    {
        return result;
    }

    /*
        PCM8 is stored in unsigned 8-bit, so convert it to signed 8-bit for FMOD
    */
    if (waveformat[mCurrentIndex].format == FMOD_SOUND_FORMAT_PCM8)
    {
        unsigned char *wptr = (unsigned char *)buffer;

        for (unsigned int count = 0; count < *bytesread; count++)
        {
            *wptr++ ^= 128;
        }
    }
    else    /* Must be PCM16 */
    {
        #ifdef PLATFORM_ENDIAN_BIG
        {
            signed short *wptr = (signed short *)buffer;
            
            for (unsigned int count = 0; count < *bytesread >> 1; count++)
            {
                wptr[count] = FMOD_SWAPENDIAN_WORD(wptr[count]);
            }
        
        }
        #endif
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: eb3a36bb auto
// OFFSET: 0x008F9D00
FMOD_RESULT CodecDLS::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result = FMOD_OK;

    if (subsound < 0 || (numsubsounds && subsound >= numsubsounds))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (mFile->mSeekable)
    {
        unsigned int posbytes;

        if (subsound != mCurrentIndex)
        {
            mCurrentIndex = subsound;
        }

        result = SoundI::getBytesFromSamples(position, &posbytes, waveformat[mCurrentIndex].channels, waveformat[mCurrentIndex].format);
        if (result != FMOD_OK)
        {
            return result;
        }

        posbytes += mSample[mCurrentIndex].mDataOffset;

        result = mFile->seek(posbytes, SEEK_SET);
        if (result != FMOD_OK)
        {
            return result;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: ad3e5d48 manual
// OFFSET: 0x008F9EF0
FMOD_RESULT F_CALLBACK CodecDLS::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecDLS *dls = (CodecDLS *)codec;    

    return dls->openInternal(usermode, userexinfo);
}


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
// VERIFY: 28830508 manual
// OFFSET: 0x008F8DE0
FMOD_RESULT F_CALLBACK CodecDLS::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecDLS *dls = (CodecDLS *)codec;    

    return dls->closeInternal();
}


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
// VERIFY: 54101f48 auto
// OFFSET: 0x008F8E10
FMOD_RESULT F_CALLBACK CodecDLS::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecDLS *dls = (CodecDLS *)codec;    

    return dls->readInternal(buffer, sizebytes, bytesread);
}

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
// VERIFY: e646ee42 auto
// OFFSET: 0x008F9F30
FMOD_RESULT F_CALLBACK CodecDLS::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecDLS *dls = (CodecDLS *)codec;    

    return dls->setPositionInternal(subsound, position, postype);
}

}

#endif

