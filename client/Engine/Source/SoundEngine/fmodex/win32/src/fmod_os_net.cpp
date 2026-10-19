#include "fmod_net.h"
#include "fmod_os_misc.h"
#include "fmod_time.h"

#include <windows.h>


static const int                FMOD_NET_MAXADDRLEN      = 46;

static bool                     FMOD_OS_Net_Initialised  = false;

FMOD_OS_CRITICALSECTION         *gResolveCrit            = 0;


/*
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
*/
// VERIFY: 7ea5a02e auto
// OFFSET: 0x009266E0
FMOD_RESULT FMOD_OS_Net_Init()
{
    FMOD_RESULT result;

    if (!FMOD_OS_Net_Initialised)
    {
        WSADATA wsadata;

        WSAStartup(0x0002, &wsadata);

        result = FMOD_OS_CriticalSection_Create(&gResolveCrit);
        if (result != FMOD_OK)
        {
            WSACleanup();
            return result;
        }

        FMOD_OS_Net_Initialised = true;
    }

    return FMOD_OK;
}


/*
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
*/
// VERIFY: 4531a50d auto
// OFFSET: 0x00926730
FMOD_RESULT FMOD_OS_Net_Shutdown()
{
    if (FMOD_OS_Net_Initialised)
    {
        FMOD_OS_CriticalSection_Free(gResolveCrit);
        gResolveCrit = 0;

        WSACleanup();

        FMOD_OS_Net_Initialised = false;
    }

    return FMOD_OK;
}


/*
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
*/
// VERIFY: 38e0cca2 manual
// OFFSET: 0x00926760
FMOD_RESULT FMOD_OS_Net_Connect(const char *host, const unsigned short port, void **handle)
{
    struct sockaddr_in   server;
    struct hostent      *h;
    FMOD_RESULT          result;
    SOCKET               sock;
    int                  rc;
    struct               timeval tv;
    unsigned long        nonblocking = 1;
    fd_set               writefd;
    unsigned int         thistime = 0, lasttime = 0, elapsedtime = 0;
    unsigned int         timeout = FMOD_Net_Timeout;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        return FMOD_ERR_MEMORY;
    }
    else
    {
        *handle = (void *)sock;
    }

    FMOD_memset(&server, 0, sizeof(struct sockaddr_in));

    *((unsigned long *)&server.sin_addr) = inet_addr(host);

    if (*((unsigned long *)&server.sin_addr) == INADDR_NONE)
    {
        char buf[MAXGETHOSTSTRUCT];
        HANDLE handle;

        result = FMOD_OS_CriticalSection_Enter(gResolveCrit);
        if (result != FMOD_OK)
        {
            closesocket(sock);
            return result;
        }

        FMOD_memset(buf, 0, MAXGETHOSTSTRUCT);

        handle = WSAAsyncGetHostByName(NULL, NULL, host, buf, MAXGETHOSTSTRUCT);
    
        h = (struct hostent *)buf;

        FMOD_OS_Time_GetMs(&lasttime);

        for (;;)
        {
            if (h->h_name)
            {
                break;
            }

            FMOD_OS_Time_GetMs(&thistime);

            elapsedtime += (thistime - lasttime);
            lasttime = thistime;

            if (elapsedtime >= timeout)
            {
                WSACancelAsyncRequest(handle);
                FMOD_OS_CriticalSection_Leave(gResolveCrit);
                return FMOD_ERR_NET_URL;
            }

            FMOD_OS_Time_Sleep(10);
        }

        FMOD_memcpy(&server.sin_addr, (struct in_addr *)h->h_addr, sizeof(struct in_addr));
        FMOD_OS_CriticalSection_Leave(gResolveCrit);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    FD_ZERO(&writefd);
    FD_SET(sock, &writefd);

    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    ioctlsocket(sock, FIONBIO, &nonblocking);

    rc = connect(sock, (struct sockaddr *)&server, sizeof(server));
    if (rc == SOCKET_ERROR)
    {
        int rc = WSAGetLastError();

        if (rc != WSAEWOULDBLOCK)
        {
            closesocket(sock);
            return FMOD_ERR_NET_CONNECT;
        }
    }

    /*
        Select will block for timeout length, or return when connected
    */
    rc = select(0, 0, &writefd, 0, &tv);
    if (rc <= 0)
    {
        closesocket(sock);
        return FMOD_ERR_NET_CONNECT;
    }

    nonblocking = 0;
    ioctlsocket(sock, FIONBIO, &nonblocking);

    return FMOD_OK;
}


/*
	[DESCRIPTION]
    Begin listening on the designated port, non-blocking

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
*/
// VERIFY: c70e6879 auto
// OFFSET: 0x00926960
FMOD_RESULT FMOD_OS_Net_Listen(const unsigned short port, void **listenhandle)
{
	SOCKET          listensocket;
    SOCKADDR_IN     address;
    u_long          nonblocking = TRUE;
    
    // Create the socket
    listensocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listensocket == INVALID_SOCKET)
    {
        return FMOD_ERR_MEMORY;
    }
    
	// Set blocking mode
	if (ioctlsocket(listensocket, FIONBIO, &nonblocking) == SOCKET_ERROR)
    {
        return FMOD_ERR_NET_SOCKET_ERROR;
    }

	// Set to listen from any IP on the defined port
    FMOD_memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	
	// Bind the address to the socket
	if (bind(listensocket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
	{
        return FMOD_ERR_NET_SOCKET_ERROR;
	}

	// Start listening for connections
	if (listen(listensocket, 3) == SOCKET_ERROR)
	{
        return FMOD_ERR_NET_SOCKET_ERROR;
	}

    *listenhandle = (void *)listensocket;    
    return FMOD_OK;
}


/*
	[DESCRIPTION]
    Accept a pending connection from the queue

	[PARAMETERS]
    
	[RETURN_VALUE]
    
	[REMARKS]

	[SEE_ALSO]
*/
// VERIFY: 3325434e auto
// OFFSET: 0x00926A00
FMOD_RESULT FMOD_OS_Net_Accept(const void *listenhandle, void **clienthandle)
{
    SOCKET listensocket = (SOCKET)listenhandle;
    SOCKET clientsocket = NULL;

    if (listenhandle == (void *)-1)
    {
        return FMOD_ERR_NET_SOCKET_ERROR;
    }
    
    // Accept a waiting connection
    clientsocket = accept(listensocket, NULL, NULL);
    if(clientsocket == INVALID_SOCKET)
    {
        int errorcode = WSAGetLastError();
        if(errorcode == WSAEWOULDBLOCK)
        {
            return FMOD_ERR_NET_WOULD_BLOCK;
        }
        else
        {
            return FMOD_ERR_NET_SOCKET_ERROR;
        }
    }
       
    *clienthandle = (void *)clientsocket;
    return FMOD_OK;
}


/*
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
*/
// VERIFY: 89ff84e1 auto
// OFFSET: 0x00926A50
FMOD_RESULT FMOD_OS_Net_Close(const void *handle)
{
    SOCKET sock = (SOCKET)handle;

    if (sock != -1)
    {
        closesocket(sock);
    }

    return FMOD_OK;
}


/*
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
*/
// VERIFY: e9be26dd manual
// OFFSET: 0x00926A70
FMOD_RESULT FMOD_OS_Net_Write(const void *handle, const char *buf, const unsigned int len, unsigned int *byteswritten)
{
    SOCKET sock = (SOCKET)handle;
    int written, bytestowrite = len;

    if (handle == (void *)-1)
    {
        return FMOD_ERR_NET_SOCKET_ERROR;
    }

    *byteswritten = 0;

    while (bytestowrite)
    {
        written = send(sock, buf, bytestowrite, 0);
        if (written == SOCKET_ERROR)
        {
            int errorcode = WSAGetLastError();
            if(errorcode == WSAEWOULDBLOCK)
            {
                return FMOD_ERR_NET_WOULD_BLOCK;
            }
            else
            {
                return FMOD_ERR_NET_SOCKET_ERROR;
            }
        }

        *byteswritten += written;
        bytestowrite -= written;
        buf += written;
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
// VERIFY: 4374d422 manual
// OFFSET: 0x00926AE0
FMOD_RESULT FMOD_OS_Net_Read(const void *handle, char *buf, const unsigned int len, unsigned int *bytesread)
{
    SOCKET sock = (SOCKET)handle;
    int read, bytestoread = len;

    if (handle == (void *)-1)
    {
        return FMOD_ERR_NET_SOCKET_ERROR;
    }

    if (!buf || (len <= 0))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *bytesread = 0;

    while (bytestoread)
    {
        read = recv(sock, buf, bytestoread, 0);

        if (read == SOCKET_ERROR)
        {
            int errorcode = WSAGetLastError();
            if(errorcode == WSAEWOULDBLOCK)
            {
                return FMOD_ERR_NET_WOULD_BLOCK;
            }
            else
            {
                return FMOD_ERR_NET_SOCKET_ERROR;
            }
        }
        else if (read == 0)
        {
            /*
                Connection closed gracefully
            */
            return FMOD_ERR_FILE_EOF;
        }
        else
        {
            *bytesread  += read;
            bytestoread -= read;
            buf += read;
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
// VERIFY: 9bbba7de auto
// OFFSET: 0x00926B70
FMOD_RESULT FMOD_OS_Net_ReadLine(const void *handle, char *buf, const unsigned int len)
{
    unsigned int pos, read;
    char c = 0;

    buf[0] = 0;

    if (handle == (void *)-1)
    {
        return FMOD_ERR_NET_SOCKET_ERROR;
    }

    if (!buf || (len <= 0))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    pos = 0;

    while (pos < len)
    {
        FMOD_RESULT result = FMOD_OS_Net_Read(handle, &c, 1, &read);
        if (result == FMOD_ERR_NET_WOULD_BLOCK)
        {
            return FMOD_ERR_NET_WOULD_BLOCK;
        }

        if (read == 1)
        {
            if (c == '\n')
            {
                break;
            }
            else if (c != '\r')
            {
                buf[pos++] = c;
            }
        }
        else
        {
            break;
        }
    }

    if (pos >= len)
    {
        pos = len - 1;
    }

    buf[pos] = 0;

    return FMOD_OK;
}
