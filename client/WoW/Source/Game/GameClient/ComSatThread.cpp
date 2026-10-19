#include "ComSatThread.h"
#include "Engine/Source/ComSat/ComSatInterface.h"
#include "Engine/Source/Os/Input.h"
#include "Engine/Source/Os/TimeManager.h"
#include "Engine/Source/SoundEngine/SoundEngine.h"
#include "WoW/Source/Game/GameClient/ComSatClient.h"
#include "Engine/Source/Storm/Source/SLock.h"
#include "Engine/Source/Storm/Source/SThread.h"

// STATIC: 0x00D37D20
int32_t ComSatThread::s_threadRunning;

// STATIC: 0x00D37D24
ComSat::Interface* ComSatThread::s_comsat;

// STATIC: 0x00D37D28
int32_t s_pushToTalkDown;

// STATIC: 0x00D37D2C
int32_t s_localTalkerActive;

// STATIC: 0x00D37D58
SThread ComSatThread::s_comsatThread;

// STATIC: 0x00D37D84
SEvent ComSatThread::s_threadShutdownEvent(0, 0);

// VERIFY: acb19d40 auto
// OFFSET: 0x007DBA00
static void QueryPushToTalkKey() {
    if (s_voiceActivation) {
        return;
    }

    s_pushToTalkDown = s_pushToTalkModifiers != 0;

    if (s_pushToTalkKey != -1) {
        s_pushToTalkDown = OsInputIsKeyDown(static_cast<KEY>(s_pushToTalkKey));
    }

    if (s_pushToTalkMouseButton) {
        s_pushToTalkDown = OsInputIsMouseButtonDown(static_cast<MOUSEBUTTON>(s_pushToTalkMouseButton));
    }

    if (s_pushToTalkDown && s_pushToTalkModifiers) {
        for (int32_t i = 0; i <= 5; i++) {
            if ((s_pushToTalkModifiers & (1 << i)) && !OsInputIsKeyDown(static_cast<KEY>(i))) {
                s_pushToTalkDown = 0;
                break;
            }
        }
    }

    ComSatThread::s_comsat->LocalTalkerEnable(0, s_pushToTalkDown != 0);
}

// VERIFY: 82445c21 auto
// OFFSET: 0x007DC950
uint32_t STORMAPI ComSatThread::Work(void* param) {
    while (ComSatThread::s_threadShutdownEvent.Wait(0)) {
        s_comsatDeviceLock.Enter();
        QueryPushToTalkKey();
        SE2::CaptureWork();
        uint32_t now = OsGetAsyncTimeMs();

        if (ComSatThread::s_comsat) {
            ComSatThread::s_comsat->Work(now);
        }

        s_comsatDeviceLock.Leave();
        OsSleep(s_comsatSleepTime);
    }

    return 0;
}
