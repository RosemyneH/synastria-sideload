#ifndef WOW_SOURCE_WORLDCLIENT_FRAMETIME_H
#define WOW_SOURCE_WORLDCLIENT_FRAMETIME_H

#include <cstddef>
#include <cstdint>

class FrameTime {
    public:
    static int32_t s_tickTimeFixed;
    static float s_tickTimeSec;
    static float s_curTimeSec;
    static uint32_t s_tickTimeMs;
    static uint32_t s_curTimeMs;

    static void Update(float elapsedSec, uint32_t time);
};

#endif
