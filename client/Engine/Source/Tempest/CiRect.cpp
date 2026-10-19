#include "Engine/Source/Tempest/CiRect.h"

// VERIFY: 224583e3 manual
// OFFSET: 0x00683A20
CiRect operator&(const CiRect& l, const CiRect& r) {
    int32_t maxX = l.maxX >= r.maxX ? r.maxX : l.maxX;
    int32_t maxY = l.maxY >= r.maxY ? r.maxY : l.maxY;
    int32_t minX = l.minX <= r.minX ? r.minX : l.minX;
    int32_t minY = l.minY > r.minY ? l.minY : r.minY;

    CiRect result;
    result.minX = minX;
    result.minY = minY;
    result.maxY = maxY;
    result.maxX = maxX;
    return result;
}

// VERIFY: 3daad48c manual
// OFFSET: 0x00683A80
CiRect CiRect::operator&=(const CiRect& r) {
    *this = *this & r;
    return *this;
}
