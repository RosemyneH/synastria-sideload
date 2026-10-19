#ifndef ENGINE_SOURCE_TEMPEST_CRECT_H
#define ENGINE_SOURCE_TEMPEST_CRECT_H

#include <cstddef>
#include <cstdint>

class C2Vector;

class CRect {
    public:
    /* 0000 */ float minY;
    /* 0004 */ float minX;
    /* 0008 */ float maxY;
    /* 000C */ float maxX;

    bool ContainsPoint(const C2Vector& point) const;
    bool IsPointInside(const C2Vector& point) const;
    CRect operator&=(const CRect& r);
};

bool operator!=(const CRect& l, const CRect& r);
CRect operator&(const CRect& l, const CRect& r);
CRect operator+(const CRect& l, const CRect& r);
CRect operator/(const CRect& l, const CRect& r);
CRect operator|(const CRect& l, const CRect& r);

#endif
