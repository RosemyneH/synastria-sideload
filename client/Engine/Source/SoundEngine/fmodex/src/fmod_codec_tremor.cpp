#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_TREMOR

#include "fmod.h"
#include "fmod_codec_tremor.h"
#include "fmod_codec_wav.h"
#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_metadata.h"
#include "fmod_string.h"

//#include "../lib/ogg_vorbis/vorbis/lib/window.h"

#include <string.h>


/*
    ***************************************************************************************************************
    ***************************************************************************************************************
    ***************************************************************************************************************

    NOTES ON OPTIMIZATION
    ---------------------

    - Throw away floor0 and res0 and res1.  Only keep floor1 and res2.  
    - Special case all the transform stuff to the two blocksizes used (if you use two of-em at all).
    - You can also make smaller or algoritmically decodable Huffman tables instead of the big lookups, 
    if you so choose (you'll have to tune the encoder to generate correct streams for you, in that case, though).  
    - You can even hardwire the modes into the decoder (no headers needed anymore --> good for small samples, no setup
    overhead).

    - Remove floor 0: Floor 0 is not to be considered deprecated, but it is of
      limited modern use. No known Vorbis encoder past Xiph.Org's own beta
      4 makes use of floor 0. Floor 1 is also considerably less expensive to
      decode than floor 0 [9]. Removing support for floor 0 will preserve
      memory.
    - Window sizes: Remove all look-up tables for IMDCT support of
      window lengths other than 256 and 2048. This is possible since these are
      the only windows lengths that is used by most (if not all) encoders. Since
      the current Vorbis standard features window lengths of up to 8192
      samples the memory preserve is significant.
    - Low accuracy: If Tremor is compiled with _LOW_ACCURACY_
      defined it will run in low accuracy mode, which means that all
      calculations during decode is limited to a maximum of 32 bit precision.
      Otherwise, 64 bit values are used to store intermediate 32-bit multiply
      results. And the look up tables for windows and IMDCT are converted
      from 32 bits to 8 bits. Sound quality will be reduced but the processor
      load will be reduced.
    - Rewrite data structures: To further reduce memory usage, and possibly
      gain some speed, data structures can be optimized by determining the
      actual needed sizes as well as removing unnecessary or redundant
      members. And due to the alignment restrictions of some processor
      architectures, struct members can be rearranged to not wasted memory
      due to padding. For example. when an odd number of int is followed by a
      long. This has also been done in a preciously thesis projects and it gained
      some memory [2].


    ***************************************************************************************************************
    ***************************************************************************************************************
    ***************************************************************************************************************
*/


namespace FMOD
{


FMOD_CODEC_DESCRIPTION_EX tremorcodec;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetCodecDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_CODEC_DESCRIPTION_EX * F_API FMODGetCodecDescriptionEx()
    {
        return CodecTremor::getDescriptionEx();
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
size_t FMOD_Tremor_ReadCallback(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    FMOD_RESULT result;
    unsigned int rd;
	File *fp;
    
    fp = (File *)datasource;
	
    result = fp->read(ptr, (unsigned int)size, (unsigned int)nmemb, &rd);
	if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
    {
        return (size_t)-1;
    }

	return rd;
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
int FMOD_Tremor_SeekCallback(void *datasource, ogg_int64_t offset, int whence)
{
	File *fp = (File *)datasource;

    if (fp->mSeekable)
    {
	    return fp->seek((int)offset, (signed char)whence);
    }
    else
    {
        return -1;
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
int FMOD_Tremor_TellCallback(void *datasource)
{
	File *fp = (File *)datasource;
    unsigned int pos;

    fp->tell(&pos);

	return (ogg_int32_t)pos;
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
void * FMOD_Tremor_Malloc(int size)
{
    return FMOD_Memory_Alloc(size);
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
void * FMOD_Tremor_Calloc(int count, int size)
{
    return FMOD_Memory_Calloc(count * size);
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
void * FMOD_Tremor_ReAlloc(void *ptr, int size)
{
    return FMOD_Memory_ReAlloc(ptr, size);
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
void   FMOD_Tremor_Free(void *ptr)
{
    FMOD_Memory_Free(ptr);
}

extern "C"
{
    void * (*_ogg_malloc)  (int size)            = FMOD_Tremor_Malloc;
    void * (*_ogg_calloc)  (int count, int size) = FMOD_Tremor_Calloc;
    void * (*_ogg_realloc) (void *ptr, int size) = FMOD_Tremor_ReAlloc;
    void   (*_ogg_free)    (void *ptr)           = FMOD_Tremor_Free;
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
FMOD_CODEC_DESCRIPTION_EX *CodecTremor::getDescriptionEx()
{
    memset(&tremorcodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    tremorcodec.name        = "FMOD Tremor Codec";
    tremorcodec.version     = 0x00010100;
    tremorcodec.timeunits   = FMOD_TIMEUNIT_PCM;
    tremorcodec.open        = &CodecTremor::openCallback;
    tremorcodec.close       = &CodecTremor::closeCallback;
    tremorcodec.read        = &CodecTremor::readCallback;
    tremorcodec.setposition = &CodecTremor::setPositionCallback;

    tremorcodec.mType       = FMOD_SOUND_TYPE_OGGVORBIS;
    tremorcodec.mSize       = sizeof(CodecTremor);

    return &tremorcodec;
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
FMOD_RESULT CodecTremor::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    FMOD_RESULT     result = FMOD_OK;
    vorbis_info     *vi;
    char            str[4];
    unsigned int    lengthbytes = 0;
    int             oggresult;
	ov_callbacks callbacks = 
	{
		FMOD_Tremor_ReadCallback, /* size_t (*read_func)  (void *ptr, size_t size, size_t nmemb, void *datasource); */
		FMOD_Tremor_SeekCallback, /* int    (*seek_func)  (void *datasource, ogg_int64_t offset, int whence); */
		0,                        /* int    (*close_func) (void *datasource); */
		FMOD_Tremor_TellCallback, /* int    (*tell_func)  (void *datasource); */
	};
    bool manualsizecalc = false;

    init(FMOD_SOUND_TYPE_OGGVORBIS);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecTremor::openInternal", "attempting to open as OGG..\n"));

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

	mSrcDataOffset = 0;

	/*
        Support RIFF wrapped MP3?
    */
    {
        CodecWav tempwav;
        WAVE_CHUNK chunk;
        FMOD_CODEC_WAVEFORMAT tempwaveformat;
        
        memset(&tempwav, 0, sizeof(CodecWav));
        memset(&tempwaveformat, 0, sizeof(FMOD_CODEC_WAVEFORMAT));

        tempwav.mFile = mFile;
        tempwav.mSrcDataOffset = (unsigned int)-1;
        tempwav.waveformat = &tempwaveformat;
	    
	    /*
            Read header
        */
        result = mFile->read(&chunk, 1, sizeof(WAVE_CHUNK), 0);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (!FMOD_strncmp((const char *)chunk.id, "RIFF", 4))
        {
            char wave[4];
            
            result = mFile->read(wave, 1, 4, 0);
            if (result != FMOD_OK)
            {
                return result;
            }

            if (!FMOD_strncmp(wave, "WAVE", 4))
            {
                #ifdef PLATFORM_ENDIAN_BIG
                chunk.size = FMOD_SWAPENDIAN_DWORD(chunk.size);
                #endif
            
                result = tempwav.parseChunk(chunk.size);
                if (result == FMOD_OK && tempwav.mSrcFormat && tempwav.mSrcDataOffset == (unsigned int)-1)
                {
                    int format = tempwav.mSrcFormat->Format.wFormatTag;

                    if (format == WAVE_FORMAT_OGGVORBIS)
                    {
                        mSrcDataOffset          = tempwav.mSrcDataOffset;
                        lengthbytes             = tempwav.waveformat[0].lengthbytes;
                        mLoopPoints[0]          = tempwav.mLoopPoints[0];
                        mLoopPoints[1]          = tempwav.mLoopPoints[1];
                        mSyncPoint              = tempwav.mSyncPoint;
                        mNumSyncPoints          = tempwav.mNumSyncPoints;
                    }
                    else
                    {
    		            result = FMOD_ERR_FORMAT;
                    }

                    if (result != FMOD_OK)
                    {
	                    if (tempwav.mSrcFormat)
                        {
		                    FMOD_Memory_Free(tempwav.mSrcFormat);
                            tempwav.mSrcFormat = 0;
                        }  
                        if (tempwav.mSyncPoint && mSyncPoint != tempwav.mSyncPoint)
                        {
		                    FMOD_Memory_Free(tempwav.mSyncPoint);
                            tempwav.mSyncPoint = 0;
                        }
                        return result;
                    }
                }

	            if (tempwav.mSrcFormat)
                {
		            FMOD_Memory_Free(tempwav.mSrcFormat);
                    tempwav.mSrcFormat = 0;
                }                 
                if (tempwav.mSyncPoint && mSyncPoint != tempwav.mSyncPoint)
                {
                    FMOD_Memory_Free(tempwav.mSyncPoint);
                    tempwav.mSyncPoint = 0;
                }
            }
        }

        result = mFile->seek(mSrcDataOffset, SEEK_SET);
    }

    result = mFile->read(str, 1, 4, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

	if (FMOD_strncmp(str, "OggS", 4))
	{
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecTremor::openInternal", "failed to open as ogg\n"));

		return FMOD_ERR_FORMAT;
	}

	/*
        Get size of ogg file in bytes
    */
	/*
        If there wasnt a riff size chunk
    */
	if (!lengthbytes)
	{
	    result = mFile->getSize(&lengthbytes);
        if (result != FMOD_OK)
        {
            return result;
        }

        manualsizecalc = true;
    }

	result = mFile->seek(mSrcDataOffset, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    oggresult = ov_open_callbacks(mFile, &mVf, 0, 0, callbacks);
	if (oggresult < 0)
	{
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecTremor::openInternal", "failed to open as ogg\n"));

        if (oggresult == -139) //OV_EMEMORY)
        {
            return FMOD_ERR_MEMORY;
        }
		return FMOD_ERR_FORMAT;
	}

	vi = ov_info(&mVf,-1);

    result = readVorbisComments();
    if (result != FMOD_OK)
    {
        return result;
    }

    waveformat = &mWaveFormat;
    waveformat[0].lengthbytes = lengthbytes;
    waveformat[0].format     = FMOD_SOUND_FORMAT_PCM16;
	waveformat[0].channels   = vi->channels;
    waveformat[0].frequency  = vi->rate;
    waveformat[0].blockalign = waveformat[0].channels * 2;                    /* THIS SHOULD BE EQUIVALENT TO THE OGG DECODE BUFFER SIZE? */
 
    if (manualsizecalc && waveformat[0].lengthbytes != (unsigned int)-1)
    {
        waveformat[0].lengthbytes -= mSrcDataOffset;
    }

    if (mFile->mSeekable)
    {
        int count, streams = ov_streams(&mVf);

        waveformat[0].lengthpcm = 0;

        for (count = 0; count < streams; count++)
        {
	        waveformat[0].lengthpcm += (int)ov_pcm_total(&mVf, count);
        }

	    if (waveformat[0].lengthpcm <= 0)
	    {
            FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecTremor::openInternal", "failed to open as ogg\n"));
    
            waveformat[0].lengthpcm = 0;

		    return FMOD_ERR_FORMAT;
	    }
    }
    else
    {
        waveformat[0].lengthpcm = 0x7fffffff;
    }

    if (!mSrcDataOffset)
    {
	    mSrcDataOffset = (unsigned int)ov_raw_tell(&mVf);
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
FMOD_RESULT CodecTremor::closeInternal()
{       
    mVf.datasource = 0;		/* this stops vorbis from trying to close our file */

    ov_clear(&mVf);
    
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
FMOD_RESULT CodecTremor::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    int bigendian;
    vorbis_comment *vc;
    char *name, *value;
    int i;

#ifdef PLATFORM_ENDIAN_BIG
    bigendian = 1;
#else
    bigendian = 0;
#endif

    *bytesread = ov_read(&mVf, buffer, sizebytes, 0);
    if (!*bytesread)
    {
        return FMOD_ERR_FILE_EOF;
    }

    vc  = ov_comment(&mVf, -1);

    if (vc && vc->comments)
    {
        for (i=0;i < vc->comments;i++)
        {
            name = vc->user_comments[i];
            value = name;
            for (;*value && (*value != '=');value++) ;

            if (*value == '=')
            {
                *value++ = 0;
            }
            else
            {
                value = name;
                name  = "NONAME";
            }

            metadata((FMOD_CODEC_STATE *)this, FMOD_TAGTYPE_VORBISCOMMENT, name, value, FMOD_strlen(value) + 1, FMOD_TAGDATATYPE_STRING, true);
        }

#ifndef FMOD_MODIFIED
        vorbis_comment_clear(vc);
#endif
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
FMOD_RESULT CodecTremor::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result = FMOD_OK;

    if (ov_pcm_seek(&mVf, position) < 0)
    {
        return FMOD_ERR_MEMORY;
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
FMOD_RESULT CodecTremor::readVorbisComments()
{
    FMOD_RESULT result;
    char *p;
    int count, i;
    vorbis_comment *vc;

    vc = ov_comment(&mVf, -1);
    if (!vc)
    {
        return FMOD_OK;
    }

    for (count = 0; count < vc->comments; count++)
    {
        if (vc->comment_lengths[count])
        {
            p = vc->user_comments[count];
            for (i=0;*p && (*p != '=');i++, p++) ;
            if (*p == '=')
            {
                *p++ = 0;
                result = metaData(FMOD_TAGTYPE_VORBISCOMMENT, vc->user_comments[count], p, FMOD_strlen(p) + 1, FMOD_TAGDATATYPE_STRING, false);
                if (result != FMOD_OK)
                {
                    return result;
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
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT F_CALLBACK CodecTremor::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecTremor *ogg = (CodecTremor *)codec;

    return ogg->openInternal(usermode, userexinfo);
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
FMOD_RESULT F_CALLBACK CodecTremor::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecTremor *ogg = (CodecTremor *)codec;

    return ogg->closeInternal();
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
FMOD_RESULT F_CALLBACK CodecTremor::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecTremor *ogg = (CodecTremor *)codec;

    return ogg->readInternal(buffer, sizebytes, bytesread);
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
FMOD_RESULT F_CALLBACK CodecTremor::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecTremor *ogg = (CodecTremor *)codec;

    return ogg->setPositionInternal(subsound, position, postype);
}

}

#endif


