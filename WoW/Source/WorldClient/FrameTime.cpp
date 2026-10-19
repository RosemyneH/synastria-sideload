#include "FrameTime.h"
#include "WoW/Source/WorldClient/MapWeather.h"
#include "WoW/Source/WorldClient/World.h"

// STATIC: 0x00CD7698
int32_t FrameTime::s_tickTimeFixed;

// STATIC: 0x00CD76A0
float FrameTime::s_tickTimeSec;

// STATIC: 0x00CD76A4
float FrameTime::s_curTimeSec;

// STATIC: 0x00CD76A8
uint32_t FrameTime::s_tickTimeMs;

// STATIC: 0x00CD76AC
uint32_t FrameTime::s_curTimeMs;

// VERIFY: 99f83708 manual
// OFFSET: 0x0077ECB0
void FrameTime::Update(float elapsedSec, uint32_t time) {
    FrameTime::s_curTimeMs = time;
    FrameTime::s_tickTimeMs = static_cast<int32_t>(elapsedSec * 1000.0f);
    FrameTime::s_curTimeSec = static_cast<float>(FrameTime::s_curTimeMs) * 0.001f;
    FrameTime::s_tickTimeSec = elapsedSec;
    CWorld::s_elapsedTimeSec += elapsedSec;

    int32_t ticks = Weather::TimeToFixed(elapsedSec);
    CWorld::s_elapsedTicks += ticks;
    FrameTime::s_tickTimeFixed = ticks;
}
