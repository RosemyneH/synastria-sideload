#include "fmod_settings.h"

#include "fmod.h"
#include "fmod_net.h"
#include "fmod_string.h"
#include "fmod_memory.h"

#include <stdlib.h>


enum
{
    FMOD_NET_VERSION_HTTP_1_0 = 0,
    FMOD_NET_VERSION_HTTP_1_1,
    FMOD_NET_VERSION_ICY,
    CT_FMOD_NET_VERSION
};

static const char *FMOD_Net_VersionString[CT_FMOD_NET_VERSION] = 
{
    "HTTP/1.0",
    "HTTP/1.1",
    "ICY",
};

static const char base64_code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *FMOD_Net_ProxyString   = 0;
char *FMOD_Net_ProxyHostname = 0;
int   FMOD_Net_ProxyPort     = 0;
char *FMOD_Net_ProxyAuth     = 0;        // Base64 encoded, user:pass pair
int   FMOD_Net_Timeout       = 5000;



/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
// VERIFY: e224be9d auto
// OFFSET: 0x0091A710
FMOD_RESULT FMOD_Net_ParseHTTPStatus(char *buf, int buflen, int *version, int *statuscode)
{
    int i, j, k;

    for (i=0;(i < buflen) && (buf[i] != 0) && (buf[i] != ' ');i++);

    if (i < buflen)
    {
        buf[i] = 0;

        for (j=0;j < CT_FMOD_NET_VERSION;j++)
        {
            if (!FMOD_strcmp(buf, FMOD_Net_VersionString[j]))
            {
                *version = j;
                break;
            }
        }

        if (j < CT_FMOD_NET_VERSION)
        {
            i++;
            k = i;
            for (;(i < buflen) && (buf[i] != 0) && (buf[i] != ' ');i++);

            if (i < buflen)
            {
                buf[i] = 0;
                *statuscode = atoi(&buf[k]);
                return FMOD_OK;
            }
        }
    }

    return FMOD_ERR_INVALID_PARAM;
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
#define CHECK_LENGTH if (out_cnt == outbuflen) { return FMOD_ERR_INVALID_PARAM; }

// VERIFY: 745e864b auto
// OFFSET: 0x0091A7D0
FMOD_RESULT FMOD_Net_EncodeBase64(char *inbuf, char *outbuf, int outbuflen)
{
    int bits = 0, char_count = 0, out_cnt = 0, c;

    if (!inbuf || !outbuf)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    while ((c = (unsigned char)*inbuf++))
    {
        bits += c;
        char_count++;
        if (char_count == 3)
        {
            CHECK_LENGTH
            outbuf[out_cnt++] = base64_code[bits >> 18];
            CHECK_LENGTH
            outbuf[out_cnt++] = base64_code[(bits >> 12) & 0x3f];
            CHECK_LENGTH
            outbuf[out_cnt++] = base64_code[(bits >> 6) & 0x3f];
            CHECK_LENGTH
            outbuf[out_cnt++] = base64_code[bits & 0x3f];
            bits = 0;
            char_count = 0;
        }
        else
        {
            bits <<= 8;
        }
    }

    if (char_count != 0)
    {
        bits <<= 16 - (8 * char_count);
        CHECK_LENGTH
        outbuf[out_cnt++] = base64_code[bits >> 18];
        CHECK_LENGTH
        outbuf[out_cnt++] = base64_code[(bits >> 12) & 0x3f];
        if (char_count == 1)
        {
            CHECK_LENGTH
            outbuf[out_cnt++] = '=';
            CHECK_LENGTH
            outbuf[out_cnt++] = '=';
        }
        else
        {
            CHECK_LENGTH
            outbuf[out_cnt++] = base64_code[(bits >> 6) & 0x3f];
            CHECK_LENGTH
            outbuf[out_cnt++] = '=';
        }
    }

    CHECK_LENGTH
    outbuf[out_cnt] = 0;

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
FMOD_RESULT FMOD_Net_SetProxy(const char *proxy)
{
    char *hostname, *username, *tmp, *p;
    FMOD_RESULT result;
    int port;

    if (FMOD_Net_ProxyString)
    {
        FMOD_Memory_Free(FMOD_Net_ProxyString);
        FMOD_Net_ProxyString = 0;
    }

    if (FMOD_Net_ProxyHostname)
    {
        FMOD_Memory_Free(FMOD_Net_ProxyHostname);
        FMOD_Net_ProxyHostname = 0;
    }

    if (FMOD_Net_ProxyAuth)
    {
        FMOD_Memory_Free(FMOD_Net_ProxyAuth);
        FMOD_Net_ProxyAuth = 0;
    }

    FMOD_Net_ProxyPort = 0;

    if (FMOD_strlen(proxy))
    {
        p = FMOD_strdup(proxy);
        if (!p)
        {
            return FMOD_ERR_MEMORY;
        }
    }
    else
    {
        /*
            Setting proxy server to null here
        */
        return FMOD_OK;
    }

    FMOD_Net_ProxyString = FMOD_strdup(proxy);
    if (!FMOD_Net_ProxyString)
    {
        return FMOD_ERR_MEMORY;
    }

    hostname = FMOD_strstr(p, "@");
    if (hostname)
    {
        *hostname = 0;
        hostname++;
        username = p;
    }
    else
    {
        hostname = p;
        username = 0;
    }

    if (username)
    {
        char buf[4096];

        result = FMOD_Net_EncodeBase64(username, buf, 4095);
        if (result != FMOD_OK)
        {
            FMOD_Memory_Free(p);
            return result;
        }

        FMOD_Net_ProxyAuth = FMOD_strdup(buf);
        if (!FMOD_Net_ProxyAuth)
        {
            return FMOD_ERR_MEMORY;
        }
    }

    tmp = FMOD_strstr(hostname, ":");
    if (tmp)
    {
        *tmp = 0;
        tmp++;
        port = atoi(tmp);
    }
    else
    {
        port = 80;
    }

    FMOD_Net_ProxyHostname = FMOD_strdup(hostname);
    if (!FMOD_Net_ProxyHostname)
    {
        return FMOD_ERR_MEMORY;
    }

    FMOD_Net_ProxyPort     = port;

    FMOD_Memory_Free(p);

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
FMOD_RESULT FMOD_Net_GetProxy(char *proxy, int proxylen)
{
    if (proxy && proxylen)
    {
        if (FMOD_Net_ProxyString)
        {
            FMOD_strncpy(proxy, FMOD_Net_ProxyString, proxylen);
            proxy[proxylen - 1] = 0;
        }
        else
        {
            proxy[0] = 0;
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
FMOD_RESULT FMOD_Net_SetTimeout(int timeout)
{
    FMOD_Net_Timeout = timeout;

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
FMOD_RESULT FMOD_Net_GetTimeout(int *timeout)
{
    if (!timeout)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *timeout = FMOD_Net_Timeout;

    return FMOD_OK;
}
