#ifndef WOW_SOURCE_WORLDCLIENT_WORLDVIEW_H
#define WOW_SOURCE_WORLDCLIENT_WORLDVIEW_H

#include <cstdint>

class C3Vector;

class CWorldView {
    public:
    static float s_fadeDistSqr[5];
    static float s_fadeSize[5];

    static void AddOccluder(const C3Vector& a, const C3Vector& b);
    static int16_t GetFadeLevelForDistance(float distance);
};

#endif
