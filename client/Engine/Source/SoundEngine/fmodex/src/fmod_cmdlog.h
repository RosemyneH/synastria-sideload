#ifndef _FMOD_CMDLOG_H
#define _FMOD_CMDLOG_H

#ifdef FMOD_SUPPORT_CMDLOG


#ifndef _FMOD_H
#include "fmod.h"
#endif


// don't change the order!
enum
{
    FMOD_CMDLOG_SYSTEM_PLAYSOUND = 0,
};


extern FMOD_RESULT FMOD_CmdLog_Init();
extern FMOD_RESULT FMOD_CmdLog_Release();
extern void FMOD_CmdLog_Push(void *data, int datalen);

extern FMOD_RESULT FMOD_CmdLog_Begin();
extern FMOD_RESULT FMOD_CmdLog_Execute();
extern FMOD_RESULT FMOD_CmdLog_End();

#endif

#endif
