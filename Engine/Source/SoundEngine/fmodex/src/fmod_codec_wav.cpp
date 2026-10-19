#include "fmod_settings.h"

#if defined(FMOD_SUPPORT_WAV) || defined(FMOD_SUPPORT_IMAADPCM)

#include "fmod_codec_wav.h"
#ifdef FMOD_SUPPORT_IMAADPCM
#include "fmod_codec_wav_imaadpcm.h"
#endif
#include "fmod_debug.h"
#include "fmod_dsp_codec.h"
#include "fmod_file.h"
#include "fmod_soundi.h"
#include "fmod_systemi.h"

#if defined(PLATFORM_WINDOWS) && !defined(PLUGIN_FSB)
	#include <windows.h>
	#include <mmreg.h>
	#include <msacm.h>   
#endif

#ifdef PLATFORM_PS3
extern unsigned int _binary_spu_fmod_codec_adpcm_pic_start[];
#endif

namespace FMOD
{

FMOD_CODEC_DESCRIPTION_EX wavcodec;

#if defined(PLUGIN_EXPORTS) && !defined(PLUGIN_FSB)

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetCodecDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_CODEC_DESCRIPTION_EX * F_API FMODGetCodecDescriptionEx()
    {
        return CodecWav::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */

static const FMOD_GUID _KSDATAFORMAT_SUBTYPE_PCM        = { 0x00000001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9b, 0x71 }};
static const FMOD_GUID _KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9b, 0x71 }};


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
// VERIFY: b221a29d manual
// OFFSET: 0x009101E0
FMOD_CODEC_DESCRIPTION_EX *CodecWav::getDescriptionEx()
{
#ifndef PLATFORM_PS3_SPU
    FMOD_memset(&wavcodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    wavcodec.name        = "FMOD Wav Codec";
    wavcodec.version     = 0x00010100;
    wavcodec.timeunits   = FMOD_TIMEUNIT_PCM | FMOD_TIMEUNIT_RAWBYTES;

    #if defined(FMOD_SUPPORT_WAV) && !defined(PLATFORM_PS3_SPU)
    #if !defined(PLUGIN_FSB)
    wavcodec.open        = &CodecWav::openCallback;
    #endif
    wavcodec.close       = &CodecWav::closeCallback;
    #endif

    #if defined(FMOD_SUPPORT_WAV) || defined(FMOD_SUPPORT_IMAADPCM)
    wavcodec.read        = &CodecWav::readCallback;
    wavcodec.setposition = &CodecWav::setPositionCallback;
    #endif

    #if defined(FMOD_SUPPORT_WAV)
    
    #ifndef PLATFORM_PS3_SPU
    wavcodec.soundcreate = &CodecWav::soundCreateCallback;
    #endif
    wavcodec.canpoint    = &CodecWav::canPointCallback;
    
    #endif

    wavcodec.mType       = FMOD_SOUND_TYPE_WAV;
    wavcodec.mSize       = sizeof(CodecWav);

    #if defined (PLATFORM_PS3)
    wavcodec.mModule     = (FMOD_OS_LIBRARY *)_binary_spu_fmod_codec_adpcm_pic_start;
    #endif
#else
    wavcodec.read        = &CodecWav::readCallback;
    wavcodec.setposition = &CodecWav::setPositionCallback;
#endif
    return &wavcodec;
}

#if defined(FMOD_SUPPORT_WAV) && !defined(PLATFORM_PS3_SPU)
#if !defined(PLUGIN_FSB)
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
// VERIFY: 6e0e49e4 manual
// OFFSET: 0x0090EBC0
FMOD_RESULT CodecWav::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    FMOD_RESULT     result;
    int             tag;
    char            wave[4];
    WAVE_CHUNK      chunk;

    init(FMOD_SOUND_TYPE_WAV);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecWav::openInternal", "attempting to open as WAV..\n"));

    result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = mFile->read(&chunk, 1, sizeof(WAVE_CHUNK), 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (FMOD_strncmp((const char *)chunk.id, "RIFF", 4))
    {
        return FMOD_ERR_FORMAT;
    }
        
    result = mFile->read(wave, 1, 4, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (FMOD_strncmp(wave, "WAVE", 4))
    {
        return FMOD_ERR_FORMAT;
    }

    mWaveFormatMemory = (FMOD_CODEC_WAVEFORMAT *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_WAVEFORMAT));
    if (!mWaveFormatMemory)
    {
        return FMOD_ERR_MEMORY;
    }
    waveformat = mWaveFormatMemory;

    #ifdef PLATFORM_ENDIAN_BIG
    chunk.size = FMOD_SWAPENDIAN_DWORD(chunk.size);
    #endif

    mSrcDataOffset = (unsigned int)-1;
    mSyncPoint     = 0;
    mNumSyncPoints = 0;

    result = parseChunk(chunk.size);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        Didnt find the format chunk!
    */
    if (!mSrcFormat)
    {
        return FMOD_ERR_FORMAT;
    }

    /*
        Didnt find the data chunk?
    */
    if (mSrcDataOffset == (unsigned int)-1)
    {
        FMOD_Memory_Free(mSrcFormat);
        mSrcFormat = 0;
        mSrcDataOffset = 0;
        return FMOD_ERR_FORMAT;
    }
    
    /*
	    GET A DESTINATION FORMAT USING ACM
	*/
	FMOD_memset(&mDestFormat, 0, sizeof(WAVE_FORMATEXTENSIBLE));
	mDestFormat.Format.wFormatTag = WAVE_FORMAT_PCM;
	
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecWav::openInternal", "mSrcFormat->wFormatTag = %d\n",      mSrcFormat->Format.wFormatTag));
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecWav::openInternal", "mSrcFormat->nChannels = %d\n",       mSrcFormat->Format.nChannels));
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecWav::openInternal", "mSrcFormat->nSamplesPerSec = %d\n",  mSrcFormat->Format.nSamplesPerSec));
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecWav::openInternal", "mSrcFormat->nAvgBytesPerSec = %d\n", mSrcFormat->Format.nAvgBytesPerSec));
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecWav::openInternal", "mSrcFormat->nBlockAlign = %d\n",     mSrcFormat->Format.nBlockAlign));
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecWav::openInternal", "mSrcFormat->wBitsPerSample = %d\n",  mSrcFormat->Format.wBitsPerSample));
    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecWav::openInternal", "mSrcFormat->cbSize = %d\n",          mSrcFormat->Format.cbSize));

    tag = mSrcFormat->Format.wFormatTag;

    if (tag == WAVE_FORMAT_MPEG || tag == WAVE_FORMAT_MPEGLAYER3)
    {
        return FMOD_ERR_FORMAT;    /* We want the fmod decoder to take care of mp3 decoding, not windows codecs */
    }

    if (tag == WAVE_FORMAT_EXTENSIBLE)
    {
		if (!memcmp(&mSrcFormat->SubFormat, &_KSDATAFORMAT_SUBTYPE_PCM, sizeof(FMOD_GUID)) || 
            !memcmp(&mSrcFormat->SubFormat, &_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(FMOD_GUID)) )
		{
		    FMOD_memcpy(&mDestFormat, mSrcFormat, sizeof(WAVE_FORMATEXTENSIBLE));    /* copy src format into mDestFormat */

		    waveformat[0].lengthpcm = (unsigned int)((FMOD_UINT64)waveformat[0].lengthbytes * (FMOD_UINT64)8 / (FMOD_UINT64)mDestFormat.Format.wBitsPerSample / (FMOD_UINT64)mDestFormat.Format.nChannels);
            waveformat[0].channelmask = mDestFormat.dwChannelMask;

            if (tag == WAVE_FORMAT_IEEE_FLOAT || !memcmp(&mSrcFormat->SubFormat, &_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(FMOD_GUID)))
            {
                waveformat[0].format = FMOD_SOUND_FORMAT_PCMFLOAT;
                if (mDestFormat.Format.wBitsPerSample != 32)
                {
                    return FMOD_ERR_FORMAT;
                }
            }
            else
            {
                result = SoundI::getFormatFromBits(mDestFormat.Format.wBitsPerSample, &waveformat[0].format);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
        }
        else
        {
            return FMOD_ERR_FORMAT;
        }
    }
    else if (tag == WAVE_FORMAT_PCM || tag == WAVE_FORMAT_IEEE_FLOAT)
	{
		FMOD_memcpy(&mDestFormat, mSrcFormat, sizeof(WAVE_FORMATEX));		/* copy src format into mDestFormat */

		waveformat[0].lengthpcm = (unsigned int)((FMOD_UINT64)waveformat[0].lengthbytes * (FMOD_UINT64)8 / (FMOD_UINT64)mDestFormat.Format.wBitsPerSample / (FMOD_UINT64)mDestFormat.Format.nChannels);

        if (tag == WAVE_FORMAT_IEEE_FLOAT)
        {
            waveformat[0].format = FMOD_SOUND_FORMAT_PCMFLOAT;
            if (mDestFormat.Format.wBitsPerSample != 32)
            {
                return FMOD_ERR_FORMAT;
            }
        }
        else
        {
            result = SoundI::getFormatFromBits(mDestFormat.Format.wBitsPerSample, &waveformat[0].format);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
	}
#ifdef FMOD_SUPPORT_IMAADPCM
    else if (tag == WAVE_FORMAT_IMA_ADPCM || tag == WAVE_FORMAT_XBOX_ADPCM)
    {      
        if (false
            #if defined(PLATFORM_XBOX) 
            || (mMode & FMOD_HARDWARE && tag == WAVE_FORMAT_XBOX_ADPCM)
            #endif
            )
        {
       		FMOD_memcpy(&mDestFormat, mSrcFormat, sizeof(WAVE_FORMATEXTENSIBLE));		/* copy src format into mDestFormat */
            waveformat[0].lengthpcm = waveformat[0].lengthbytes / 36 * 64;

            waveformat[0].format = FMOD_SOUND_FORMAT_IMAADPCM;
        }
        else
        {
            WAVE_FORMAT_IMAADPCM *formatadpcm = (WAVE_FORMAT_IMAADPCM *)mSrcFormat;
            int numblocks;
            
            #if defined(PLATFORM_XBOX) 
            if (usermode & FMOD_HARDWARE && usermode & FMOD_CREATECOMPRESSEDSAMPLE)   
            {
                return FMOD_ERR_NEEDSSOFTWARE; /* XADPCM yes, IMA ADPCM no.  */
            }
            #endif
            
            numblocks  = waveformat[0].lengthbytes / formatadpcm->wfx.nBlockAlign;         
            waveformat[0].lengthpcm = numblocks * formatadpcm->wSamplesPerBlock;

		    FMOD_memcpy(&mDestFormat, mSrcFormat, sizeof(WAVE_FORMATEX));		/* copy src format into mDestFormat */

            if (usermode & FMOD_CREATECOMPRESSEDSAMPLE)
            {
                waveformat[0].format               = FMOD_SOUND_FORMAT_IMAADPCM;
            }
            else
            {
                waveformat[0].format               = FMOD_SOUND_FORMAT_PCM16;
            }

            mDestFormat.Format.wFormatTag     = WAVE_FORMAT_PCM;
            mDestFormat.Format.wBitsPerSample = 16;
            mDestFormat.Format.nBlockAlign     = mDestFormat.Format.nChannels   * mDestFormat.Format.wBitsPerSample / 8;
            mDestFormat.Format.nAvgBytesPerSec = mDestFormat.Format.nBlockAlign * mDestFormat.Format.nSamplesPerSec;

            /*
                Now calculate an intermediate bufferlength for file reading and its equivalent decode buffer.
            */
            mSamplesPerADPCMBlock = formatadpcm->wSamplesPerBlock;
            mPCMBufferLength      = mSamplesPerADPCMBlock;
            if (usermode & FMOD_CREATECOMPRESSEDSAMPLE)
            {
        	    mPCMBufferLengthBytes = 0;
                mReadBufferLength     = 0;
            }
            else
            {
        	    mPCMBufferLengthBytes = mPCMBufferLength * mDestFormat.Format.wBitsPerSample / 8 * mDestFormat.Format.nChannels;
                mReadBufferLength     = mSrcFormat->Format.nBlockAlign;
            }
        }
    }
#endif
	else
	{
#if defined(PLATFORM_WINDOWS) && !defined(__MINGW32__) && !defined(PLUGIN_FSB)
        MMRESULT            hr;
    	HACMSTREAM			has;

		/*
            Now find the best destination format
        */
		hr = acmFormatSuggest(NULL, (WAVEFORMATEX *)mSrcFormat, (WAVEFORMATEX *)&mDestFormat, sizeof(WAVE_FORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG);
		if (hr)
		{
			return FMOD_ERR_FORMAT;
		}

		hr = acmStreamOpen(&has, NULL, (WAVEFORMATEX *)mSrcFormat, (WAVEFORMATEX *)&mDestFormat, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME);
		if (hr)
		{
			return FMOD_ERR_FORMAT;
		}
		mACMCodec = has;

        waveformat[0].format = FMOD_SOUND_FORMAT_PCM16;

        /*
            Get the length of the whole buffer in decompressed pcm samples.
        */
		acmStreamSize(has, waveformat[0].lengthbytes, (unsigned long *)&waveformat[0].lengthpcm, ACM_STREAMSIZEF_SOURCE);	
		
        waveformat[0].lengthpcm = (unsigned int)((FMOD_UINT64)waveformat[0].lengthpcm * (FMOD_UINT64)8 / (FMOD_UINT64)mDestFormat.Format.wBitsPerSample / (FMOD_UINT64)mDestFormat.Format.nChannels);

        /*
            Now calculate an intermediate bufferlength for file reading and its equivalent decode buffer.
        */
        if (mSrcFormat->Format.nBlockAlign)
        {
            mReadBufferLength = mSrcFormat->Format.nBlockAlign;
            if (mReadBufferLength <= 1)
            {
                mReadBufferLength = 1024;
            }

    		acmStreamSize(has, mReadBufferLength, (unsigned long *)&mPCMBufferLengthBytes, ACM_STREAMSIZEF_SOURCE);	
            if (!mPCMBufferLengthBytes)
            {
                return FMOD_ERR_MEMORY;
            }
        }
#else
		return FMOD_ERR_FORMAT;
#endif
	}
    
#ifdef PLATFORM_PS3
    if (waveformat[0].format != FMOD_SOUND_FORMAT_PCM16)
    {
        return FMOD_ERR_FORMAT;
    }
#endif

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
        #ifdef PLATFORM_PS3

        mPCMBufferMemory = (unsigned char *)FMOD_Memory_Calloc(mPCMBufferLengthBytes + 16);
        if (!mPCMBufferMemory)
        {
            return FMOD_ERR_MEMORY;
        }
        mPCMBuffer = (unsigned char *)FMOD_ALIGNPOINTER(mPCMBufferMemory, 16);

        #else

        mPCMBuffer = (unsigned char *)FMOD_Memory_Calloc(mPCMBufferLengthBytes);
        if (!mPCMBuffer)
        {
            return FMOD_ERR_MEMORY;
        }

        #endif
    }

    waveformat[0].channels   = mSrcFormat->Format.nChannels;
    waveformat[0].frequency  = mSrcFormat->Format.nSamplesPerSec;
    waveformat[0].blockalign = mSrcFormat->Format.nBlockAlign;
    waveformat[0].loopstart  = mLoopPoints[0];
    waveformat[0].loopend    = mLoopPoints[1];

    if (mLoopPoints[1] > mLoopPoints[0])
    {
        waveformat[0].mode = FMOD_LOOP_NORMAL;
    }

#ifdef FMOD_SUPPORT_IMAADPCM
    if (waveformat[0].format == FMOD_SOUND_FORMAT_IMAADPCM)
    {
        if (waveformat[0].channels > 2)                           
        {
            return FMOD_ERR_TOOMANYCHANNELS;    /* Sorry we're only allocating memory for a pool of maximum stereo voices. */
        }

        mReadBufferLength = mSrcFormat->Format.nBlockAlign;

        #ifdef PLATFORM_XBOX
        if (!(usermode & FMOD_HARDWARE))
        #endif
        {
            #ifdef FMOD_SUPPORT_DSPCODEC
            int count;

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

                    wav->mSrcFormat  = &wav->mSrcFormatMemory;
                    wav->mReadBuffer = mSystem->mDSPCodecPool_ADPCM.mReadBuffer;
                    wav->mSrcFormat->Format.wFormatTag = WAVE_FORMAT_IMA_ADPCM;
                }
            }

            /*
                This form of wav needs PCM buffers allocated, as the decode block size is variable.
            */
            for (count = 0; count < mSystem->mDSPCodecPool_ADPCM.mNumDSPCodecs; count++)
            {
                DSPCodec *dspcodec = SAFE_CAST(DSPCodec, mSystem->mDSPCodecPool_ADPCM.mPool[count]);
                CodecWav *wav      = (CodecWav *)dspcodec->mCodec;

                if (!wav->mPCMBufferMemory)
                {
                    wav->mPCMBufferMemory  = (unsigned char *)FMOD_Memory_Calloc(mPCMBufferLength * mDestFormat.Format.wBitsPerSample / 8 * 2);   /* *2 = maximum channels = 2 */
                    if (!wav->mPCMBufferMemory)
                    {
                        return FMOD_ERR_MEMORY;
                    }

                    wav->mPCMBuffer = wav->mPCMBufferMemory;
                }
            }
            #else
            return FMOD_ERR_UNSUPPORTED;
            #endif
        }       
    }
#endif

    /*
        Fill out base class members, also pointing to or allocating storage for them.
    */
    numsubsounds = 0;

    return result;
}

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
// VERIFY: 0a8f55f1 auto
// OFFSET: 0x0090F550
FMOD_RESULT CodecWav::closeInternal()
{
	if (mSrcFormat && mSrcFormat != &mSrcFormatMemory)
    {
		FMOD_Memory_Free(mSrcFormat);
        mSrcFormat = 0;
    }  
    if (mWaveFormatMemory)
    {
        FMOD_Memory_Free(mWaveFormatMemory);
        mWaveFormatMemory = 0;
    }
    if (mReadBuffer)
    {
        FMOD_Memory_Free(mReadBuffer);
        mReadBuffer = 0;
    }
    mReadBufferLength = 0;
    if (mSyncPoint)
    {
        FMOD_Memory_Free(mSyncPoint);
        mSyncPoint = 0;
    }
    mNumSyncPoints = 0;

    if (mPCMBuffer)
    {
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecWAV::release", "Free PCM Buffer\n"));

        FMOD_Memory_Free(mPCMBuffer);
        mPCMBuffer = 0;
    }
    mPCMBufferLengthBytes = 0;

    return FMOD_OK;
}
#endif // FMOD_SUPPORT_WAV


#if defined(FMOD_SUPPORT_WAV) || defined(FMOD_SUPPORT_IMAADPCM)
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
// VERIFY: d082d360 manual
// OFFSET: 0x0090F650
FMOD_RESULT CodecWav::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    FMOD_RESULT result;
    bool finished = false;

    if (mSrcFormat->Format.wFormatTag == WAVE_FORMAT_PCM
     || mSrcFormat->Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT
     || mSrcFormat->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE
        #ifdef PLATFORM_XBOX
     || (mSrcFormat->Format.wFormatTag == WAVE_FORMAT_XBOX_ADPCM && waveformat[0].format == FMOD_SOUND_FORMAT_IMAADPCM && !mPCMBuffer)
        #endif
        )
    {
        unsigned int pos;
                       
        mFile->tell(&pos);

        if (pos >= (mSrcDataOffset + waveformat[0].lengthbytes))
        {
            FLOG((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "CodecWav::readInternal", "ERROR! File position was past end of data! pos = %d : end = %d\n", pos, mSrcDataOffset + waveformat[0].lengthbytes));
            return FMOD_ERR_FILE_EOF;
        }
	    if (pos + sizebytes > mSrcDataOffset + waveformat[0].lengthbytes)
        {           
		    sizebytes = (mSrcDataOffset + waveformat[0].lengthbytes) - pos;
            finished = true;
        }

        if (waveformat[0].format == FMOD_SOUND_FORMAT_PCM8)
        {           
            unsigned char *ptr = (unsigned char *)buffer;
            unsigned int len;

            result = mFile->read(buffer, 1, sizebytes, bytesread);

            len = *bytesread >> 2;
            while (len)
            {
                ptr[0] ^= 128;      /* Convert from unsigned to signed */
                ptr[1] ^= 128;      /* Convert from unsigned to signed */
                ptr[2] ^= 128;      /* Convert from unsigned to signed */
                ptr[3] ^= 128;      /* Convert from unsigned to signed */
                ptr+=4;
                len--;
            }
            len = *bytesread & 3;
            while (len)
            {
                ptr[0] ^= 128;      /* Convert from unsigned to signed */
                ptr++;
                len--;
            }
        }
        else if (waveformat[0].format == FMOD_SOUND_FORMAT_PCM16)
        {
            result = mFile->read(buffer, 2, sizebytes / 2, bytesread);

            *bytesread *= 2;    /* convert from 16bit words back to bytes */
        }
        else
        {
            result = mFile->read(buffer, 1, sizebytes, bytesread);
        }

        if (finished)
        {
            result = FMOD_ERR_FILE_EOF;
        }

        if (result != FMOD_OK)
        {
            return result;
        }
    }
    else
#ifdef FMOD_SUPPORT_IMAADPCM
    if (mSrcFormat->Format.wFormatTag == WAVE_FORMAT_IMA_ADPCM || mSrcFormat->Format.wFormatTag == WAVE_FORMAT_XBOX_ADPCM)
    {
        int            blockalign = waveformat[0].blockalign;
        signed short  *destbuff = (signed short *)buffer;
        unsigned char *readbuff = (unsigned char *)FMOD_alloca(mReadBufferLength);

        if (!readbuff)
        {
            return FMOD_ERR_MEMORY;
        }

        result = mFile->read(readbuff, 1, mReadBufferLength, 0);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (waveformat[0].format == FMOD_SOUND_FORMAT_IMAADPCM)
        {
            if (waveformat[0].channels == 1)
            {
                result = IMAAdpcm_DecodeM16(readbuff, destbuff, 1, blockalign, mSamplesPerADPCMBlock, 1);
            }
            else if (waveformat[0].channels == 2)
            {
                result = IMAAdpcm_DecodeS16(readbuff, destbuff, 1, blockalign, mSamplesPerADPCMBlock);
            }
            else
            {
                int count = 0;
    
                blockalign /= waveformat[0].channels;

                for (count = 0; count < waveformat[0].channels; count++)
                {                   
                    signed short tempin[4096];
                    int count2;

                    for (count2 = 0; count2 < (int)mReadBufferLength / waveformat[0].channels; count2++)
                    {
                        tempin[count2] = ((signed short *)readbuff)[(count2 * waveformat[0].channels) + count];
                    }

                    result = IMAAdpcm_DecodeM16((unsigned char *)tempin, destbuff + count, 1, blockalign, mSamplesPerADPCMBlock, waveformat[0].channels);
                }
            }

            *bytesread = mSamplesPerADPCMBlock * sizeof(signed short) * waveformat[0].channels;
        }
        else
        {
            if (waveformat[0].channels == 1)
            {
                result = IMAAdpcm_DecodeM16(readbuff, destbuff, 1, blockalign, mSamplesPerADPCMBlock, 1);
            }
            else if (waveformat[0].channels == 2)
            {
                result = IMAAdpcm_DecodeS16(readbuff, destbuff, 1, blockalign, mSamplesPerADPCMBlock);
            }
            else
            {
                int count = 0;
    
                blockalign /= waveformat[0].channels;

                for (count = 0; count < waveformat[0].channels; count++)
                {                   
                    signed short tempin[4096];
                    int count2;

                    for (count2 = 0; count2 < (int)mReadBufferLength / waveformat[0].channels; count2++)
                    {
                        tempin[count2] = ((signed short *)readbuff)[(count2 * waveformat[0].channels) + count];
                    }

                    result = IMAAdpcm_DecodeM16((unsigned char *)tempin, destbuff + count, 1, blockalign, mSamplesPerADPCMBlock, waveformat[0].channels);
                }
            }

            *bytesread = mSamplesPerADPCMBlock * sizeof(signed short) * waveformat[0].channels;
        }
    }
    else
#endif
#if defined(PLATFORM_WINDOWS) && !defined(__MINGW32__) && !defined(PLUGIN_FSB)
    if (mACMCodec)
    {
	    ACMSTREAMHEADER ash;
	    MMRESULT        mmresult;

        result = mFile->read(mReadBuffer, 1, mReadBufferLength, 0);
        if (result != FMOD_OK)
        {
            return result;
        }

	    FMOD_memset(&ash, 0, sizeof(ACMSTREAMHEADER)); 
	    ash.cbStruct	= sizeof(ACMSTREAMHEADER);
	    ash.pbSrc		= mReadBuffer;
	    ash.cbSrcLength	= mReadBufferLength;
	    ash.pbDst		= (unsigned char *)buffer;
	    ash.cbDstLength	= sizebytes;

	    mmresult = acmStreamPrepareHeader(mACMCodec, &ash, 0);

	    mmresult = acmStreamConvert(mACMCodec, &ash, ACM_STREAMCONVERTF_BLOCKALIGN);

        *bytesread = ash.cbDstLengthUsed;

	    mmresult = acmStreamUnprepareHeader(mACMCodec, &ash, 0);
    }
    else
#endif  
    {
        return FMOD_ERR_PLUGIN_MISSING;
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
// VERIFY: 122b0e2a manual
// OFFSET: 0x0090FE20
FMOD_RESULT CodecWav::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result;
    unsigned int raw = 0;
    unsigned int pcmbytes = 0;
    unsigned int pcmbytesaligned = 0;
    unsigned int pcmaligned = 0;
    unsigned int excessbytes = 0;

    if (postype == FMOD_TIMEUNIT_RAWBYTES)
    {
        result = mFile->seek(mSrcDataOffset + position, SEEK_SET);
        return result;
    }

    if (position)
    {
	    raw = (unsigned int)((FMOD_UINT64)position * (FMOD_UINT64)waveformat[0].lengthbytes / (FMOD_UINT64)waveformat[0].lengthpcm);
        raw /= waveformat[0].blockalign;
        raw *= waveformat[0].blockalign;
 
	    pcmaligned = (unsigned int)((FMOD_UINT64)raw * (FMOD_UINT64)waveformat[0].lengthpcm / (FMOD_UINT64)waveformat[0].lengthbytes);

        result = SoundI::getBytesFromSamples(position, &pcmbytes, waveformat[0].channels, waveformat[0].format);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = SoundI::getBytesFromSamples(pcmaligned, &pcmbytesaligned, waveformat[0].channels, waveformat[0].format);
        if (result != FMOD_OK)
        {
            return result;
        }   
    }

    result = mFile->seek(mSrcDataOffset + raw, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    excessbytes = pcmbytes - pcmbytesaligned;

    while (excessbytes)
    {
        char buff[4096];
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

        if (excessbytes >= read)
        {
            excessbytes -= read;
        }
        else
        {
            excessbytes = 0;
        }
    }

    return result;
}
#endif // defined(FMOD_SUPPORT_WAV) || defined(FMOD_SUPPORT_IMAADPCM)


#if defined(FMOD_SUPPORT_WAV) && !defined(PLATFORM_PS3_SPU)
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
// VERIFY: 9bea24ad auto
// OFFSET: 0x0090FAF0
FMOD_RESULT CodecWav::soundCreateInternal(int subsound, FMOD_SOUND *sound)
{
    SoundI *s = (SoundI *)sound;

    if (mNumSyncPoints && mSyncPoint)
    {
        int count;

        for (count = 0; count < mNumSyncPoints; count++)
        {
            s->addSyncPoint(mSyncPoint[count].mOffset, FMOD_TIMEUNIT_PCM, mSyncPoint[count].mName, 0);
        }

        FMOD_Memory_Free(mSyncPoint);
        mSyncPoint = 0;
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
// VERIFY: e142e82d manual
// OFFSET: 0x0090FB80
FMOD_RESULT CodecWav::canPointInternal() 
{
#if defined(PLATFORM_WINDOWS) && !defined(__MINGW32__) && !defined(PLUGIN_FSB)	
    if (mACMCodec || !mSrcFormat)
    {
        return FMOD_ERR_MEMORY_CANTPOINT;
    }
#else
    if (!mSrcFormat)
    {
        return FMOD_ERR_MEMORY_CANTPOINT;
    }
#endif

    if (mSrcFormat->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
		if (memcmp(&mSrcFormat->SubFormat, &_KSDATAFORMAT_SUBTYPE_PCM, sizeof(FMOD_GUID)) && 
            memcmp(&mSrcFormat->SubFormat, &_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(FMOD_GUID)) )
        {
            return FMOD_ERR_MEMORY_CANTPOINT;
        }
    }
    else if (mSrcFormat->Format.wFormatTag != WAVE_FORMAT_PCM && mSrcFormat->Format.wFormatTag != WAVE_FORMAT_IEEE_FLOAT)
    {
        return FMOD_ERR_MEMORY_CANTPOINT;
    }



    if (mSrcFormat->Format.wBitsPerSample == 8)
    {
        return FMOD_ERR_MEMORY_CANTPOINT;
    }

    return FMOD_OK;
}


#if !defined(PLUGIN_FSB)
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
// VERIFY: 85ee4c35 manual
// OFFSET: 0x0090FD10
FMOD_RESULT F_CALLBACK CodecWav::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecWav *wav = (CodecWav *)codec;

    return wav->openInternal(usermode, userexinfo);
}
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
// VERIFY: 50a01959 manual
// OFFSET: 0x0090FD50
FMOD_RESULT F_CALLBACK CodecWav::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecWav *wav = (CodecWav *)codec;

    return wav->closeInternal();
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
// VERIFY: cf439250 manual
// OFFSET: 0x0090FD80
FMOD_RESULT F_CALLBACK CodecWav::soundCreateCallback(FMOD_CODEC_STATE *codec, int subsound, FMOD_SOUND *sound)
{
    CodecWav *wav = (CodecWav *)codec;    

    return wav->soundCreateInternal(subsound, sound);
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
// VERIFY: 2ca636eb manual
// OFFSET: 0x0090FDC0
FMOD_RESULT F_CALLBACK CodecWav::canPointCallback(FMOD_CODEC_STATE *codec)
{
    CodecWav *wav = (CodecWav *)codec;    

    return wav->canPointInternal();
}

#endif  //FMOD_SUPPORT_WAV


#if defined(FMOD_SUPPORT_WAV) || defined(FMOD_SUPPORT_IMAADPCM)
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
// VERIFY: 37f39a5d auto
// OFFSET: 0x0090FDF0
FMOD_RESULT F_CALLBACK CodecWav::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecWav *wav = (CodecWav *)codec;

    return wav->readInternal(buffer, sizebytes, bytesread);
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
// VERIFY: 44ad3cb4 auto
// OFFSET: 0x009101B0
FMOD_RESULT F_CALLBACK CodecWav::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecWav *wav = (CodecWav *)codec;

    return wav->setPositionInternal(subsound, position, postype);
}
#endif


}

#endif


