#include "fmod_settings.h"

#include "fmod_codeci.h"
#include "fmod_file.h"
#include "fmod_metadata.h"
#include "fmod_soundi.h"
#include "fmod_file_disk.h"
namespace FMOD
{

#ifndef PLATFORM_PS3_SPU
// VERIFY: f3700c7c auto
// OFFSET: 0x0091D2D0
bool Codec::readyForRelease()
{
    if (mFile && !mFile->readyForClose())
    {
        return false;
    }

    return true;
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
// VERIFY: 0844ff2a auto
// OFFSET: 0x0091D2F0
FMOD_RESULT Codec::release()
{
    FMOD_RESULT result;

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "Codec::release", "\n"));

    if (mDescription.close)
    {
        mDescription.close(this);
    }

    if (mFile)
    {
        FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "Codec::release", "Close file (mFile = %p)\n", mFile));

        mFile->close();

        FMOD_Memory_Free(mFile);
        mFile = 0;
    }

    if (mWaveFormatMemory && mType == FMOD_SOUND_TYPE_FSB)
    {
        FMOD_Memory_Free(mWaveFormatMemory);
        mWaveFormatMemory = 0;
    }
    
    if (mMetadata)
    {
        mMetadata->release();
        mMetadata = 0;
    }
    
    result = Plugin::release();

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "Codec::release", "done\n"));

    return result;
}

#endif // !PLATFORM_PS3_SPU

/*
[
	[DESCRIPTION]
    This is just a helper function for codecs if mPCMBuffer and mPCMBufferLengthBytes have been allocated inside the codec code.
    It simply calls the decode in a loop, feeding the output buffer from a smaller pcm decode buffer (mPCMBuffer). 
    Codecs that want to skip offline buffering and write directly to the output pointer (ie raw 1:1 pcm that just does an fread for example)
    should not allocate those variables.

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/
// VERIFY: 82ec2bee manual
// OFFSET: 0x0091DD50
FMOD_RESULT Codec::read(void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
    FMOD_RESULT result = FMOD_OK;
    bool checkmetadata = false;
    unsigned int read = 0;

    if (mPCMBuffer && mPCMBufferLengthBytes)
    {
        while (sizebytes)
        {
            unsigned int lenbytes = sizebytes;
            unsigned int bytesdecoded = 0;

            if (!mPCMBufferOffsetBytes)
            {
                result = mDescription.read(this, mPCMBuffer, mPCMBufferLengthBytes, &bytesdecoded);
                if (result != FMOD_OK)
                {
                    break;
                }

                mPCMBufferFilledBytes = bytesdecoded;

                lenbytes = bytesdecoded;
                if (lenbytes > sizebytes)
                {
                    lenbytes = sizebytes;
                }

                checkmetadata = true;
            }
            
            if (mPCMBufferOffsetBytes + lenbytes > mPCMBufferFilledBytes)
            {
                lenbytes = mPCMBufferFilledBytes - mPCMBufferOffsetBytes;
            }

            FMOD_memcpy((char *)buffer + read, mPCMBuffer + mPCMBufferOffsetBytes, lenbytes);
          

            mPCMBufferOffsetBytes += lenbytes;
            if (mPCMBufferOffsetBytes >= mPCMBufferFilledBytes)
            {
                mPCMBufferOffsetBytes = 0;
            }
            
            if (!lenbytes)
            {
                lenbytes = sizebytes;
                break;
            }

            sizebytes -= lenbytes;
            read += lenbytes;
        }
    }
    else
    {
        result = mDescription.read(this, buffer, sizebytes, &read);
        if (result == FMOD_OK)
        {
            checkmetadata = true;
        }
    }

    #ifndef PLATFORM_PS3_SPU
    if (checkmetadata)
    {
        getMetadataFromFile();
    }
    #endif

    if (bytesread)
    {
        *bytesread = read;
    }

    return result;
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
// VERIFY: f41fe9ef manual
// OFFSET: 0x0091DCD0
FMOD_RESULT Codec::getMetadataFromFile()
{
    FMOD_RESULT  result = FMOD_OK;
    Metadata    *filemetadata;

    if (mFile)
    {
        result = mFile->getMetadata(&filemetadata);
        if (result == FMOD_OK)
        {
            if (!mMetadata)
            {
                mMetadata = FMOD_Object_Alloc(Metadata);
                if (!mMetadata)
                {
                    return FMOD_ERR_MEMORY;
                }
            }

            result = mMetadata->add(filemetadata);
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
// VERIFY: cdbc6dbf auto
// OFFSET: 0x0091D390
FMOD_RESULT Codec::getLength(unsigned int *length, FMOD_TIMEUNIT lengthtype)
{
    FMOD_RESULT result;

    if (lengthtype == FMOD_TIMEUNIT_RAWBYTES)
    {
        FMOD_CODEC_WAVEFORMAT waveformat;

        result = mDescription.getwaveformat(this, mSubSoundIndex, &waveformat);
        if (result != FMOD_OK)
        {
            return result;
        }

        *length = waveformat.lengthbytes;

        return FMOD_OK;
    }

    if (!mDescription.getlength)
    {
        *length = 0;
        return FMOD_ERR_UNSUPPORTED;
    }

    result = mDescription.getlength(this, length, lengthtype);
    if (result != FMOD_OK)
    {
        return result;
    }

    return FMOD_OK;
}

#endif // !PLATFORM_PS3_SPU

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
// VERIFY: df3c4bc9 manual
// OFFSET: 0x0091D4B0
FMOD_RESULT Codec::setPosition(int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result;
    FMOD_CODEC_WAVEFORMAT waveformat;

    if (numsubsounds && subsound >= numsubsounds)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!mDescription.setposition)
    {
        return FMOD_ERR_UNSUPPORTED;
    }

    if (subsound < 0)
    {
        subsound = mSubSoundIndex;
    }

    result = mDescription.getwaveformat(this, subsound, &waveformat);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        If the incoming timeformat is not a codec friendly format, convert it to the 
        codec's favourite time format.  (for convertable formats like ms/pcm/bytes)
    */
    if (mDescription.timeunits & FMOD_TIMEUNIT_PCM)
    {
        if (postype & FMOD_TIMEUNIT_PCMBYTES)
        {
            SoundI::getSamplesFromBytes(position, &position, waveformat.channels, waveformat.format);
            postype = FMOD_TIMEUNIT_PCM;
        }
        else if (postype & FMOD_TIMEUNIT_MS)
        {
            position = (unsigned int)((float)position / 1000.0f * waveformat.frequency);
            postype = FMOD_TIMEUNIT_PCM;            
        }
    }
    else if (mDescription.timeunits & FMOD_TIMEUNIT_PCMBYTES)
    {
        if (postype & FMOD_TIMEUNIT_PCM)
        {
            SoundI::getBytesFromSamples(position, &position, waveformat.channels, waveformat.format);
            postype = FMOD_TIMEUNIT_PCMBYTES;
        }
        else if (postype & FMOD_TIMEUNIT_MS)
        {
            position = (unsigned int)((float)position / 1000.0f * waveformat.frequency);
            SoundI::getBytesFromSamples(position, &position, waveformat.channels, waveformat.format);
            postype = FMOD_TIMEUNIT_PCMBYTES;            
        }
    }
    else if (mDescription.timeunits & FMOD_TIMEUNIT_MS)
    {
        if (postype & FMOD_TIMEUNIT_PCM)
        {
            position = (unsigned int)((float)position / waveformat.frequency * 1000.0f);
            postype = FMOD_TIMEUNIT_MS;            
        }
        else if (postype & FMOD_TIMEUNIT_PCMBYTES)
        {
            SoundI::getSamplesFromBytes(position, &position, waveformat.channels, waveformat.format);
            position = (unsigned int)((float)position / waveformat.frequency * 1000.0f);
            postype = FMOD_TIMEUNIT_MS;
        }
    }

    if (!(postype & mDescription.timeunits))
    {
        return FMOD_ERR_FORMAT;
    }

    mPCMBufferOffsetBytes = 0;

    result = mDescription.setposition(this, subsound, position, postype);
    if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
    {
        return result;
    }

    mSubSoundIndex = subsound;

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
// VERIFY: 5b95f9b4 auto
// OFFSET: 0x0091D400
FMOD_RESULT Codec::getPosition(unsigned int *position, FMOD_TIMEUNIT postype)
{
    FMOD_RESULT result;

    if (postype == FMOD_TIMEUNIT_RAWBYTES)
    {
        if (!mFile)
        {
            *position = 0;
        }

        result = mFile->tell(position);
        if (result != FMOD_OK)
        {
            *position = 0;
            return result;
        }

        *position -= mSrcDataOffset;
    }

    if (!mDescription.getposition)
    {
        return FMOD_ERR_UNSUPPORTED;
    }

    if (!(postype & mDescription.timeunits))
    {
        return FMOD_ERR_FORMAT;
    }

    result = mDescription.getposition(this, position, postype);
    if (result != FMOD_OK)
    {
        return result;
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
    FMOD_TAGDATATYPE
]
*/
// VERIFY: a8203f07 manual
// OFFSET: 0x0091DC30
FMOD_RESULT Codec::metaData(FMOD_TAGTYPE type, const char *name, void *data, unsigned int datalen, FMOD_TAGDATATYPE datatype, bool unique)
{
    if (!mMetadata)
    {
        mMetadata = FMOD_Object_Alloc(Metadata);
        if (!mMetadata)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    return mMetadata->addTag(type, name, data, datalen, datatype, unique);
}

#endif // !PLATFORM_PS3_SPU


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



/*
[API]
[
	[DESCRIPTION]
    Open callback for the codec for when FMOD tries to open a sound using this codec.
    This is the callback the file format check is done in, codec related memory is allocated, and things are generally initialized / set up for the codec.

	[PARAMETERS]
	'codec_state'   Pointer to the codec state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
    'usermode'      Mode that the user supplied via System::createSound.  This is informational and can be ignored, or used if it has relevance to your codec.
    'userexinfo'    Extra info structure that the user supplied via System::createSound.  This is informational and can be ignored, or used if it has relevance to your codec.

	[RETURN_VALUE]

	[REMARKS]
    The usermode and userexinfo parameters tell the codec what was passed in by the user.<br>
    Generally these can be ignored, as the file format usually determines the format and frequency of the sound.<br>
    <br>
    If you have a flexible format codec (ie you don't mind what output format your codec writes to), you might want to use the parameter that was passed in by the user to specify the output sound format / frequency.<br>
    For example if you normally create a codec that is always 32bit floating point, the user might supply 16bit integer to save memory, so you could use this information to decode your data to this format instead of the original default format.
    <br>
    Read and seek within the file using the 'fileread' and 'fileseek' members of the FMOD_CODEC codec that is passed in.<br>
    Note: <b>DO NOT USE YOUR OWN FILESYSTEM.</b><br>
    The reasons for this are:<br>
    <li>The user may have set their own file system via user filesystem callbacks.<br>
    <li>FMOD allows file reading via disk, memory and TCP/IP.  If you use your own file routines you will lose this ability.<br>
    <br>
    <b>Important!</b>  FMOD will ping all codecs trying to find the right one for the file the user has passed in.  Make sure the first line of your codec open is a FAST format check.  Ie it reads an identifying string, checks it and returns an error FMOD_ERR_FORMAT if it is not found.<br>
    There may be a lot of codecs loaded into FMOD, so you don't want yours slowing down the System::createSound call because it is inneficient in determining if it is the right format or not.<br>
    <br>
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::createSound
    FMOD_CREATESOUNDEXINFO
    FMOD_CODEC_STATE
    FMOD_CODEC_DESCRIPTION
    FMOD_CODEC_CLOSECALLBACK
    FMOD_CODEC_READCALLBACK
    FMOD_CODEC_GETLENGTHCALLBACK
    FMOD_CODEC_SETPOSITIONCALLBACK
    FMOD_CODEC_GETPOSITIONCALLBACK
    FMOD_CODEC_SOUNDCREATECALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_CODEC_OPENCALLBACK(FMOD_CODEC_STATE *codec_state, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Close callback for the codec for when FMOD tries to close a sound using this codec.<br>
    This is the callback any codec related memory is freed, and things are generally de-initialized / shut down for the codec.

	[PARAMETERS]
	'codec_state'   Pointer to the codec state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.

	[RETURN_VALUE]

	[REMARKS]
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    FMOD_CODEC_STATE
    FMOD_CODEC_DESCRIPTION
    FMOD_CODEC_OPENCALLBACK
    FMOD_CODEC_READCALLBACK
    FMOD_CODEC_GETLENGTHCALLBACK
    FMOD_CODEC_SETPOSITIONCALLBACK
    FMOD_CODEC_GETPOSITIONCALLBACK
    FMOD_CODEC_SOUNDCREATECALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_CODEC_CLOSECALLBACK(FMOD_CODEC_STATE *codec_state)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Read callback for the codec for when FMOD tries to read some data from the file to the destination format (format specified in the open callback).

	[PARAMETERS]
	'codec_state'   Pointer to the codec state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
	'buffer'        Buffer to read PCM data to.  Note that the format of this data is the format described in FMOD_CODEC_WAVEFORMAT.
    'sizebytes'     Number of bytes to read
    'bytesread'     Number of bytes actually read

	[RETURN_VALUE]

	[REMARKS]
    If you cannot read number of bytes requested, simply return FMOD_OK and give bytesread the number of bytes you read.
    <br>
    Read and seek within the file using the 'fileread' and 'fileseek' members of the FMOD_CODEC codec that is passed in.<br>
    Note: <b>DO NOT USE YOUR OWN FILESYSTEM.</b><br>
    The reasons for this are:<br>
    <li>The user may have set their own file system via user filesystem callbacks.<br>
    <li>FMOD allows file reading via disk, memory and TCP/IP.  If you use your own file routines you will lose this ability.<br>
    <br>
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    FMOD_CODEC_STATE
    FMOD_CODEC_DESCRIPTION
    FMOD_CODEC_OPENCALLBACK
    FMOD_CODEC_CLOSECALLBACK
    FMOD_CODEC_GETLENGTHCALLBACK
    FMOD_CODEC_SETPOSITIONCALLBACK
    FMOD_CODEC_GETPOSITIONCALLBACK
    FMOD_CODEC_SOUNDCREATECALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_CODEC_READCALLBACK(FMOD_CODEC_STATE *codec_state, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Callback to return the length of the song in whatever format required when Sound::getLength is called.

	[PARAMETERS]
	'codec_state'   Pointer to the codec state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
    'length'        Address of a variable that is to receive the length of the sound determined by the format specified in the lengttype parameter.
    'lengthtype'    Timeunit type of length to return.  This will be one of the timeunits supplied by the codec author in the FMOD_CODEC_DESCRIPTION structure.

	[RETURN_VALUE]

	[REMARKS]
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    FMOD_TIMEUNIT
    FMOD_CODEC_STATE
    FMOD_CODEC_DESCRIPTION
    FMOD_CODEC_OPENCALLBACK
    FMOD_CODEC_CLOSECALLBACK
    FMOD_CODEC_READCALLBACK
    FMOD_CODEC_SETPOSITIONCALLBACK
    FMOD_CODEC_GETPOSITIONCALLBACK
    FMOD_CODEC_SOUNDCREATECALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_CODEC_GETLENGTHCALLBACK(FMOD_CODEC_STATE *codec_state, unsigned int *length, FMOD_TIMEUNIT lengthtype)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/
/*
[API]
[
	[DESCRIPTION]
    Seek callback for the codec for when FMOD tries to seek within the file with Channel::setPosition.

	[PARAMETERS]
	'codec_state'   Pointer to the codec state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
	'subsound'      Subsound within which to seek.
    'position'      Position to seek to in the sound based on the timeunit specified in the postype parameter.
    'postype'       Timeunit type of the position parameter.  This will be one of the timeunits supplied by the codec author in the FMOD_CODEC_DESCRIPTION structure.

	[RETURN_VALUE]

	[REMARKS]
    Read and seek within the file using the 'fileread' and 'fileseek' members of the FMOD_CODEC codec that is passed in.<br>
    Note: <b>DO NOT USE YOUR OWN FILESYSTEM.</b><br>
    The reasons for this are:<br>
    <li>The user may have set their own file system via user filesystem callbacks.<br>
    <li>FMOD allows file reading via disk, memory and TCP/IP.  If you use your own file routines you will lose this ability.<br>
    <br>
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    Channel::setPosition
    FMOD_CODEC_STATE
    FMOD_CODEC_DESCRIPTION
    FMOD_CODEC_OPENCALLBACK
    FMOD_CODEC_CLOSECALLBACK
    FMOD_CODEC_READCALLBACK
    FMOD_CODEC_GETLENGTHCALLBACK
    FMOD_CODEC_GETPOSITIONCALLBACK
    FMOD_CODEC_SOUNDCREATECALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_CODEC_SETPOSITIONCALLBACK(FMOD_CODEC_STATE *codec_state, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/
/*
[API]
[
	[DESCRIPTION]
    Tell callback for the codec for when FMOD tries to get the current position within the with Channel::getPosition.  

	[PARAMETERS]
	'codec_state'   Pointer to the codec state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
    'position'      Address of a variable to receive the current position in the codec based on the timeunit specified in the postype parameter.
    'postype'       Timeunit type of the position parameter that is requested.  This will be one of the timeunits supplied by the codec author in the FMOD_CODEC_DESCRIPTION structure.

	[RETURN_VALUE]

	[REMARKS]
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    Channel::getPosition
    FMOD_CODEC_STATE
    FMOD_CODEC_DESCRIPTION
    FMOD_CODEC_OPENCALLBACK
    FMOD_CODEC_CLOSECALLBACK
    FMOD_CODEC_READCALLBACK
    FMOD_CODEC_GETLENGTHCALLBACK
    FMOD_CODEC_SETPOSITIONCALLBACK
    FMOD_CODEC_SOUNDCREATECALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_CODEC_GETPOSITIONCALLBACK(FMOD_CODEC_STATE *codec_state, unsigned int *position, FMOD_TIMEUNIT postype)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/


/*
[API]
[
	[DESCRIPTION]
    Sound creation callback for the codec when FMOD finishes creating the sound.  Ie so the codec can set more parameters for the related created sound, ie loop points/mode or 3D attributes etc.

	[PARAMETERS]
	'codec_state'   Pointer to the codec state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
	'subsound'      Subsound index being created.
    'sound'         Pointer to the sound being created.

	[RETURN_VALUE]

	[REMARKS]
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::createSound
    System::createStream
    FMOD_CODEC_STATE
    FMOD_CODEC_DESCRIPTION
    FMOD_CODEC_OPENCALLBACK
    FMOD_CODEC_CLOSECALLBACK
    FMOD_CODEC_READCALLBACK
    FMOD_CODEC_GETLENGTHCALLBACK
    FMOD_CODEC_SETPOSITIONCALLBACK
    FMOD_CODEC_GETPOSITIONCALLBACK
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_CODEC_SOUNDCREATECALLBACK(FMOD_CODEC_STATE *codec_state, int subsound, FMOD_SOUND *sound)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/

/*
[API]
[
	[DESCRIPTION]
    Callback for sounds that have their

	[PARAMETERS]
	'codec_state'   Pointer to the codec state.  The user can use this variable to access runtime plugin specific variables and plugin writer user data.
	'type'          Source of tag being updated, ie id3v2 or oggvorbis tag for example.  See FMOD_TAGDATATYPE.
    'name'          Name of the tag being updated.
    'data'          Contents of tag.
    'datalen'       Length of the tag data in bytes.
    'datatype'      Data type of tag.  Binary / string / unicode etc.  See FMOD_TAGDATATYPE.
    'unique'        If this is true, then the tag (determined by the name) being updated is the only one of its type.  If it is false then there are multiple versions of this tag with the same name.

	[RETURN_VALUE]

	[REMARKS]
    This callback is usually called from sounds that can udate their metadata / tag info at runtime.  Such a sound could be an internet SHOUTcast / Icecast stream for example.<br>
    <br>
    Remember to return FMOD_OK at the bottom of the function, or an appropriate error code from FMOD_RESULT.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    FMOD_CODEC_STATE
    FMOD_CODEC_DESCRIPTION
    FMOD_CODEC_OPENCALLBACK
    FMOD_CODEC_CLOSECALLBACK
    FMOD_CODEC_READCALLBACK
    FMOD_CODEC_GETLENGTHCALLBACK
    FMOD_CODEC_SETPOSITIONCALLBACK
    FMOD_CODEC_GETPOSITIONCALLBACK
    FMOD_CODEC_SOUNDCREATECALLBACK
    FMOD_TAGDATATYPE
]
*/
/*
FMOD_RESULT F_CALLBACK FMOD_CODEC_METADATACALLBACK(FMOD_CODEC_STATE *codec_state, FMOD_TAGTYPE type, char *name, void *data, unsigned int datalen, FMOD_TAGDATATYPE datatype, int unique)
{
#if 0
	 // here purely for documentation purposes.
#endif
}
*/

}


