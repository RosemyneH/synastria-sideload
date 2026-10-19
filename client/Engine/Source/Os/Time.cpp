#include <cstdint>
#include "Engine/Source/Os/Time.h"
#include "Engine/Source/Os/TimeManager.h"

// VERIFY: de97a1c7 auto
// OFFSET: 0x0086D400
float OsGetAsyncTimeSec() {
    return static_cast<float>(OsGetAsyncTimeMs()) * 0.001f;
}

// VERIFY: b4efc621 auto
// OFFSET: 0x0086D430
void OsTimeStartup(OsTimeManager::TimingMethod method) {
    OsTimeManagerCreate(method);
}

// VERIFY: f2a82140 auto
// OFFSET: 0x0086D440
void OsTimeShutdown() {
    OsTimeManagerDestroy();
}
