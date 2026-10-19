#include "fmod_settings.h"

#include "fmod_debug.h"
#include "fmod_file_net.h"
#include "fmod_metadata.h"
#include "fmod_net.h"
#include "fmod_os_net.h"
#include "fmod_string.h"
#include "fmod_stringw.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace FMOD
{

static const unsigned int FMOD_FILE_NET_SEEK_BLOCKSIZE = 16384;
static const unsigned int SIZEOF_METABUF = (255 * 16) + 1;



/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: bd50eb5b manual
// OFFSET: 0x00920FA0
NetFile::NetFile()
{
    mHandle           = (void *)-1;
    mProtocol         = -1;
    mAbsolutePos      = 0;
    mHttpStatus       = 0;
    mMetaint          = 0;
    mBytesBeforeMeta  = 0;
    mMetabuf          = 0;
    mMetaFormat       = FMOD_TAGTYPE_UNKNOWN;
    mSeekable        = false;
    mChunked          = false;
    mBytesLeftInChunk = 0;
    mDeviceType       = DEVICE_NET;
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
FMOD_RESULT NetFile::init()
{
    return FMOD_OS_Net_Init();
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
// VERIFY: 9aa4dc65 auto
// OFFSET: 0x0091FA30
FMOD_RESULT NetFile::shutDown()
{
    return FMOD_OS_Net_Shutdown();
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
// VERIFY: 8a1edad0 manual
// OFFSET: 0x0091FA40
FMOD_RESULT NetFile::parseUrl(char *url, char *host, int hostlen, char *auth, int authlen, unsigned short *port, char *file, const int filelen, bool *mms)
{
    char *p;
    bool isipaddr    = true;
    bool hasuserpass = false;
    char userpass[4096];

    if (mms)
    {
        *mms = false;
    }

    if (!FMOD_strnicmp("http://", url, 7) || !FMOD_strnicmp("http:\\\\", url, 7))
    {
        url += 7;
    }
    else if (!FMOD_strnicmp("https://", url, 8) || !FMOD_strnicmp("https:\\\\", url, 8))
    {
        url += 8;
    }
    else if (!FMOD_strnicmp("mms://", url, 6) || !FMOD_strnicmp("mms:\\\\", url, 6))
    {
        if (mms)
        {
            *mms = true;
        }
        url += 6;
    }
    else
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    p = (char *)url;

    /*
        Could be username:password@url:port or just url:port
    */

    /*
        Get username:password
    */
    while (*p && *p != '/')
    {
        if (*p == '@')
        {
            hasuserpass = true;
            break;
        }

        p++;
    }

    if (hasuserpass)
    {
        FMOD_strcpy(userpass, url);
        userpass[p - url] = 0;
        p++;
        url = p;
    }
    else
    {
        p = (char *)url;
    }

    /*
        Get host
    */
    while (*p && *p != ':' && *p != '/')
    {
        if ((*p < '0' || *p > '9') && *p != '.')
        {
            isipaddr = false;
        }
        p++;
    }

    if (FMOD_strlen(url) >= hostlen)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_strncpy(host, url, hostlen);
    host[p - url] = 0;

    if (!*p || *p == '/')
    {
        *port = 80;
    }
    else
    {
        int i;
        char tmp[FMOD_STRING_MAXPATHLEN];
        p++;
        for (i=0;(i < FMOD_STRING_MAXPATHLEN) && *p && (*p >= '0') && (*p <= '9');i++, p++)
        {
            tmp[i] = *p;
        }
        tmp[i] = 0;
        *port = atoi(tmp);
    }

    if (*p && !FMOD_isspace(*p))
    {
        if (FMOD_strlen(p) >= filelen)
        {
            return FMOD_ERR_INVALID_PARAM;
        }
        else
        {
            char *t = p + FMOD_strlen(p) - 1;
            for (;(t > p) && FMOD_isspace(*t);t--) ;
            FMOD_strncpy(file, p, (int)(t - p + 1));
            file[t - p + 1] = 0;
        }
    }
    else
    {
        FMOD_strcpy(file, "/");
    }

    if (hasuserpass)
    {
        if (auth)
        {
            FMOD_RESULT result = FMOD_Net_EncodeBase64(userpass, auth, authlen);
            if (result != FMOD_OK)
            {
                return result;
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

	[SEE_ALSO]
]
*/
// VERIFY: 313e66fb manual
// OFFSET: 0x0091FCA0
FMOD_RESULT NetFile::openAsHTTP(const char *name_or_data, char *host, char *file, char *auth, unsigned short port, unsigned int *filesize)
{
    FMOD_RESULT result;
	char buf[8192], http_request[8192];
    int version, statuscode, redirect_count = 0;

CONNECT:

    /*
        Connect to the remote host
    */
    if (FMOD_Net_ProxyHostname)
    {
        result = FMOD_OS_Net_Connect(FMOD_Net_ProxyHostname, FMOD_Net_ProxyPort, &mHandle);
    }
    else
    {
        result = FMOD_OS_Net_Connect(host, port, &mHandle);
    }
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        Construct a http request
    */
    http_request[0] = 0;
    sprintf(buf, "GET %s HTTP/1.1\r\n", FMOD_Net_ProxyHostname ? name_or_data : file);
    FMOD_strcat(http_request, buf);
    sprintf(buf, "Host: %s\r\n", host);
    FMOD_strcat(http_request, buf);
    sprintf(buf, "User-Agent: FMOD/%d.%d\r\n", (FMOD_VERSION >> 8) & 0xff, FMOD_VERSION & 0xff);
    FMOD_strcat(http_request, buf);
    sprintf(buf, "Icy-MetaData:1\r\n");
    FMOD_strcat(http_request, buf);
    if (FMOD_Net_ProxyAuth)
    {
        sprintf(buf, "Proxy-Authorization: Basic %s\r\n", FMOD_Net_ProxyAuth);
        FMOD_strcat(http_request, buf);
    }
    if (auth[0])
    {
        sprintf(buf, "Authorization: Basic %s\r\n", auth);
        FMOD_strcat(http_request, buf);
    }
    sprintf(buf, "Connection: close\r\n");
    FMOD_strcat(http_request, buf);
    FMOD_strcat(http_request, "\r\n");

    /*
        Send the http request
    */
    unsigned int byteswritten = 0;
    result = FMOD_OS_Net_Write(mHandle, http_request, FMOD_strlen(http_request), &byteswritten);
    if (result != FMOD_OK)
    {
        return result;
    }

    /*
        Read and parse the remote host's response
    */
    result = FMOD_OS_Net_ReadLine(mHandle, buf, 8191);
    if (result == FMOD_OK)
    {
        result = FMOD_Net_ParseHTTPStatus(buf, FMOD_strlen(buf), &version, &statuscode);
        if (result == FMOD_OK)
        {
            mHttpStatus = statuscode;

            if (statuscode == 200)
            {
                /*
                    Request was successful
                */
                do
                {
                    result = FMOD_OS_Net_ReadLine(mHandle, buf, 8191);
                    if (result == FMOD_OK)
                    {
                        char *tmp;

                        /*
                            HTTP headers
                        */
                        if (!FMOD_strncmp("Content-Length:", buf, 15))
                        {
                            *filesize = atoi(buf + 15);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_HTTP;
                            }
                        }
                        else if (!FMOD_strncmp("Transfer-Encoding: chunked", buf, 26))
                        {
                            mChunked = true;
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_HTTP;
                            }
                        }
                        /*
                            SHOUTcast headers
                        */
                        else if (!FMOD_strncmp("icy-metaint:", buf, 12))
                        {
                            mMetaint = atoi(buf + 12);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_SHOUTCAST;
                            }
                        }
                        else if (!FMOD_strncmp("icy-br:", buf, 7))
                        {
                            tmp = FMOD_eatwhite(buf + 7);
                            mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "icy-br", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_SHOUTCAST;
                            }
                        }
                        else if (!FMOD_strncmp("icy-pub:", buf, 8))
                        {
                            tmp = FMOD_eatwhite(buf + 8);
                            mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "icy-pub", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_SHOUTCAST;
                            }
                        }
                        else if (!FMOD_strncmp("icy-notice1:", buf, 12))
                        {
                            tmp = FMOD_eatwhite(buf + 12);
                            mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "icy-notice1", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_SHOUTCAST;
                            }
                        }
                        else if (!FMOD_strncmp("icy-notice2:", buf, 12))
                        {
                            tmp = FMOD_eatwhite(buf + 12);
                            mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "icy-notice2", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_SHOUTCAST;
                            }
                        }
                        else if (!FMOD_strncmp("icy-name:", buf, 9))
                        {
                            tmp = FMOD_eatwhite(buf + 9);
                            mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "icy-name", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_SHOUTCAST;
                            }
                        }
                        else if (!FMOD_strncmp("icy-genre:", buf, 10))
                        {
                            tmp = FMOD_eatwhite(buf + 10);
                            mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "icy-genre", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_SHOUTCAST;
                            }
                        }
                        else if (!FMOD_strncmp("icy-url:", buf, 8))
                        {
                            tmp = FMOD_eatwhite(buf + 8);
                            mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "icy-url", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_SHOUTCAST;
                            }
                        }
                        else if (!FMOD_strncmp("icy-irc:", buf, 8))
                        {
                            tmp = FMOD_eatwhite(buf + 8);
                            mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "icy-irc", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_SHOUTCAST;
                            }
                        }
                        else if (!FMOD_strncmp("icy-icq:", buf, 8))
                        {
                            tmp = FMOD_eatwhite(buf + 8);
                            mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "icy-icq", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_SHOUTCAST;
                            }
                        }
                        else if (!FMOD_strncmp("icy-aim:", buf, 8))
                        {
                            tmp = FMOD_eatwhite(buf + 8);
                            mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "icy-aim", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_SHOUTCAST;
                            }
                        }
                        /*
                            Icecast headers
                        */
                        if (!FMOD_strncmp("ice-bitrate: ", buf, 13))
                        {
                            tmp = FMOD_eatwhite(buf + 13);
                            mMetadata.addTag(FMOD_TAGTYPE_ICECAST, "ice-bitrate", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_ICECAST;
                            }
                        }
                        else if (!FMOD_strncmp("ice-description: ", buf, 17))
                        {
                            tmp = FMOD_eatwhite(buf + 17);
                            mMetadata.addTag(FMOD_TAGTYPE_ICECAST, "ice-description", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_ICECAST;
                            }
                        }
                        else if (!FMOD_strncmp("ice-public: ", buf, 12))
                        {
                            tmp = FMOD_eatwhite(buf + 12);
                            mMetadata.addTag(FMOD_TAGTYPE_ICECAST, "ice-public", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_ICECAST;
                            }
                        }
                        else if (!FMOD_strncmp("ice-name: ", buf, 10))
                        {
                            tmp = FMOD_eatwhite(buf + 10);
                            mMetadata.addTag(FMOD_TAGTYPE_ICECAST, "ice-name", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_ICECAST;
                            }
                        }
                        else if (!FMOD_strncmp("ice-genre: ", buf, 11))
                        {
                            tmp = FMOD_eatwhite(buf + 11);
                            mMetadata.addTag(FMOD_TAGTYPE_ICECAST, "ice-genre", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_ICECAST;
                            }
                        }
                        else if (!FMOD_strncmp("ice-url: ", buf, 9))
                        {
                            tmp = FMOD_eatwhite(buf + 9);
                            mMetadata.addTag(FMOD_TAGTYPE_ICECAST, "ice-url", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_ICECAST;
                            }
                        }
                        else if (!FMOD_strncmp("ice-irc: ", buf, 9))
                        {
                            tmp = FMOD_eatwhite(buf + 9);
                            mMetadata.addTag(FMOD_TAGTYPE_ICECAST, "ice-irc", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_ICECAST;
                            }
                        }
                        else if (!FMOD_strncmp("ice-icq: ", buf, 9))
                        {
                            tmp = FMOD_eatwhite(buf + 9);
                            mMetadata.addTag(FMOD_TAGTYPE_ICECAST, "ice-icq", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_ICECAST;
                            }
                        }
                        else if (!FMOD_strncmp("ice-aim: ", buf, 9))
                        {
                            tmp = FMOD_eatwhite(buf + 9);
                            mMetadata.addTag(FMOD_TAGTYPE_ICECAST, "ice-aim", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_ICECAST;
                            }
                        }
                        else if (!FMOD_strncmp("ice-audio-info: ", buf, 16))
                        {
                            tmp = FMOD_eatwhite(buf + 16);
                            mMetadata.addTag(FMOD_TAGTYPE_ICECAST, "ice-audio-info", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_ICECAST;
                            }
                        }
                        else if (!FMOD_strncmp("ice-private: ", buf, 13))
                        {
                            tmp = FMOD_eatwhite(buf + 13);
                            mMetadata.addTag(FMOD_TAGTYPE_ICECAST, "ice-private", tmp, FMOD_strlen(tmp) + 1, FMOD_TAGDATATYPE_STRING, false);
                            if (mProtocol == -1)
                            {
                                mProtocol = FMOD_PROTOCOL_ICECAST;
                            }
                        }
                    }
                    else
                    {
                        break;
                    }

                } while (FMOD_strlen(buf));
            }
            else if ((statuscode == 301) || (statuscode == 302) || (statuscode == 303) || 
                     (statuscode == 305) || (statuscode == 307))
            {
                /*
                    HTTP redirect
                */
                if (redirect_count < 16)
                {
                    do
                    {
                        result = FMOD_OS_Net_ReadLine(mHandle, buf, 8191);
                        if (result == FMOD_OK)
                        {
                            if (!FMOD_strncmp("Location: ", buf, 10))
                            {
                                /*
                                    Got the new location - parse it and then go back and connect again
                                */
                                char new_host[FMOD_STRING_MAXPATHLEN], new_file[FMOD_STRING_MAXPATHLEN];
                                unsigned short new_port;

                                new_host[0] = new_file[0] = 0;
                                new_port = 0;

                                if (FMOD_strnicmp(buf + 10, "http://", 7) &&
                                    FMOD_strnicmp(buf + 10, "http:\\\\", 7))
                                {
                                    FMOD_strncpy(buf + 3, "http://", 7);
                                    parseUrl(buf + 3, new_host, FMOD_STRING_MAXNAMELEN-1, 0, 0, &new_port, new_file, FMOD_STRING_MAXNAMELEN-1, 0);
                                }
                                else
                                {
                                    parseUrl(buf + 10, new_host, FMOD_STRING_MAXNAMELEN-1, 0,0, &new_port, new_file, FMOD_STRING_MAXNAMELEN-1, 0);
                                }

                                if (FMOD_strlen(new_host))
                                {
                                    FMOD_strcpy(host, new_host);
                                }

                                if (FMOD_strlen(new_file))
                                {
                                    FMOD_strcpy(file, new_file);
                                }

                                if (new_port)
                                {
                                    port = new_port;
                                }

                                redirect_count++;
                                FMOD_OS_Net_Close(mHandle);
                                mHandle = (void *)-1;

                                goto CONNECT;
                            }
                        }
                        else
                        {
                            break;
                        }

                    } while (FMOD_strlen(buf));
                }

                return FMOD_ERR_HTTP;
            }
            else
            {
                /*
                    HTTP Error
                */
                switch (mHttpStatus)
                {
                    case 401 :
                    case 403 :
                        return FMOD_ERR_HTTP_ACCESS;

                    case 404 : 
                        return FMOD_ERR_FILE_NOTFOUND;

                    case 407 : 
                        return FMOD_ERR_HTTP_PROXY_AUTH;

                    case 408 : 
                        return FMOD_ERR_HTTP_TIMEOUT;

                    case 500 :
                    case 501 :
                    case 502 :
                    case 503 :
                    case 504 :
                    case 505 :
                        return FMOD_ERR_HTTP_SERVER_ERROR;

                    default :
                        return FMOD_ERR_HTTP;
                }
            }
        }
        else
        {
            return result;
        }
    }
    else
    {
        return result;
    }

    if ((mProtocol == FMOD_PROTOCOL_ICECAST) || (mProtocol == FMOD_PROTOCOL_HTTP))
    {
        /*
            These protocols can support any number of formats - try to detect ones we want to treat specially
        */
        if (FMOD_strlen(name_or_data) > 4)
        {
            name_or_data = name_or_data + FMOD_strlen(name_or_data) - 4;

            if (!FMOD_stricmp(name_or_data, ".ogg"))
            {
                mMetaFormat = FMOD_TAGTYPE_VORBISCOMMENT;
            }
            else if (!FMOD_stricmp(name_or_data, ".mp3"))
            {
                mMetaFormat = FMOD_TAGTYPE_SHOUTCAST;
            }

            // Add other format checks here...
        }
    }
    else
    {
        /*
            SHOUTcast only supports mp3
        */
        mMetaFormat = FMOD_TAGTYPE_SHOUTCAST;
    }

    if (mMetaint)
    {
        mBytesBeforeMeta = mMetaint;
        mMetabuf         = (char *)FMOD_Memory_Alloc(SIZEOF_METABUF);
        if (!mMetabuf)
        {
            reallyClose();
            return FMOD_ERR_MEMORY;
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

	[SEE_ALSO]
]
*/
FMOD_RESULT NetFile::openAsMMS(const char *name_or_data, char *host, char *file, char *auth, unsigned short port, unsigned int *filesize)
{
    return FMOD_ERR_UNSUPPORTED;
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

// VERIFY: f7ce5149 auto
// OFFSET: 0x00920920
FMOD_RESULT NetFile::reallyOpen(const char *name_or_data, unsigned int *filesize)
{
    FMOD_RESULT result;
    char host[FMOD_STRING_MAXPATHLEN], file[FMOD_STRING_MAXPATHLEN];
    char auth[4096];
    unsigned short port;
    bool mms = false;

    init();

    *filesize = (unsigned int)-1;

    FMOD_memset(auth, 0, 4096);

    result = parseUrl((char *)name_or_data, host, FMOD_STRING_MAXNAMELEN-1, auth, 4096, &port, file, FMOD_STRING_MAXNAMELEN-1, &mms);
    if (result != FMOD_OK)
    {
        return result;
    }

    if (mms)
    {
        return openAsMMS(name_or_data, host, file, auth, port, filesize);
    }
    else
    {
        return openAsHTTP(name_or_data, host, file, auth, port, filesize);
    }
    
	return FMOD_ERR_INTERNAL;
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
// VERIFY: 9e719a12 auto
// OFFSET: 0x009209E0
FMOD_RESULT NetFile::reallyClose()
{
    FMOD_RESULT result;

    if (mHandle != (void *)-1)
    {
        result = FMOD_OS_Net_Close(mHandle);
        mHandle = (void *)-1;
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    if (mMetabuf)
    {
        FMOD_Memory_Free(mMetabuf);
        mMetabuf = 0;
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
// VERIFY: d81d4c29 manual
// OFFSET: 0x00920A40
FMOD_RESULT NetFile::reallyRead(void *buffer, unsigned int size, unsigned int *rd)
{
    FMOD_RESULT result;

AGAIN:

    if (mChunked)
    {
        if (!mBytesLeftInChunk)
        {
            const int MAX_CHUNKSIZESTR_LEN = 256;
            char chunksizestr[MAX_CHUNKSIZESTR_LEN];

            FMOD_memset(chunksizestr, 0, MAX_CHUNKSIZESTR_LEN);
            chunksizestr[0] = '0';
            chunksizestr[1] = 'x';

            result = FMOD_OS_Net_ReadLine(mHandle, chunksizestr+2, MAX_CHUNKSIZESTR_LEN);
            if (result != FMOD_OK)
            {
                return result;
            }

            if (sscanf(chunksizestr, "%x", &mBytesLeftInChunk) < 0)
            {
                return FMOD_ERR_FILE_EOF;
            }
        }

        if (!size)
        {
            return FMOD_ERR_FILE_EOF;
        }

        if (size > mBytesLeftInChunk)
        {
            size = mBytesLeftInChunk;
        } 
    }

    if (mMetaint)
    {
        if (!mBytesBeforeMeta)
        {
            if (mMetaFormat == FMOD_TAGTYPE_SHOUTCAST)
            {
                char tmp, *p, *name, *value, *artist, *title, *strend;
                unsigned int metaread, metabytestoread;

                /*
                    Read metadata length
                */
        	    result = FMOD_OS_Net_Read(mHandle, (char *)&tmp, 1, &metaread);
                if (result != FMOD_OK)
                {
                    return result;
                }

                if (metaread != 1)
                {
                    return FMOD_ERR_NET_SOCKET_ERROR;
                }

                /*
                    Read metadata
                */
                metabytestoread = (int)tmp * 16;
                p = mMetabuf;
                FMOD_memset(mMetabuf, 0, SIZEOF_METABUF);
                while (metabytestoread)
                {
        	        result = FMOD_OS_Net_Read(mHandle, p, metabytestoread, &metaread);
                    if (result != FMOD_OK)
                    {
                        return result;
                    }

                    metabytestoread -= metaread;
                    p += metaread;
                }

                /*
                    Parse metadata
                */
                p = FMOD_strstr(mMetabuf, "StreamTitle='");
                if (p)
                {
                    name = p;
                    name[11] = 0;

                    value = &name[13];
                    strend = value;
                    for (;*strend && *strend != ';';strend++);
                    strend--;       // skip back to last "'"
                    *strend = 0;

                    artist = value;
                    title = FMOD_strstr(artist, " - ");
                    if (title)
                    {
                        *title = 0;
                        title += 3;
                    }

                    mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "ARTIST", artist, FMOD_strlen(artist) + 1, FMOD_TAGDATATYPE_STRING, true);
                    if (title)
                    {
                        mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, "TITLE",  title,  FMOD_strlen(title) + 1,  FMOD_TAGDATATYPE_STRING, true);
                    }

                    name[11] = ' ';
                    *strend = ' ';
                    if (title)
                    {
                        title -= 3;
                        *title = ' ';
                    }
                }

                p = FMOD_strstr(mMetabuf, "StreamUrl='");
                if (p)
                {
                    name = p;
                    name[9] = 0;

                    value = &name[11];
                    strend = value;
                    for (;*strend && *strend != ';';strend++);
                    strend--;       // skip back to last "'"
                    *strend = 0;

                    mMetadata.addTag(FMOD_TAGTYPE_SHOUTCAST, name, value, FMOD_strlen(value) + 1, FMOD_TAGDATATYPE_STRING, true);
                }

                mBytesBeforeMeta = mMetaint;

                goto AGAIN;
            }
            else if (mMetaFormat == FMOD_TAGTYPE_VORBISCOMMENT)
            {
                mBytesBeforeMeta = mMetaint;

                goto AGAIN;
            }
            else if (mMetaFormat == FMOD_TAGTYPE_UNKNOWN)
            {
                mBytesBeforeMeta = mMetaint;

                goto AGAIN;
            }
            else
            {
                /*
                    Unknown metadata format
                */
                return FMOD_ERR_INVALID_PARAM;
            }
        }
        else
        {
            if (mBytesBeforeMeta < size)
            {
                size = mBytesBeforeMeta;
            }
        }
    }

    if (mAbsolutePos == mLength)
    {
        return FMOD_ERR_FILE_EOF;
    }

    if ((mAbsolutePos + size) > mLength)
    {
        size = mLength - mAbsolutePos;
    }

    if (size > 4096)
    {
        size = 4096;
    }

	result = FMOD_OS_Net_Read(mHandle, (char *)buffer, size, rd);
    if (result != FMOD_OK)
    {
        return result;
    }

    mAbsolutePos += *rd;

    if (mMetaint)
    {
        mBytesBeforeMeta -= *rd;
    }
    if (mChunked)
    {
        mBytesLeftInChunk -= *rd;
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
// VERIFY: 51420ad8 auto
// OFFSET: 0x00920D90
FMOD_RESULT NetFile::reallySeek(unsigned int pos)
{
    FMOD_RESULT result = FMOD_OK;

    if (pos >= mAbsolutePos)
    {
        unsigned int read, bytestoread = pos - mAbsolutePos;

        if (bytestoread)
        {
            char *tmpbuf;
            unsigned int blocksize = bytestoread > FMOD_FILE_NET_SEEK_BLOCKSIZE ? FMOD_FILE_NET_SEEK_BLOCKSIZE : bytestoread;

            tmpbuf = (char *)FMOD_Memory_Alloc(blocksize);
            if (!tmpbuf)
            {
                return FMOD_ERR_MEMORY;
            }

            while (bytestoread)
            {
                unsigned int readlen = bytestoread > blocksize ? blocksize : bytestoread;

                if (mMetaint)
                {
                    if (!mBytesBeforeMeta)
                    {
                        if (mMetaFormat == FMOD_TAGTYPE_SHOUTCAST)
                        {
                            char tmp, *p;
                            unsigned int metaread, metabytestoread;

        	                result = FMOD_OS_Net_Read(mHandle, (char *)&tmp, 1, &metaread);
                            if (result != FMOD_OK)
                            {
                                break;
                            }

                            if (metaread != 1)
                            {
                                result = FMOD_ERR_NET_SOCKET_ERROR;
                                break;
                            }

                            metabytestoread = (int)tmp * 16;
                            p = mMetabuf;
                            FMOD_memset(mMetabuf, 0, SIZEOF_METABUF);
                            while (metabytestoread)
                            {
        	                    result = FMOD_OS_Net_Read(mHandle, p, metabytestoread, &metaread);
                                if (result != FMOD_OK)
                                {
                                    break;
                                }

                                if (metaread == 0)
                                {
                                    result = FMOD_ERR_FILE_COULDNOTSEEK;
                                    break;
                                }

                                metabytestoread -= metaread;
                                p += metaread;
                            }

                            if (result != FMOD_OK)
                            {
                                break;
                            }
                        }

                        mBytesBeforeMeta = mMetaint;
                    }

                    if (readlen > mBytesBeforeMeta)
                    {
                        readlen = mBytesBeforeMeta;
                    }
                }

                result = FMOD_OS_Net_Read(mHandle, tmpbuf, readlen, &read);
                if (result != FMOD_OK)
                {
                    result = FMOD_ERR_FILE_COULDNOTSEEK;
                    break;
                }

                if (read == 0)
                {
                    result =  FMOD_ERR_FILE_COULDNOTSEEK;
                    break;
                }

                bytestoread  -= read;
                mAbsolutePos += read;
                if (mMetaint)
                {
                    mBytesBeforeMeta -= read;
                }
            }

            FMOD_Memory_Free(tmpbuf);
        }
    }
    else
    {
        result = FMOD_ERR_FILE_COULDNOTSEEK;
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
FMOD_RESULT NetFile::reallyCancel()
{
    FMOD_OS_Net_Close(mHandle);
    
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
// VERIFY: 091bbe4e auto
// OFFSET: 0x00920F70
FMOD_RESULT NetFile::getMetadata(Metadata **metadata)
{
    if (!metadata)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *metadata = &mMetadata;

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


}



