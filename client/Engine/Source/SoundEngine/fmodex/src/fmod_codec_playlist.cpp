#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_PLAYLIST

#include "fmod.h"
#include "fmod_codec_playlist.h"
#include "fmod_debug.h"
#include "fmod_file.h"
#include "fmod_metadata.h"
#include "fmod_string.h"

#include <stdlib.h>

const int MAX_TOKEN_SIZE = 512;

namespace FMOD
{

FMOD_CODEC_DESCRIPTION_EX playlistcodec;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the register plugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_CODEC_DESCRIPTION_EX * F_API FMODGetCodecDescriptionEx()
    {
        return CodecPlaylist::getDescriptionEx();
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
// VERIFY: ee626622 manual
// OFFSET: 0x008F7BC0
FMOD_CODEC_DESCRIPTION_EX *CodecPlaylist::getDescriptionEx()
{
    FMOD_memset(&playlistcodec, 0, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    playlistcodec.name        = "FMOD Playlist Reader Codec";
    playlistcodec.version     = 0x00010100;
    playlistcodec.timeunits   = FMOD_TIMEUNIT_PCM;
    playlistcodec.open        = &CodecPlaylist::openCallback;
    playlistcodec.close       = &CodecPlaylist::closeCallback;
    playlistcodec.read        = &CodecPlaylist::readCallback;
    playlistcodec.setposition = &CodecPlaylist::setPositionCallback;

    playlistcodec.mType       = FMOD_SOUND_TYPE_PLAYLIST;
    playlistcodec.mSize       = sizeof(CodecPlaylist);

    return &playlistcodec;
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
// VERIFY: 95a010bf auto
// OFFSET: 0x008F7990
FMOD_RESULT CodecPlaylist::openInternal(FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    FMOD_RESULT   result = FMOD_ERR_FORMAT;
    char          playlistid[16];

    init(FMOD_SOUND_TYPE_PLAYLIST);

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecPlaylist::openInternal", "attempting to open playlist file\n"));

	result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        Get the first 12 characters to identify whether or
        not this is a playlist file, and which type if it is.
    */
    result = skipWhiteSpace(0);
    if (result != FMOD_OK)
    {
        return result;
    }

    FMOD_memset(playlistid, 0, 16);
    result = mFile->read(playlistid, 12, 1);
    if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF)
    {
        return result;
    }

    if (!FMOD_strnicmp("#EXTM3U", playlistid, 7))
    {
        result = readM3U();
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    else if (!FMOD_strnicmp("[PLAYLIST]", playlistid, 10))
    {
        result = readPLS();
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    else if (!FMOD_strnicmp("<ASX VERSION", playlistid, 12))   // TODO
    {
        result = readASX();
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    /*
        It might be a "simple" pls or m3u which is just a text file with
        a list of files. Check file extension to see if that is the case
    */
    else
    {
        char *filename;
        int   stringlength;

        result = mFile->getName(&filename);
        if (result != FMOD_OK)
        {
            return result;
        }

        stringlength = FMOD_strlen(filename);

        if (!FMOD_strncmp(filename + (stringlength - 4), ".pls", 4) ||
            !FMOD_strncmp(filename + (stringlength - 4), ".m3u", 4) ||
            !FMOD_strncmp(filename + (stringlength - 4), ".asx", 4) ||
            //!FMOD_strncmp(filename + (stringlength - 4), ".wpl", 4) ||
            !FMOD_strncmp(filename + (stringlength - 4), ".wax", 4))
        {
            result = readSimple();
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        else
        {
            return FMOD_ERR_FORMAT;
        }
    }

    FMOD_memset(&mWaveFormat, 0, sizeof(FMOD_CODEC_WAVEFORMAT));

    waveformat = &mWaveFormat;

    numsubsounds = 0;

    FLOG((FMOD_DEBUG_LEVEL_LOG, __FILE__, __LINE__, "CodecPlaylist::openInternal", "open successful\n"));

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
// VERIFY: d9f48159 manual
// OFFSET: 0x008F7470
FMOD_RESULT CodecPlaylist::readASX()
{
    FMOD_RESULT result;

    char tag    [MAX_TOKEN_SIZE];
    char tagdata[MAX_TOKEN_SIZE];

    int tagsize = MAX_TOKEN_SIZE;
    int tagdatasize = MAX_TOKEN_SIZE;
    
    /*
        <Abstract>
        <Title>
        <Author>
        <Copyright>
        <MoreInfo href="path"/>
        <Entry>
        <Duration value="00:00:00">
        <Logo href="path" Style="MARK or ICON"/>
        <Banner href="path"/>
        <Ref href="path"/>
    */

    result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        First tag should be <ASX VERSION="3.0">
    */
    result = getNextXMLTag(tag, &tagsize, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (FMOD_strnicmp("ASX VERSION", tag, 11))
    {
        return FMOD_ERR_FORMAT;
    }
    
    for (;;)
    {
        tagsize = MAX_TOKEN_SIZE;
        tagdatasize = MAX_TOKEN_SIZE;
        
        result = getNextXMLTag(tag, &tagsize, tagdata, &tagdatasize);
        if (result != FMOD_OK)
        {
            break;
        }

        tag[tagsize]         = '\0';
        tagdata[tagdatasize] = '\0';

        if (!FMOD_strnicmp("ENTRY", tag, 5))
        {
            metaData(FMOD_TAGTYPE_PLAYLIST, "ENTRY", 0, 0, FMOD_TAGDATATYPE_STRING, false);
        }
        else if (tagdatasize == 0)
        {
            char url[MAX_TOKEN_SIZE];
            int urlsize = MAX_TOKEN_SIZE;

            result = getQuoteData(tag, url, &urlsize);
            if (result != FMOD_OK)
            {
                return result;
            }

            if (!FMOD_strnicmp("REF HREF", tag, 8))
            {
                metaData(FMOD_TAGTYPE_PLAYLIST, "FILE", url, urlsize + 1, FMOD_TAGDATATYPE_STRING, false);
            }
            else if (!FMOD_strnicmp("MOREINFO HREF", tag, 13))
            {
                metaData(FMOD_TAGTYPE_PLAYLIST, "MOREINFO", url, urlsize + 1, FMOD_TAGDATATYPE_STRING, false);
            }
            else if (!FMOD_strnicmp("DURATION VALUE", tag, 14))
            {
                metaData(FMOD_TAGTYPE_PLAYLIST, "DURATION", url, urlsize + 1, FMOD_TAGDATATYPE_STRING, false);
            }
            else if (!FMOD_strnicmp("LOGO HREF", tag, 9))
            {
                metaData(FMOD_TAGTYPE_PLAYLIST, "LOGO", url, urlsize + 1, FMOD_TAGDATATYPE_STRING, false);
            }
            else if (!FMOD_strnicmp("BANNER HREF", tag, 11))
            {
                metaData(FMOD_TAGTYPE_PLAYLIST, "BANNER", url, urlsize + 1, FMOD_TAGDATATYPE_STRING, false);
            }
        }
        else
        {
            metaData(FMOD_TAGTYPE_PLAYLIST, FMOD_strupr(tag), tagdata, tagdatasize + 1, FMOD_TAGDATATYPE_STRING, false);
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
FMOD_RESULT CodecPlaylist::readWPL()
{
    FMOD_RESULT result;
    char tag    [MAX_TOKEN_SIZE];
    char tagdata[MAX_TOKEN_SIZE];

    int tagsize = MAX_TOKEN_SIZE;
    int tagdatasize = MAX_TOKEN_SIZE;

    result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        First tag should be <?wpl version="1.0"?>
    */
    result = getNextXMLTag(tag, &tagsize, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (FMOD_strnicmp("?WPL VERSION", tag, 12))
    {
        return FMOD_ERR_FORMAT;
    }

    /*
        Skip to the <seq> tag
    */
    do
    {
        tagsize = MAX_TOKEN_SIZE;
        result = getNextXMLTag(tag, &tagsize, 0, 0);

        if (result != FMOD_OK)
        {
            return result;
        }
    }
    while (FMOD_strnicmp("seq", tag, 3));
    
    /*
        Read the data from file
    */
    for (;;)
    {
        tagsize = MAX_TOKEN_SIZE;
        tagdatasize = MAX_TOKEN_SIZE;
        
        result = getNextXMLTag(tag, &tagsize, tagdata, &tagdatasize);
        if (result != FMOD_OK)
        {
            break;
        }
        tag[tagsize] = '\0';
        tagdata[tagdatasize] = '\0';

        if (tagdatasize == 0)
        {
            char url[MAX_TOKEN_SIZE];
            int urlsize = MAX_TOKEN_SIZE;

            getQuoteData(tag, url, &urlsize);
            if (result != FMOD_OK)
            {
                return result;
            }

            if (!FMOD_strnicmp("MEDIA SRC", tag, 8))
            {
                metaData(FMOD_TAGTYPE_PLAYLIST, "FILE", url, urlsize + 1, FMOD_TAGDATATYPE_STRING, false);
            }
        }
        else
        {
            metaData(FMOD_TAGTYPE_PLAYLIST, FMOD_strupr(tag), tagdata, tagdatasize + 1, FMOD_TAGDATATYPE_STRING, false);
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
FMOD_RESULT CodecPlaylist::readB4S()
{
    FMOD_RESULT result;
    char tag    [MAX_TOKEN_SIZE];
    char tagdata[MAX_TOKEN_SIZE];

    int tagsize = MAX_TOKEN_SIZE;
    int tagdatasize = MAX_TOKEN_SIZE;

    result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        First tag should be <?xml version="1.0" encoding='UTF-8' standalone="yes"?>
    */
    result = getNextXMLTag(tag, &tagsize, 0, 0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (FMOD_strnicmp("?XML VERSION", tag, 12))
    {
        return FMOD_ERR_FORMAT;
    }

    /*  
        Find the first entry
    */
    while (FMOD_strnicmp("ENTRY PLAYSTRING=", tag, 16))
    {
        tagsize = MAX_TOKEN_SIZE;
        tagdatasize = MAX_TOKEN_SIZE;
        
        result = getNextXMLTag(tag, &tagsize, tagdata, &tagdatasize);
        if (result != FMOD_OK)
        {
            return FMOD_ERR_FORMAT;
        }
        tag[tagsize] = '\0';
        tagdata[tagdatasize] = '\0';
    }

    /*
        Read the data from file
    */
    for (;;)
    {
        if (tagdatasize == 0)
        {
            char url[MAX_TOKEN_SIZE];
            int urlsize;

            getQuoteData(tag, url, &urlsize);

            if (!FMOD_strnicmp("ENTRY PLAYSTRING=", tag, 17))
            {
                if(!FMOD_strnicmp("FILE:", url, 5)) //Note sometimes B4S paths are "file:c:/..." instead of "c:/..."
                {
                    metaData(FMOD_TAGTYPE_PLAYLIST, "FILE", url + 5, urlsize + 1, FMOD_TAGDATATYPE_STRING, false);
                }
                else
                {
                    metaData(FMOD_TAGTYPE_PLAYLIST, "FILE", url, urlsize + 1, FMOD_TAGDATATYPE_STRING, false);
                }
            }
            else if (!FMOD_strnicmp("NAME", tag, 13))
            {
                metaData(FMOD_TAGTYPE_PLAYLIST, "NAME", url, urlsize + 1, FMOD_TAGDATATYPE_STRING, false);
            }
            else if (!FMOD_strnicmp("LENGTH", tag, 14))
            {
                metaData(FMOD_TAGTYPE_PLAYLIST, "LENGTH", url, urlsize + 1, FMOD_TAGDATATYPE_STRING, false);
            }
        }
        else
        {
            metaData(FMOD_TAGTYPE_PLAYLIST, FMOD_strupr(tag), tagdata, tagdatasize + 1, FMOD_TAGDATATYPE_STRING, false);
        }
        
        tagsize = MAX_TOKEN_SIZE;
        tagdatasize = MAX_TOKEN_SIZE;
        
        result = getNextXMLTag(tag, &tagsize, tagdata, &tagdatasize);
        if (result != FMOD_OK)
        {
            break;
        }
        tag[tagsize] = '\0';
        tagdata[tagdatasize] = '\0';
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]
    Insanely simple parsing of xml for the sole purpose of ASX files.
    Doesn't handle attributes.
    Doesn't handle nesting.

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: a553db4d manual
// OFFSET: 0x008F6DD0
FMOD_RESULT CodecPlaylist::getNextXMLTag(char *tagname, int *tagnamesize, char *tagdata, int *tagdatasize)
{
    FMOD_RESULT     result;
    int             count = 0;
    unsigned char   c = 0;
    int             datasize = 0;

    result = skipWhiteSpace(0);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        Get the tag
    */
    do
    {
        result = mFile->getByte(&c);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    while (c != '<');

    do
    {
        result = mFile->getByte(&c);
        if (result != FMOD_OK)
        {
            return result;
        }

        tagname[count++] = c;
    }
    while (c != '>' && count < *tagnamesize);

    *tagnamesize = count - 1;

    count = 0;

    result = skipWhiteSpace(0);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (tagdatasize)
    {
        datasize = *tagdatasize;
    }

    /*
        Get the tag data
    */
    do
    {
        result = mFile->getByte(&c);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (count < datasize)
        {
            tagdata[count++] = c;
        }
    }
    while (c != '<');

    if (tagdatasize)
    {
        *tagdatasize = count - 1;
    }

    result = mFile->getByte(&c);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        Check if it is a closing tag
    */
    if (c == '/')
    {
        /*
            Its a closing tag, move file pointer to the end of it
        */
        do
        {
            result = mFile->getByte(&c);
            if (result != FMOD_OK)
            {
                return result;
            }
        }
        while (c != '>');
    }
    else
    {
        result = mFile->seek(-2, SEEK_CUR);
        if (result != FMOD_OK)
        {
            return result;
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
// VERIFY: 2df3be07 manual
// OFFSET: 0x008F6EF0
FMOD_RESULT CodecPlaylist::readM3U()
{
    FMOD_RESULT     result;
    int             count = 0;
    unsigned char   c;
    char            token[MAX_TOKEN_SIZE];
    int             length = 0;

    result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        Get the first token, it should be '#EXTM3U'
    */
    do
    {
        result = mFile->getByte(&c);
        if (result != FMOD_OK)
        {
            return result;
        }

        token[count++] = c;
    }
    while (!isNewLine(c));

    if (FMOD_strnicmp(token, "#EXTM3U", 7))
    {
        return FMOD_ERR_FORMAT;
    }

    /*
        #EXTINF:([0-9]*),(.*)
    */
    for (;;)
    {
        count = 0;

        result = skipWhiteSpace(0);
        if (result != FMOD_OK)
        {
            break;
        }   

        /*
            Get #EXTINF:
        */
        do
        {
            result = mFile->getByte(&c);
            if (result != FMOD_OK)
            {
                break;
            }

            token[count++] = c;
        }
        while (c != ':');

        if (FMOD_strnicmp("#EXTINF", token, 7))
        {
            return FMOD_ERR_FORMAT;
        }

        result = skipWhiteSpace(0);
        if (result != FMOD_OK)
        {
            break;
        }   

        /*
            Get LENGTH
        */
        count = 0;
        do
        {
            result = mFile->getByte(&c);
            if (result != FMOD_OK)
            {
                break;
            }

            token[count++] = c;
        }
        while (c != ',');

         // Convert to int
        token[count - 1] = '\0';
        length = atoi(token);

        metaData(FMOD_TAGTYPE_PLAYLIST, "LENGTH", &length, sizeof(int), FMOD_TAGDATATYPE_INT, false);

        result = skipWhiteSpace(0);
        if (result != FMOD_OK)
        {
            break;
        }   

        /*
            Get TITLE
        */
        count = 0;
        do
        {
            result = mFile->getByte(&c);
            if (result != FMOD_OK)
            {
                break;
            }

            if (c != '\n' && c != '\r')
            {
                token[count++] = c;
            }
        }
        while (!isNewLine(c));

        token[count] = '\0';

        metaData(FMOD_TAGTYPE_PLAYLIST, "TITLE", token, count + 1, FMOD_TAGDATATYPE_STRING, false);

        result = skipWhiteSpace(0);
        if (result != FMOD_OK)
        {
            break;
        }   

        /*
            Get FILE
        */
        count = 0;
        do
        {
            result = mFile->getByte(&c);
            if (result != FMOD_OK)
            {
                break;
            }

            if (c != '\n' && c != '\r')
            {
                token[count++] = c;
            }
        }
        while (!isNewLine(c));

        token[count] = '\0';

        metaData(FMOD_TAGTYPE_PLAYLIST, "FILE", token, count, FMOD_TAGDATATYPE_STRING, false);
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
// VERIFY: dc63259a auto
// OFFSET: 0x008F76F0
FMOD_RESULT CodecPlaylist::readPLS()
{
    FMOD_RESULT   result;
    int           tokensize;
    char          token[MAX_TOKEN_SIZE];

    result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        Get the first token, it should be '[playlist]'
    */
    result = getPLSToken(token, MAX_TOKEN_SIZE, 0);
    if (result != FMOD_OK)
    {
        return FMOD_ERR_FORMAT;
    }
    if (FMOD_strnicmp(token, "[playlist]", 10))
    {
        return FMOD_ERR_FORMAT;
    }

    /*
        Get the playlist information
    */
    for (;;)
    {
        result = getPLSToken(token, MAX_TOKEN_SIZE, 0);
        if (result != FMOD_OK)
        {
            break;
        }

        if (!FMOD_strnicmp("File", token, 4))
        {
            result = getPLSToken(token, MAX_TOKEN_SIZE, &tokensize);
            if (result != FMOD_OK)
            {
                break;
            }

            metaData(FMOD_TAGTYPE_PLAYLIST, "FILE", token, tokensize + 1, FMOD_TAGDATATYPE_STRING, false);
        }
        else if (!FMOD_strnicmp("Title", token, 5))
        {
            result = getPLSToken(token, MAX_TOKEN_SIZE, &tokensize);
            if (result != FMOD_OK)
            {
                break;
            }

            metaData(FMOD_TAGTYPE_PLAYLIST, "TITLE", token, tokensize + 1, FMOD_TAGDATATYPE_STRING, false);            
        }
        else if (!FMOD_strnicmp("Length", token, 6))
        {
            int length = 0;

            result = getPLSToken(token, MAX_TOKEN_SIZE, &tokensize);
            if (result != FMOD_OK)
            {
                break;
            }

            // Convert to int
            token[tokensize] = '\0';
            length = atoi(token);

            metaData(FMOD_TAGTYPE_PLAYLIST, "LENGTH", &length, sizeof(int), FMOD_TAGDATATYPE_INT, false);           
        }

        else if (!FMOD_strnicmp("NumberOfEntries", token, 15))
        {
            result = getPLSToken(token, MAX_TOKEN_SIZE, 0);
            if (result != FMOD_OK)
            {
                break;
            }
        }
        else if (!FMOD_strnicmp("Version", token, 7))
        {
            result = getPLSToken(token, MAX_TOKEN_SIZE, 0);
            if (result != FMOD_OK)
            {
                break;
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
// VERIFY: 6772112b manual
// OFFSET: 0x008F7180
FMOD_RESULT CodecPlaylist::getPLSToken(char *buffer, int length, int *tokensize)
{
    FMOD_RESULT     result = FMOD_OK;
    unsigned char   c;
    int             count = 0;
    int             numwhitespaces = 0;

    /*
        [playlist]

        File(0-9)*=
        Title(0-9)*=
        Length(0-9)*=

        NumberOfEntries=
        Version=

        numberofentries=
        version=

        (and data after the '=' sign)
    */

    result = skipWhiteSpace(&numwhitespaces);
    if (result != FMOD_OK)
    {
        return result;
    }

    do
    {
        result = mFile->getByte(&c);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (c != '\n' && c != '\r' && count < length)
        {
            buffer[count++] = c;
        }

        /*
            It could be just a filename with an '=' in it,
            Check if char at -count - numwhitespaces is a newline.
        */
        if (c == '=')
        {
            result = mFile->seek(-1*count - numwhitespaces - 1, SEEK_CUR);
            if (result != FMOD_OK)
            {
                return result;
            }

            result = mFile->getByte(&c);
            if (result != FMOD_OK)
            {
                return result;
            }

            result = mFile->seek(count + numwhitespaces, SEEK_CUR);
            if (result != FMOD_OK)
            {
                return result;
            }

            if (isNewLine(c))
            {
                count++;
                break;
            }
        }

        /*
            It could be just a filename with a ']' in it,
            check if char at -count is a '[' 
        */
        if (c == ']')
        {
            /* seek to start of line */
            result = mFile->seek(-1*count, SEEK_CUR);
            if (result != FMOD_OK)
            {
                return result;
            }

            /* get the first character (after whitespace) on this line */
            result = mFile->getByte(&c);
            if (result != FMOD_OK)
            {
                return result;
            }

            /* seek back to where we were */
            result = mFile->seek(count + 1, SEEK_CUR);
            if (result != FMOD_OK)
            {
                return result;
            }

            /* check if first character of line is a '[' */
            if (c == '[')
            {
                count++;
                break;
            }
        }
    }
    while (!isNewLine(c));

    if (tokensize)
    {
        *tokensize = count;
    }

    buffer[count] = '\0';

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
// VERIFY: c951b1f8 auto
// OFFSET: 0x008F7910
FMOD_RESULT CodecPlaylist::readSimple()
{
    FMOD_RESULT result;
    char        line[MAX_TOKEN_SIZE];
    int         linelength = 0;

    result = mFile->seek(0, SEEK_SET);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (;;)
    {
        result = skipSimpleComments();
        if (result != FMOD_OK)
        {
            break;
        }

        result = readLine(line, MAX_TOKEN_SIZE, &linelength);
        if (result != FMOD_OK)
        {
            break;
        }

        metaData(FMOD_TAGTYPE_PLAYLIST, "FILE", line, linelength + 1, FMOD_TAGDATATYPE_STRING, false);  
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
// VERIFY: 1fe36eaa manual
// OFFSET: 0x008F7330
FMOD_RESULT CodecPlaylist::readLine(char *buffer, int length, int *linelength)
{
    FMOD_RESULT     result;
    int             count = 0;
    unsigned char   c;

    result = skipWhiteSpace(0);
    if (result != FMOD_OK)
    {
        return result;
    }

    do
    {
        result = mFile->getByte(&c);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (c != '\n' && c != '\r' && count < length)
        {
            buffer[count++] = c;
        }
    }
    while (!isNewLine(c));

    if (linelength)
    {
        *linelength = count;
    }

    buffer[count] = '\0';

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
// VERIFY: 1a4762e7 manual
// OFFSET: 0x008F73C0
FMOD_RESULT CodecPlaylist::skipSimpleComments()
{
    FMOD_RESULT     result = FMOD_OK;
    int             numwhitespaces = 0;
    unsigned char   c;
    
    for (;;)
    {
        result = skipWhiteSpace(&numwhitespaces);
        if (result != FMOD_OK)
        {
            return result;
        }

        result = mFile->getByte(&c);
        if (result != FMOD_OK)
        {
            return result;
        }

        if (c == '#' || c == '[')
        {
            /*
                Skip this comment
            */
            do
            {
                result = mFile->getByte(&c);
                if (result != FMOD_OK)
                {
                    return result;
                }
            }
            while (!isNewLine(c));
        }
        else
        {
            result = mFile->seek(-1, SEEK_CUR);
            if (result != FMOD_OK)
            {
                return result;
            }

            break;
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
FMOD_RESULT CodecPlaylist::getQuoteData(const char *tag, char *buffer, int *datasize)
{
    char c = 0;
    int count1 = 0;
    int count2 = 0;

    /*
        Find first quotation mark
    */
    do
    {
        c = tag[count1++];
    }
    while (c != '"');

    /*
        Find second quotation mark and copy data
    */
    for (;;)
    {
        c = tag[count1++];
        if (c == '"')
        {
            break;
        }
        buffer[count2++] = c;
    }

    buffer[count2] = '\0';
    *datasize = count2;

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
// VERIFY: b7fb4a13 auto
// OFFSET: 0x008F6D50
FMOD_RESULT CodecPlaylist::skipWhiteSpace(int *numwhitespaces)
{
    FMOD_RESULT   result;
    unsigned char c;
    int           whitespacecount = 0;
    
    do
    {
        result = mFile->getByte(&c);
        if (result != FMOD_OK)
        {
            return result;
        }

        whitespacecount++;
    }
    while (FMOD_isspace(c) || c == '\r');

    result = mFile->seek(-1, SEEK_CUR);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (numwhitespaces)
    {
        *numwhitespaces = whitespacecount - 1;
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]
    Checks if we are up to a newline or not.

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]
    Some text editors put a '\n' as a newline,
    some use '\r' and some use '\r\n' :@ !!!

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
bool CodecPlaylist::isNewLine(char c)
{
    switch (c)
    {
        case '\n':
            return true;

        case '\r':
        {
            unsigned char next;

            mFile->getByte(&next);
            mFile->seek(-1, SEEK_CUR);
            if (next == '\n')
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    
        default:
            return false;
    }

    return false;
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
FMOD_RESULT CodecPlaylist::closeInternal()
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
// VERIFY: 3f03016e manual
// OFFSET: 0x008F7B80
FMOD_RESULT F_CALLBACK CodecPlaylist::openCallback(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    CodecPlaylist *playlist = (CodecPlaylist *)codec;

    return playlist->openInternal(usermode, userexinfo);
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
FMOD_RESULT F_CALLBACK CodecPlaylist::closeCallback(FMOD_CODEC_STATE *codec)
{
    CodecPlaylist *playlist = (CodecPlaylist *)codec;

    return playlist->closeInternal();
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
FMOD_RESULT F_CALLBACK CodecPlaylist::readCallback(FMOD_CODEC_STATE *codec, void *buffer, unsigned int sizebytes, unsigned int *bytesread)
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
FMOD_RESULT F_CALLBACK CodecPlaylist::setPositionCallback(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
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
FMOD_RESULT F_CALLBACK CodecPlaylist::soundcreateCallback(FMOD_CODEC_STATE *codec, int subsound, FMOD_SOUND *sound)
{
    return FMOD_OK;
}

}

#endif


