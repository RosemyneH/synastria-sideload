#ifndef ENGINE_SOURCE_OS_TIME_H
#define ENGINE_SOURCE_OS_TIME_H

#include <cstdint>
#include "Engine/Source/Os/TimeManager.h"

float OsGetAsyncTimeSec();
void OsGetTimeStamp(char* buffer, uint32_t size);
int32_t OsGetTimeZone();
void OsTimeStartup(OsTimeManager::TimingMethod method);
void OsTimeShutdown();

#endif
