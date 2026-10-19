#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_AIFF

#include "fmod.h"
#include "fmod_codec_aiff.h"
#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_soundi.h"
#include "fmod_string.h"

#include <stdio.h>

namespace FMOD
{


FMOD_CODEC_DESCRIPTION_EX aiffcodec;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetCodecDescriptionEx is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_CODEC_DESCRIPTION_EX * F_API FMODGetCodecDescriptionEx()
    {
        return CodecAIFF::getDescriptionEx();
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
// VERIFY: a945a395 manual
// OFFSET: 0x0090E2E0
FMOD_CODEC_DESCRIPTION_EX *CodecAIFF::getDescriptionEx()
{
    FMOD_memset(&aiffcodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    aiffcodec.name        = "FMOD AIFF Codec";
    aiffcodec.version     = 0x00010100;
    aiffcodec.timeunits   = FMOD_TIMEUNIT_PCM;
    aiffcodec.open        = &CodecAIFF::openCallback;
    aiffcodec.close       = &CodecAIFF::closeCallback;
    aiffcodec.read        = &CodecAIFF::readCallback;
    aiffcodec.setposition = &CodecAIFF::setPositionCallback;

    aiffcodec.mType       = FMOD_SOUND_TYPE_AIFF;
    aiffcodec.mSize       = sizeof(CodecAIFF);

    return &aiffcodec;
}



#define UnsignedToFloat(u)  (((float)((int)(u - 2147483647L - 1))) + 2147483648.0f)

/****************************************************************
 * Extended precision IEEE floating-point conversion routine.
 ****************************************************************/
// VERIFY: e6af8ac5 manual
// OFFSET: 0x0090D700
float ConvertFromIeeeExtended(unsigned char *bytes)
{
    float    f;
    int    expon;
    unsigned int hiMant, loMant;
    
    expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
    hiMant    =  ((unsigned int)(bytes[2] & 0xFF) << 24)
            |    ((unsigned int)(bytes[3] & 0xFF) << 16)
            |    ((unsigned int)(bytes[4] & 0xFF) << 8)
            |    ((unsigned int)(bytes[5] & 0xFF));
    loMant    =  ((unsigned int)(bytes[6] & 0xFF) << 24)
            |    ((unsigned int)(bytes[7] & 0xFF) << 16)
            |    ((unsigned int)(bytes[8] & 0xFF) << 8)
            |    ((unsigned int)(bytes[9] & 0xFF));

    if (expon == 0 && hiMant == 0 && loMant == 0) 
    {
        f = 0;
    }
    else 
    {
        if (expon == 0x7FFF) 
        {   /* Infinity or NaN */
            f = 0;
        }
        else 
        {
            expon -= 16383;
            f  = (float)FMOD_LDEXP(UnsignedToFloat(hiMant), expon-=31);
            f += (float)FMOD_LDEXP(UnsignedToFloat(loMant), expon-=32);
        }
    }

    if (bytes[0] & 0x80)
    {
        return -f;
    }
    else
    {
        return f;
    }
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
// VERIFY: d6ceef30 manual
// OFFSET: 0x0090D990
FMOD_RESULT CodecAIFF::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    FMOD_RESULT     result = FMOD_OK;
	unsigned int    fileoffset, filesize;
    int             beginloop = -1, endloop = -1;
    unsigned int    lensamples = 0;
    char            id[4];
	AIFF_CHUNK      chunk;
    bool            done = false;
    int             bits = 0;

    mIsAIFC         = false;
    mLittleEndian   = false;

    init(FMOD_SOUND_TYPE_AIFF);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecAIFF::openInternal", "attempting to open as AIFF..\n"));

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

	/*
        Read header
    */
    result = mFile->read(&chunk,  1, sizeof(AIFF_CHUNK), 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (FMOD_strncmp((const char *)chunk.id, "FORM", 4))
    {
        return FMOD_ERR_FORMAT;
    }

    result = mFile->read(&id, 1, 4, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    filesize = chunk.size;
#ifdef PLATFORM_ENDIAN_LITTLE
    filesize = FMOD_SWAPENDIAN_DWORD(filesize);
#endif

    if (!FMOD_strncmp(id, "AIFC", 4))
    {
        mIsAIFC = true;
    	FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecAIFF::openInternal", "This AIFF is an AIF-C variation.\n"));
    }
    else if (FMOD_strncmp(id, "AIFF", 4))
	{
    	FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "CodecAIFF::openInternal", "'FORM' or 'AIFF' ID check failed [%c%c%c%c] : [%c%c%c%c]\n", chunk.id[0], chunk.id[1], chunk.id[2], chunk.id[3], id[0], id[1], id[2], id[3]));
		return FMOD_ERR_FORMAT;
	}

    mWaveFormatMemory = (FMOD_CODEC_WAVEFORMAT *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_WAVEFORMAT));
    if (!mWaveFormatMemory)
    {
        return FMOD_ERR_MEMORY;
    }
    waveformat = mWaveFormatMemory;


	/*
        Get size of file in bytes
    */
	result = mFile->getSize(&waveformat[0].lengthbytes);
    if (result != FMOD_OK)
    {
        return result;
    }

    mSrcDataOffset = (unsigned int)-1;

    fileoffset = 0;
	fileoffset += sizeof(AIFF_CHUNK);
    fileoffset += 4;

	/*
        Decode chunks
    */
	do 
	{
		result = mFile->seek(fileoffset, SEEK_SET);
        if (result != FMOD_OK)
        {
            return result;
        }

		result = mFile->read(&chunk, 1, sizeof(AIFF_CHUNK), 0);
        if (result != FMOD_OK)
        {
            return result;
        }

        #ifdef PLATFORM_ENDIAN_LITTLE
        chunk.size = FMOD_SWAPENDIAN_DWORD(chunk.size);
        #endif

		FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecAIFF::openInternal", "chunk : id %c%c%c%c size %d\n", chunk.id[0],chunk.id[1],chunk.id[2],chunk.id[3], chunk.size));

		/*
            COMMON CHUNK
        */
		if (!FMOD_strncmp((const char *)chunk.id, "COMM", 4))
		{
            AIFF_COMMONCHUNK commonchunk;
            AIFC_COMMONCHUNK commonchunkaifc;

            if (mIsAIFC)
            {
                result = mFile->read(&commonchunkaifc, 1, sizeof(AIFC_COMMONCHUNK), 0);
                if (result != FMOD_OK)
                {
                    return result;
                }
                
                // If format is "sowt", this means it is little endian
                if (!FMOD_strncmp(commonchunkaifc.compressionid, "NONE", 4))
                {
                    mLittleEndian = false;
                }
                else if (!FMOD_strncmp(commonchunkaifc.compressionid, "sowt", 4))
                {
                    mLittleEndian = true;
                }
                else
                {
                    return FMOD_ERR_FORMAT;
                }
            }
            else
            {
                result = mFile->read(&commonchunk, 1, sizeof(AIFF_COMMONCHUNK), 0);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }    

            #ifdef PLATFORM_ENDIAN_LITTLE
            if (mIsAIFC)
            {
                commonchunkaifc.numChannels     = FMOD_SWAPENDIAN_WORD(commonchunkaifc.numChannels);
                commonchunkaifc.numSampleFrames = FMOD_SWAPENDIAN_DWORD(commonchunkaifc.numSampleFrames);
                commonchunkaifc.sampleSize      = FMOD_SWAPENDIAN_WORD(commonchunkaifc.sampleSize);
            }
            else
            {
                commonchunk.numChannels     = FMOD_SWAPENDIAN_WORD(commonchunk.numChannels);
                commonchunk.numSampleFrames = FMOD_SWAPENDIAN_DWORD(commonchunk.numSampleFrames);
                commonchunk.sampleSize      = FMOD_SWAPENDIAN_WORD(commonchunk.sampleSize);
            }
            #endif

            if (mIsAIFC)
            {
                lensamples = commonchunkaifc.numSampleFrames;

                waveformat[0].frequency = (int)ConvertFromIeeeExtended(&commonchunkaifc.eSampleRate[0]);

        	    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecAIFF::openInternal", "channels %d samplesize %d\n", commonchunkaifc.numChannels, commonchunkaifc.sampleSize));

                bits = commonchunkaifc.sampleSize;
                result = SoundI::getFormatFromBits(bits, &waveformat[0].format);
                if (result != FMOD_OK)
                {
                    return result;
                }
                waveformat[0].channels = commonchunkaifc.numChannels;            
            }
            else
            {
                lensamples = commonchunk.numSampleFrames;

                waveformat[0].frequency = (int)ConvertFromIeeeExtended(&commonchunk.eSampleRate[0]);

        	    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecAIFF::openInternal", "channels %d samplesize %d\n", commonchunk.numChannels, commonchunk.sampleSize));

                bits = commonchunk.sampleSize;
                result = SoundI::getFormatFromBits(bits, &waveformat[0].format);
                if (result != FMOD_OK)
                {
                    return result;
                }
                waveformat[0].channels = commonchunk.numChannels;
            }
		}

		/*
            SOUND DATA CHUNK
        */
		else if (!FMOD_strncmp((const char *)chunk.id, "SSND", 4))
		{
            AIFF_SOUNDDATACHUNK sounddatachunk;

            result = mFile->read(&sounddatachunk, 1, sizeof(AIFF_SOUNDDATACHUNK), 0);
            if (result != FMOD_OK)
            {
                return result;
            }

			if (mSrcDataOffset == (unsigned int)-1)
			{
				waveformat[0].lengthbytes = chunk.size - sizeof(AIFF_SOUNDDATACHUNK);
				result = mFile->tell(&mSrcDataOffset);
                if (result != FMOD_OK)
                {
                    return result;
                }
			}

            if (!(mFile->mSeekable))
            {
                done = true;
            }
		}

		/*
            SOUND DATA CHUNK
        */
		else if (!FMOD_strncmp((const char *)chunk.id, "INST", 4))
		{
            AIFF_INSTRUMENTCHUNK instchunk;

            result = mFile->read(&instchunk, 1, sizeof(AIFF_INSTRUMENTCHUNK), 0);
            if (result != FMOD_OK)
            {
                return result;
            }

            /* want sustainLoop */
            #ifdef PLATFORM_ENDIAN_LITTLE
            instchunk.sustainLoop.beginLoop = FMOD_SWAPENDIAN_WORD(instchunk.sustainLoop.beginLoop);
            instchunk.sustainLoop.endLoop   = FMOD_SWAPENDIAN_WORD(instchunk.sustainLoop.endLoop);
            #endif

            beginloop = instchunk.sustainLoop.beginLoop;
            endloop   = instchunk.sustainLoop.endLoop;

            if (!beginloop)
            {
                beginloop++;
                endloop++;
            }
		}

		/*
            SOUND DATA CHUNK
        */
		else if (!FMOD_strncmp((const char *)chunk.id, "MARK", 4))
		{
#if 0
            AIFF_MARKER     markerchunk;
            unsigned short  nummarkers;
            int             count;

            result = mFile->read(&nummarkers, 1, 2, 0);
            if (result != FMOD_OK)
            {
                return result;
            }

            #ifdef PLATFORM_ENDIAN_LITTLE
            nummarkers = FMOD_SWAPENDIAN_WORD(nummarkers);
            #endif

            if (nummarkers && points)
            {
                FMOD_SyncPoints_Init(points);
            }
           
            for (count=0; count < nummarkers; count++)
            {
                char name[FMOD_STRING_MAXNAMELEN];

                if (FMOD_File_Read(&markerchunk, 1, sizeof(AIFF_MARKER), fp) != sizeof(AIFF_MARKER))
                {
                    FMOD_ErrorNo = FMOD_ERR_FILE_BAD;
                    return FALSE;
                }
   
                #ifdef PLATFORM_ENDIAN_LITTLE
                markerchunk.position = FMOD_SWAPENDIAN_DWORD(markerchunk.position);
                #endif
            
                FMOD_memset(name, 0, FMOD_STRING_MAXNAMELEN);
                if (FMOD_File_Read(name, 1, markerchunk.markerlength + 1, fp) != markerchunk.markerlength + 1)
                {
                    FMOD_ErrorNo = FMOD_ERR_FILE_BAD;
                    return FALSE;
                }

                if (markerchunk.position >= lensamples)
                {
                    markerchunk.position = lensamples - 1;
                }

                if (looppoints)
                {
                    if (beginloop == count + 1)
                    {
                        looppoints[0] = markerchunk.position;
                    }
                    else if (endloop == count + 1)
                    {
                        looppoints[1] = markerchunk.position;
                    }
                }

                if (points)
                {
                    FMOD_SyncPoint_Add(points, markerchunk.position, name);
                }
            }
#endif
		}

        fileoffset += sizeof(AIFF_CHUNK);
		fileoffset += chunk.size;

		if (chunk.size & 1)
        {
			fileoffset++;
        }

		if (chunk.size < 0)
        {
			break;
        }

		FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecAIFF::openInternal", "offset = %d / %d\n", fileoffset, filesize));

	} while (fileoffset < filesize && fileoffset > 0 && !done);

	/*
        Didnt find the data chunk!
    */
	if (mSrcDataOffset == (unsigned int)-1)
	{
		mSrcDataOffset = 0;
        FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "CodecAIFF::openInternal", "couldn't find a data chunk\n"));
		
        return FMOD_ERR_FILE_BAD;
	}

    result = SoundI::getSamplesFromBytes(waveformat[0].lengthbytes, &waveformat[0].lengthpcm, waveformat[0].channels, waveformat[0].format);
    if (result != FMOD_OK)
    {
        return result;
    }

    waveformat[0].blockalign = waveformat[0].channels * bits / 8;

    /*
        Fill out base class members, also pointing to or allocating storage for them.
    */
    numsubsounds  = 0;

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
FMOD_RESULT CodecAIFF::closeInternal()
{
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
// VERIFY: 22733bbf manual
// OFFSET: 0x0090D7D0
FMOD_RESULT CodecAIFF::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    FMOD_RESULT result;

    // Bring the number of bytes to a multiple of 3 bytes
    if(waveformat[0].format == FMOD_SOUND_FORMAT_PCM24)
    {
        if(sizebytes >= 3)
        {
            sizebytes = (sizebytes / 3) * 3;
        }
    }

    result = mFile->read(buffer, 1, sizebytes, bytesread);
	if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
    {
        return result;
    }

    if (waveformat[0].format == FMOD_SOUND_FORMAT_PCM16)
    {
    	unsigned int count;
    	signed short *wptr = (signed short *)buffer;

        #ifdef PLATFORM_ENDIAN_LITTLE

        if (!mLittleEndian)
        {
            count = *bytesread >> 1;
            count >>= 2;
		    while (count)
            {
			    wptr[0] = FMOD_SWAPENDIAN_WORD(wptr[0]);
			    wptr[1] = FMOD_SWAPENDIAN_WORD(wptr[1]);
			    wptr[2] = FMOD_SWAPENDIAN_WORD(wptr[2]);
			    wptr[3] = FMOD_SWAPENDIAN_WORD(wptr[3]);
                wptr+=4;
                count--;
            }

            count = *bytesread >> 1;
            count &= 3;
		    while (count)
            {
			    wptr[0] = FMOD_SWAPENDIAN_WORD(wptr[0]);
                wptr++;
                count--;
            }
        }
        
        #else

        if (mLittleEndian)
        {
            count = *bytesread >> 1;
            count >>= 2;
		    while (count)
            {
			    wptr[0] = FMOD_SWAPENDIAN_WORD(wptr[0]);
			    wptr[1] = FMOD_SWAPENDIAN_WORD(wptr[1]);
			    wptr[2] = FMOD_SWAPENDIAN_WORD(wptr[2]);
			    wptr[3] = FMOD_SWAPENDIAN_WORD(wptr[3]);
                wptr+=4;
                count--;
            }

            count = *bytesread >> 1;
            count &= 3;
		    while (count)
            {
			    wptr[0] = FMOD_SWAPENDIAN_WORD(wptr[0]);
                wptr++;
                count--;
            }
        }

        #endif
    } 
    else if (waveformat[0].format == FMOD_SOUND_FORMAT_PCM24)
    {
    	unsigned int count;
        unsigned char tmp0, tmp1, tmp2, tmp3;
    	FMOD_INT24 *wptr = (FMOD_INT24 *)buffer;

        count = *bytesread / 3;
        count >>= 2;
		while (count)
        {
			tmp0 = wptr[0].val[0];
			tmp1 = wptr[1].val[0];
			tmp2 = wptr[2].val[0];
			tmp3 = wptr[3].val[0];

            wptr[0].val[0] = wptr[0].val[2];
            wptr[1].val[0] = wptr[1].val[2];
            wptr[2].val[0] = wptr[2].val[2];
            wptr[3].val[0] = wptr[3].val[2];

            wptr[0].val[2] = tmp0;
            wptr[1].val[2] = tmp1;
            wptr[2].val[2] = tmp2;
            wptr[3].val[2] = tmp3;                
            
            wptr+=4;
            count--;
        }

        count = *bytesread / 3;
        count &= 3;
		while (count)
        {
			tmp0 = wptr[0].val[0];
            wptr[0].val[0] = wptr[0].val[2];
            wptr[0].val[2] = tmp0;
            wptr++;
            count--;
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
// VERIFY: 5dea627a auto
// OFFSET: 0x0090E0B0
FMOD_RESULT CodecAIFF::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result;
    int bits;
    unsigned int bytes;

    result = SoundI::getBitsFromFormat(waveformat[0].format, &bits);
    if (result != FMOD_OK)
    {
        return result;
    }

    SoundI::getBytesFromSamples(position, &bytes, waveformat[0].channels, waveformat[0].format);

    result = mFile->seek(mSrcDataOffset + bytes, SEEK_SET);

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
// VERIFY: bb777d46 manual
// OFFSET: 0x0090E270
FMOD_RESULT F_CALLBACK CodecAIFF::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecAIFF *aiff = (CodecAIFF *)codec;

    return aiff->openInternal(usermode, userexinfo);
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
FMOD_RESULT F_CALLBACK CodecAIFF::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecAIFF *aiff = (CodecAIFF *)codec;

    return aiff->closeInternal();
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
// VERIFY: 9da2c7b1 auto
// OFFSET: 0x0090D960
FMOD_RESULT F_CALLBACK CodecAIFF::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecAIFF *aiff = (CodecAIFF *)codec;

    return aiff->readInternal(buffer, sizebytes, bytesread);
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
// VERIFY: 3286d4c8 auto
// OFFSET: 0x0090E2B0
FMOD_RESULT F_CALLBACK CodecAIFF::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecAIFF *aiff = (CodecAIFF *)codec;

    return aiff->setPositionInternal(subsound, position, postype);
}

}

#endif


