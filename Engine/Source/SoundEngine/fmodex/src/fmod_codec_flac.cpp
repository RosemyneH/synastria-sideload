#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_FLAC

#include "fmod.h"
#include "fmod_codec_flac.h"
#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_metadata.h"
#include "fmod_soundi.h"
#include "fmod_string.h"

#include <string.h>
#include <stdio.h>


#define FMOD_FLAC_MAX_BLOCKSIZE 8192        // Theoretically, this should be 65536 but that's way too much memory and
                                            // hopefully we won't get blocks that big in real life


extern "C"
{
// VERIFY: 17a6fc8d auto
// OFFSET: 0x0090CD90
void * FMOD_FLAC_Malloc(int size)
{
    return FMOD_Memory_Alloc(size);
}
// VERIFY: 4c1f8fb8 auto
// OFFSET: 0x0090CDC0
void * FMOD_FLAC_Calloc(int count, int size)
{
    return FMOD_Memory_Calloc(count * size);
}
// VERIFY: 37417e18 auto
// OFFSET: 0x0090CDF0
void * FMOD_FLAC_ReAlloc(void *ptr, int size)
{
    return FMOD_Memory_ReAlloc(ptr, size);
}
// VERIFY: 3111e8db auto
// OFFSET: 0x0090CE20
void   FMOD_FLAC_Free(void *ptr)
{
    FMOD_Memory_Free(ptr);
}
}

namespace FMOD
{


FMOD_CODEC_DESCRIPTION_EX flaccodec;


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
        return CodecFLAC::getDescriptionEx();
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
// VERIFY: 2273d67f auto
// OFFSET: 0x0090CE40
static FLAC__StreamDecoderReadStatus FMOD_FLAC_ReadCallback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
    FMOD_RESULT result;
    unsigned int rd;
    CodecFLAC *codec;

    codec = (CodecFLAC *)client_data;
	
    result = codec->mFile->read(buffer, 1, (unsigned int)*bytes, &rd);
    *bytes = rd;

    if (!*bytes)
    {
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }

    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}


// VERIFY: 255e91b3 auto
// OFFSET: 0x0090CE80
static FLAC__StreamDecoderWriteStatus FMOD_FLAC_WriteCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
    CodecFLAC *codec;
    int i, j;

    codec = (CodecFLAC *)client_data;

    if (codec->mPCMBuffer)
    {
        int blocksize = (frame->header.blocksize > FMOD_FLAC_MAX_BLOCKSIZE) ? FMOD_FLAC_MAX_BLOCKSIZE : frame->header.blocksize;

        if (frame->header.bits_per_sample == 8)
        {
            signed char *dst = (signed char *)codec->mPCMBuffer;

            for (i = 0; i < blocksize; i++)
            {
                for (j=0; j < (int)frame->header.channels; j++)
                {
                    *dst++ = ((signed char)buffer[j][i]);
                }
            }

            codec->mPCMBufferFilledBytes = blocksize * frame->header.channels;
        }
        else if (frame->header.bits_per_sample == 16)
        {
            signed short *dst = (signed short *)codec->mPCMBuffer;

            for (i=0; i < blocksize; i++)
            {
                for (j=0; j < (int)frame->header.channels; j++)
                {
                    *dst++ = (signed short)buffer[j][i];
                }
            }

            codec->mPCMBufferFilledBytes = blocksize * frame->header.channels * 2;
        }
        else
        {
            //printf("Unhandled bitdepth %d\n", frame->header.bits_per_sample);
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


// VERIFY: 3d5fc1a4 auto
// OFFSET: 0x0090CF60
static FLAC__StreamDecoderSeekStatus FMOD_FLAC_SeekCallback(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
    CodecFLAC *codec;

    codec = (CodecFLAC *)client_data;

    if (codec->mFile->seek((int)absolute_byte_offset, SEEK_SET) == FMOD_OK)
    {
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    }
    else
    {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
}


// VERIFY: 5ad4e2cc auto
// OFFSET: 0x0090CF80
static FLAC__StreamDecoderTellStatus FMOD_FLAC_TellCallback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
    unsigned int pos;
    CodecFLAC *codec;

    codec = (CodecFLAC *)client_data;

    if (codec->mFile->tell(&pos) == FMOD_OK)
    {
        *absolute_byte_offset = (FLAC__uint64)pos;
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }
    else
    {
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
    }
}


// VERIFY: 0d583ede auto
// OFFSET: 0x0090CFC0
static FLAC__StreamDecoderLengthStatus FMOD_FLAC_LengthCallback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
    unsigned int len;
    CodecFLAC *codec;

    codec = (CodecFLAC *)client_data;

    if (codec->mFile->getSize(&len) == FMOD_OK)
    {
        *stream_length = (FLAC__uint64)len;
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    }
    else
    {
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    }
}


// VERIFY: 6c8e76f9 auto
// OFFSET: 0x0090D000
static FLAC__bool FMOD_FLAC_EofCallback(const FLAC__StreamDecoder *decoder, void *client_data)
{
    unsigned int pos, len;
    CodecFLAC *codec;

    codec = (CodecFLAC *)client_data;

    codec->mFile->tell(&pos);
    codec->mFile->getSize(&len);

    return (pos >= len);
}


static void FMOD_FLAC_ErrorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
    CodecFLAC *codec;

    codec = (CodecFLAC *)client_data;

    switch (status)
    {
        /**< An error in the stream caused the decoder to lose synchronization. */
        case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC :
            break;

	    /**< The decoder encountered a corrupted frame header. */
        case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER :
            break;

        case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH :
            break;
    }
}


// VERIFY: c9fe870b manual
// OFFSET: 0x0090D040
static void FMOD_FLAC_MetadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
    CodecFLAC *codec;

    codec = (CodecFLAC *)client_data;

    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
    {
        FMOD_RESULT result;

        result = SoundI::getFormatFromBits(metadata->data.stream_info.bits_per_sample, &codec->waveformat[0].format);
        if (result != FMOD_OK)
        {
            return;
        }
        codec->waveformat[0].channels  = metadata->data.stream_info.channels;
        codec->waveformat[0].frequency = metadata->data.stream_info.sample_rate;
        codec->waveformat[0].lengthpcm = (unsigned int)metadata->data.stream_info.total_samples;
    }
    else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT)
    {
        int i;
        char *name, *value, buf[4096];

        for (i=0;i < (int)metadata->data.vorbis_comment.num_comments;i++)
        {
            if (metadata->data.vorbis_comment.comments[i].length > 4095)
            {
                continue;
            }

            FMOD_memcpy(buf, metadata->data.vorbis_comment.comments[i].entry, metadata->data.vorbis_comment.comments[i].length);
            buf[metadata->data.vorbis_comment.comments[i].length] = 0;

            name = buf;
            value = name;
            for (;value && (*value != '=');value++) ;
            *value++ = 0;

            codec->metadata(codec, FMOD_TAGTYPE_VORBISCOMMENT, name, value, FMOD_strlen(value) + 1, FMOD_TAGDATATYPE_STRING, false);
        }
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
// VERIFY: 7f851df0 manual
// OFFSET: 0x0090D680
FMOD_CODEC_DESCRIPTION_EX *CodecFLAC::getDescriptionEx()
{
    FMOD_memset(&flaccodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    flaccodec.name        = "FMOD FLAC Codec";
    flaccodec.version     = 0x00010100;
    flaccodec.timeunits   = FMOD_TIMEUNIT_PCM;
    flaccodec.open        = &CodecFLAC::openCallback;
    flaccodec.close       = &CodecFLAC::closeCallback;
    flaccodec.read        = &CodecFLAC::readCallback;
    flaccodec.setposition = &CodecFLAC::setPositionCallback;

    flaccodec.mType       = FMOD_SOUND_TYPE_FLAC;
    flaccodec.mSize       = sizeof(CodecFLAC);

    return &flaccodec;
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
// VERIFY: ce62ca33 manual
// OFFSET: 0x0090D340
FMOD_RESULT CodecFLAC::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    unsigned int    rd;
    char            header[4];
    FMOD_RESULT     result = FMOD_OK;
    int             bits;

    init(FMOD_SOUND_TYPE_FLAC);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecFLAC::openInternal", "attempting to open as FLAC..\n"));

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        Quick format check
    */
    result = mFile->read(header, 1, 4, &rd);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (rd != 4)
    {
        return FMOD_ERR_FILE_BAD;
    }

    if ((header[0] == 'f') &&
        (header[1] == 'L') &&
        (header[2] == 'a') &&
        (header[3] == 'C'))
    {
	    result = mFile->seek(0, SEEK_SET);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    else
    {
        return FMOD_ERR_FORMAT;
    }

    mDecoder = FLAC__stream_decoder_new();
    if (!mDecoder)
    {
        return FMOD_ERR_FILE_BAD;
    }

    if (!FLAC__stream_decoder_set_md5_checking(mDecoder, false))
    {
        return FMOD_ERR_FILE_BAD;
    }

    if (!FLAC__stream_decoder_set_metadata_respond(mDecoder, FLAC__METADATA_TYPE_VORBIS_COMMENT))
    {
        return FMOD_ERR_FILE_BAD;
    }

    if (FLAC__stream_decoder_init_stream(
                                         mDecoder,
                                         FMOD_FLAC_ReadCallback,
                                         FMOD_FLAC_SeekCallback,
                                         FMOD_FLAC_TellCallback,
                                         FMOD_FLAC_LengthCallback,
                                         FMOD_FLAC_EofCallback,
                                         FMOD_FLAC_WriteCallback,
                                         FMOD_FLAC_MetadataCallback,
                                         FMOD_FLAC_ErrorCallback,
                                         this) != FLAC__STREAM_DECODER_INIT_STATUS_OK)
    {
        return FMOD_ERR_FILE_BAD;
    }

    mWaveFormatMemory = (FMOD_CODEC_WAVEFORMAT *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_WAVEFORMAT));
    if (!mWaveFormatMemory)
    {
        return FMOD_ERR_MEMORY;
    }
    waveformat = mWaveFormatMemory;

    /*
        Get the first block which is guaranteed to be a STREAMINFO metadata block
    */
    FLAC__stream_decoder_process_until_end_of_metadata(mDecoder);

	/*
        Get size of file in bytes
    */
	result = mFile->getSize(&waveformat[0].lengthbytes);
    if (result != FMOD_OK)
    {
        return result;
    }

	mSrcDataOffset  = 0;

    result = SoundI::getBitsFromFormat(waveformat[0].format, &bits);
    if (result != FMOD_OK)
    {
        return result;
    }

    result = SoundI::getBytesFromSamples(FMOD_FLAC_MAX_BLOCKSIZE, &mPCMBufferLengthBytes, waveformat[0].channels, waveformat[0].format);
    if (result != FMOD_OK)
    {
        return result;
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

    /*
        Fill out base class members, also pointing to or allocating storage for them.
    */
    numsubsounds = 0;

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
// VERIFY: 634493c4 auto
// OFFSET: 0x0090D1B0
FMOD_RESULT CodecFLAC::closeInternal()
{
    if (mDecoder)
    {
        FLAC__stream_decoder_finish(mDecoder);
        FLAC__stream_decoder_delete(mDecoder);
        mDecoder = 0;
    }

    if (mPCMBuffer)
    {
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecFLAC::release", "Free PCM Buffer\n"));

        FMOD_Memory_Free(mPCMBuffer);
        mPCMBuffer = 0;
    }
    mPCMBufferLengthBytes = 0;

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
// VERIFY: eef15f0f auto
// OFFSET: 0x0090D210
FMOD_RESULT CodecFLAC::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    FMOD_RESULT result = FMOD_OK;
    FLAC__StreamDecoderState state;

    if (!mDecoder)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    // Only get a new frame if there isn't one ready and waiting
    if (!mFrameReady)
    {
        FLAC__stream_decoder_process_single(mDecoder);
    }

    // These variables were already set directly via the FLAC Write Callback, but pass them
    // back here for consistency
    buffer      = mPCMBuffer;
    *bytesread  = mPCMBufferFilledBytes;
    mFrameReady = false;

    state = FLAC__stream_decoder_get_state(mDecoder);
    if (state == FLAC__STREAM_DECODER_END_OF_STREAM)
    {
        *bytesread = 0;
        return FMOD_ERR_FILE_EOF;
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
FMOD_RESULT CodecFLAC::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    if (!mDecoder)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (mFile->mSeekable)
    {  
        /*
            NOTE!  I think there is a stack corruption in this function.
            previously there was an 'FMOD_RESULT result' variable that was stored in a register (esi)
            which was then pushed onto the stack before calling this function.
            When the stack was popped back into esi, the number was corrupted.
        */
        if (!FLAC__stream_decoder_seek_absolute(mDecoder, position))
        {
            return FMOD_ERR_INVALID_PARAM;
        }

        // FLAC decoder seek will cause the sample at the given position to be written via the FLAC
        // Write Callback, so flag that data is ready so ReadInternal() will not overwrite the data
        mFrameReady = true;
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
// VERIFY: 030fb44c manual
// OFFSET: 0x0090D640
FMOD_RESULT F_CALLBACK CodecFLAC::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecFLAC *flac = (CodecFLAC *)codec;

    return flac->openInternal(usermode, userexinfo);
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
// VERIFY: faa4276d manual
// OFFSET: 0x0090D280
FMOD_RESULT F_CALLBACK CodecFLAC::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecFLAC *flac = (CodecFLAC *)codec;

    return flac->closeInternal();
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
// VERIFY: 3a00b6d9 auto
// OFFSET: 0x0090D2B0
FMOD_RESULT F_CALLBACK CodecFLAC::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecFLAC *flac = (CodecFLAC *)codec;

    return flac->readInternal(buffer, sizebytes, bytesread);
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
// VERIFY: d2103686 auto
// OFFSET: 0x0090D2E0
FMOD_RESULT F_CALLBACK CodecFLAC::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecFLAC *flac = (CodecFLAC *)codec;

    return flac->setPositionInternal(subsound, position, postype);
}

}

#endif


