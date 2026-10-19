#ifndef _FMOD_OS_NET
#define _FMOD_OS_NET

#include "fmod.h"

/*
    ==============================================================================
    These functions must be filled in to enable porting to any platform.
    ==============================================================================
*/

FMOD_RESULT FMOD_OS_Net_Init();
FMOD_RESULT FMOD_OS_Net_Shutdown();
FMOD_RESULT FMOD_OS_Net_Connect(const char *host, const unsigned short port, void **handle);
FMOD_RESULT FMOD_OS_Net_Listen(const unsigned short port, void **listenhandle);
FMOD_RESULT FMOD_OS_Net_Accept(const void *listenhandle, void **clienthandle);
FMOD_RESULT FMOD_OS_Net_Close(const void *handle);
FMOD_RESULT FMOD_OS_Net_Write(const void *handle, const char *buf, const unsigned int len, unsigned int *byteswritten);
FMOD_RESULT FMOD_OS_Net_Read(const void *handle, char *buf, const unsigned int len, unsigned int *bytesread);
FMOD_RESULT FMOD_OS_Net_ReadLine(const void *handle, char *buf, const unsigned int len);


#endif

