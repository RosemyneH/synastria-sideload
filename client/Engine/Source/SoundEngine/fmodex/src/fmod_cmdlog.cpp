#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_CMDLOG

#define FMOD_CMDLOG_PLAYBACK


#include "fmod_cmdlog.h"
#include "fmod.hpp"
#include "fmod_memory.h"
#include "fmod_time.h"
#include "fmod_systemi.h"
#include "stdio.h"


FMOD_RESULT FMOD_CmdLog_Flush();

const char *FMOD_CMDLOG_FILENAME = "cmdlog.bin";
const int   FMOD_CMDLOGSIZE      = 1024 * 1024;

static char *gCmdLogBuf = 0;
static char *gCmdLogPtr = 0;
static FILE *readfp     = 0;


FMOD_RESULT FMOD_CmdLog_Init()
{
#ifndef FMOD_CMDLOG_PLAYBACK

    gCmdLogBuf = (char *)FMOD_Memory_Calloc(FMOD_CMDLOGSIZE);
    if (!gCmdLogBuf)
    {
        return FMOD_ERR_MEMORY;
    }

    FILE *fp = fopen(FMOD_CMDLOG_FILENAME, "wb");
    if (!fp)
    {
        return FMOD_ERR_FILE_BAD;
    }
    unsigned int version = FMOD_VERSION;
    fwrite(&version, 1, sizeof(version), fp);
    fclose(fp);

    gCmdLogPtr = gCmdLogBuf;

#endif

    return FMOD_OK;
}


FMOD_RESULT FMOD_CmdLog_Release()
{
#ifndef FMOD_CMDLOG_PLAYBACK

    FMOD_RESULT result = FMOD_CmdLog_Flush();
    if (result != FMOD_OK)
    {
        return result;
    }

    if (gCmdLogBuf)
    {
        FMOD_Memory_Free(gCmdLogBuf);
        gCmdLogBuf = 0;
    }

#endif

    return FMOD_OK;
}


void FMOD_CmdLog_Push(void *data, int datalen)
{
#ifndef FMOD_CMDLOG_PLAYBACK

    int           i;
    char         *d;
/*AJS
    unsigned int  t;

    FMOD_OS_Time_GetMs(&t);
    d = (char *)(&t);
    for (i=0; i < sizeof(t); i++)
    {
        *gCmdLogPtr++ = *d++;

        if ((gCmdLogPtr - gCmdLogBuf) >= FMOD_CMDLOGSIZE)
        {
            FMOD_CmdLog_Flush();
        }
    }
AJS*/

    d = (char *)data;
    for (i=0; i < datalen; i++)
    {
        *gCmdLogPtr++ = *d++;

        if ((gCmdLogPtr - gCmdLogBuf) >= FMOD_CMDLOGSIZE)
        {
            FMOD_CmdLog_Flush();
        }
    }

#endif
}


FMOD_RESULT FMOD_CmdLog_Flush()
{
#ifndef FMOD_CMDLOG_PLAYBACK

    FILE *fp = fopen(FMOD_CMDLOG_FILENAME, "ab");
    if (!fp)
    {
        FLOGC((FMOD::LOG_ERROR, __FILE__, __LINE__, "FMOD_CmdLog_Flush", "Error opening %s for writing\n", FMOD_CMDLOG_FILENAME));
        return FMOD_ERR_FILE_BAD;
    }

    int b = gCmdLogPtr - gCmdLogBuf;

    if (fwrite(gCmdLogBuf, 1, b, fp) != (unsigned int)b)
    {
        FLOGC((FMOD::LOG_ERROR, __FILE__, __LINE__, "FMOD_CmdLog_Flush", "Error writing %d bytes to %s\n", b, FMOD_CMDLOG_FILENAME));
    }

    fclose(fp);

    gCmdLogPtr = gCmdLogBuf;

#endif

    return FMOD_OK;
}



FMOD_RESULT FMOD_CmdLog_Begin()
{
    if (readfp)
    {
        return FMOD_ERR_INITIALIZED;
    }

    readfp = fopen(FMOD_CMDLOG_FILENAME, "rb");
    if (!readfp)
    {
        return FMOD_ERR_FILE_BAD;
    }

    unsigned int version;
    if (fread(&version, 1, sizeof(version), readfp) != sizeof(version))
    {
        fclose(readfp);
        return FMOD_ERR_FILE_BAD;
    }

    if (version != FMOD_VERSION)
    {
        //AJS complain?
    }

    return FMOD_OK;
}


FMOD_RESULT FMOD_CmdLog_Read(void *buf, int bytes)
{
    if (!readfp)
    {
        return FMOD_ERR_UNINITIALIZED;
    }

    if (buf)
    {
        int bytesread = fread(buf, 1, bytes, readfp);
        if (bytesread != bytes)
        {
            return FMOD_ERR_FILE_EOF;
        }
    }
    else
    {
        if (fseek(readfp, bytes, SEEK_CUR))
        {
            return FMOD_ERR_FILE_EOF;
        }
    }

    return FMOD_OK;
}


FMOD_RESULT FMOD_CmdLog_Execute()
{
    int cmd;

    FMOD_CmdLog_Read(&cmd, sizeof(int));

    switch (cmd)
    {
        case FMOD_CMDLOG_SYSTEM_PLAYSOUND :
        {
            FMOD::SystemI *_cl_system;
            FMOD_CHANNELINDEX _cl_channelid;
            FMOD::SoundI *_cl_sound;
            bool _cl_paused;
            FMOD::Channel **_cl_channel;

            FMOD_CmdLog_Read(&_cl_system, sizeof(FMOD::System *));
            FMOD_CmdLog_Read(&_cl_channelid, sizeof(FMOD_CHANNELINDEX));
            FMOD_CmdLog_Read(&_cl_sound, sizeof(FMOD::SoundI *));
            FMOD_CmdLog_Read(&_cl_paused, sizeof(bool));
            FMOD_CmdLog_Read(0, sizeof(FMOD::Channel **));

            _cl_system = (FMOD::SystemI *)FMOD::gSystemHead->getNodeByIndex(0);

            //AJS need to resolve sound and system somehow
            //AJS they're ptrs not handles - make them handles? just use indices?

            FMOD_RESULT result = _cl_system->playSound(_cl_channelid, _cl_sound, _cl_paused, 0);

            break;
        }

        default :
            break;
    }

    return FMOD_OK;
}


FMOD_RESULT FMOD_CmdLog_End()
{
    if (readfp)
    {
        fclose(readfp);
        readfp = 0;
    }

    return FMOD_OK;
}


#endif
