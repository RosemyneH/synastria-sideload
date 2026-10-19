#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_OGGVORBIS

#include "fmod.h"
#include "fmod_codec_oggvorbis.h"
#include "fmod_codec_wav.h"
#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_metadata.h"
#include "fmod_string.h"

#include "../lib/ogg_vorbis/vorbis/lib/window.h"

#include <string.h>


// VERIFY: 9be74945 auto
// OFFSET: 0x0090E3D0
void * FMOD_OggVorbis_Malloc(int size)
{
    return FMOD_Memory_Alloc(size);
}
// VERIFY: 0892c210 auto
// OFFSET: 0x0090E400
void * FMOD_OggVorbis_Calloc(int count, int size)
{
    return FMOD_Memory_Calloc(count * size);
}
// VERIFY: 3f0a43cb auto
// OFFSET: 0x0090E430
void * FMOD_OggVorbis_ReAlloc(void *ptr, int size)
{
    return FMOD_Memory_ReAlloc(ptr, size);
}
// VERIFY: 5a2400a6 auto
// OFFSET: 0x0090E460
void   FMOD_OggVorbis_Free(void *ptr)
{
    FMOD_Memory_Free(ptr);
}


namespace FMOD
{


FMOD_CODEC_DESCRIPTION_EX oggvorbiscodec;

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
        return CodecOggVorbis::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */

/*
    Globals
*/

bool            CodecOggVorbis::gInitialized = false;

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
// VERIFY: 4c6e7f66 auto
// OFFSET: 0x0090E350
size_t FMOD_OggVorbis_ReadCallback(void *ptr, size_t size, size_t nmemb, void *datasource)
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

	[SEE_ALSO]
]
*/
// VERIFY: f33e8a9a auto
// OFFSET: 0x0090E380
int FMOD_OggVorbis_SeekCallback(void *datasource, ogg_int64_t offset, int whence)
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

	[SEE_ALSO]
]
*/
// VERIFY: 274de2d6 auto
// OFFSET: 0x0090E3B0
ogg_int32_t FMOD_OggVorbis_TellCallback(void *datasource)
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

	[SEE_ALSO]
]
*/
// VERIFY: 6a455e86 manual
// OFFSET: 0x0090EB40
FMOD_CODEC_DESCRIPTION_EX *CodecOggVorbis::getDescriptionEx()
{
    FMOD_memset(&oggvorbiscodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    oggvorbiscodec.name        = "FMOD Ogg Vorbis Codec";
    oggvorbiscodec.version     = 0x00010100;
    oggvorbiscodec.timeunits   = FMOD_TIMEUNIT_PCM;
    oggvorbiscodec.open        = &CodecOggVorbis::openCallback;
    oggvorbiscodec.close       = &CodecOggVorbis::closeCallback;
    oggvorbiscodec.read        = &CodecOggVorbis::readCallback;
    oggvorbiscodec.setposition = &CodecOggVorbis::setPositionCallback;

    oggvorbiscodec.mType       = FMOD_SOUND_TYPE_OGGVORBIS;
    oggvorbiscodec.mSize       = sizeof(CodecOggVorbis);

    return &oggvorbiscodec;
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
// VERIFY: 3188a20b auto
// OFFSET: 0x0090E6D0
FMOD_RESULT CodecOggVorbis::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    FMOD_RESULT     result = FMOD_OK;
    vorbis_info     *vi;
    char            str[4];
    unsigned int    lengthbytes = 0;
    int             oggresult;
	ov_callbacks callbacks = 
	{
		FMOD_OggVorbis_ReadCallback, /* size_t (*read_func)  (void *ptr, size_t size, size_t nmemb, void *datasource); */
		FMOD_OggVorbis_SeekCallback, /* int    (*seek_func)  (void *datasource, ogg_int64_t offset, int whence); */
		0,                           /* int    (*close_func) (void *datasource); */
		FMOD_OggVorbis_TellCallback, /* int    (*tell_func)  (void *datasource); */
	};
    bool manualsizecalc = false;

    init(FMOD_SOUND_TYPE_OGGVORBIS);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecOggVorbis::openInternal", "attempting to open as OGG..\n"));

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
        
        FMOD_memset(&tempwav, 0, sizeof(CodecWav));
        FMOD_memset(&tempwaveformat, 0, sizeof(FMOD_CODEC_WAVEFORMAT));

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
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecOggVorbis::openInternal", "failed to open as ogg\n"));

		return FMOD_ERR_FORMAT;
	}

    if (!gInitialized)
    {
        _FMOD_vorbis_window_init();
        gInitialized = true;
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
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecOggVorbis::openInternal", "failed to open as ogg, format error.\n"));

        if (oggresult == OV_EMEMORY)
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
            FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecOggVorbis::openInternal", "failed to open as ogg\n"));
    
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

	[SEE_ALSO]
]
*/
FMOD_RESULT CodecOggVorbis::closeInternal()
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

	[SEE_ALSO]
]
*/
// VERIFY: 9015a491 manual
// OFFSET: 0x0090E490
FMOD_RESULT CodecOggVorbis::readInternal(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
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

    *bytesread = ov_read(&mVf, (char *)buffer, sizebytes, bigendian, 2, 1, 0);
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
                name  = (char *)"NONAME";
            }

            metadata((FMOD_CODEC_STATE *)this, FMOD_TAGTYPE_VORBISCOMMENT, name, value, FMOD_strlen(value) + 1, FMOD_TAGDATATYPE_STRING, true);
        }

        FMOD_vorbis_comment_clear(vc);
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
FMOD_RESULT CodecOggVorbis::setPositionInternal(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    int ret = ov_pcm_seek(&mVf, position);
    if (ret < 0)
    {
        return FMOD_ERR_MEMORY;
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
// VERIFY: 91105ca1 manual
// OFFSET: 0x0090E590
FMOD_RESULT CodecOggVorbis::readVorbisComments()
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

	[SEE_ALSO]
]
*/
// VERIFY: 75cc730a manual
// OFFSET: 0x0090EB00
FMOD_RESULT F_CALLBACK CodecOggVorbis::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecOggVorbis *ogg = (CodecOggVorbis *)codec;

    return ogg->openInternal(usermode, userexinfo);
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
// VERIFY: 95dd5315 manual
// OFFSET: 0x0090E630
FMOD_RESULT F_CALLBACK CodecOggVorbis::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecOggVorbis *ogg = (CodecOggVorbis *)codec;

    return ogg->closeInternal();
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
// VERIFY: 2cc1a8f1 auto
// OFFSET: 0x0090E660
FMOD_RESULT F_CALLBACK CodecOggVorbis::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    CodecOggVorbis *ogg = (CodecOggVorbis *)codec;

    return ogg->readInternal(buffer, sizebytes, bytesread);
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
// VERIFY: 25e6bef1 manual
// OFFSET: 0x0090E690
FMOD_RESULT F_CALLBACK CodecOggVorbis::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    CodecOggVorbis *ogg = (CodecOggVorbis *)codec;

    return ogg->setPositionInternal(subsound, position, postype);
}




}

#endif


