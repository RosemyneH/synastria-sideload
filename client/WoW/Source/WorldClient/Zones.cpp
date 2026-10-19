#include "Zones.h"
#include <cfloat>
#include "Engine/Source/Storm/H/storm.h"

// INLINED
ZonePath::ZonePath(const char* path) {
    this->m_offsetX = -1.66233754f;
    this->m_offsetY = -145.731598f;
    this->m_path = path;
    this->m_minX = FLT_MAX;
    this->m_minY = FLT_MAX;
    this->m_maxX = -FLT_MAX;
    this->m_maxY = -FLT_MAX;
}

// INLINED
ZonePath::~ZonePath() {
    if (this->m_path && this->m_points) {
        SMemFree(this->m_points, __FILE__, __LINE__, 0);
    }
}
