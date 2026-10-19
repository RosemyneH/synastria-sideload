#include "Engine/Source/Tempest/C2Vector.h"
#include "Engine/Source/Tempest/CRect.h"

// VERIFY: 58e5bdcd auto
// OFFSET: 0x0047FC40
bool CRect::ContainsPoint(const C2Vector& point) const {
    return this->minX < point.x && this->maxX > point.x && this->minY < point.y && this->maxY > point.y;
}

// VERIFY: 9e372182 manual
// OFFSET: 0x00488D40
bool CRect::IsPointInside(const C2Vector& point) const {
    return this->minX <= point.x && this->maxX >= point.x && this->minY <= point.y && this->maxY >= point.y;
}

// VERIFY: e12283dd auto
// OFFSET: 0x0048EDE0
CRect CRect::operator&=(const CRect& r) {
    *this = *this & r;
    return *this;
}

// VERIFY: d0f2b73f auto
// OFFSET: 0x0048ED60
CRect operator&(const CRect& l, const CRect& r) {
    float maxX = r.maxX <= l.maxX ? r.maxX : l.maxX;
    float maxY = r.maxY <= l.maxY ? r.maxY : l.maxY;
    float minX = r.minX >= l.minX ? r.minX : l.minX;

    CRect result;
    result.minY = r.minY >= l.minY ? r.minY : l.minY;
    result.minX = minX;
    result.maxY = maxY;
    result.maxX = maxX;
    return result;
}

// VERIFY: b5d6979a manual
// OFFSET: 0x0078F2F0
CRect operator|(const CRect& l, const CRect& r) {
    float maxX = r.maxX >= l.maxX ? r.maxX : l.maxX;
    float maxY = r.maxY >= l.maxY ? r.maxY : l.maxY;
    float minX = r.minX <= l.minX ? r.minX : l.minX;

    CRect result;
    result.minY = r.minY <= l.minY ? r.minY : l.minY;
    result.minX = minX;
    result.maxY = maxY;
    result.maxX = maxX;
    return result;
}

// VERIFY: cb7d68e7 manual
// OFFSET: 0x007A6DA0
CRect operator+(const CRect& l, const CRect& r) {
    CRect result;
    result.minY = l.minY + r.minY;
    result.minX = l.minX + r.minX;
    result.maxY = l.maxY + r.maxY;
    result.maxX = l.maxX + r.maxX;
    return result;
}

// VERIFY: a12d3dd3 manual
// OFFSET: 0x007A6DD0
CRect operator/(const CRect& l, const CRect& r) {
    CRect result;
    result.minY = l.minY / r.minY;
    result.minX = l.minX / r.minX;
    result.maxY = l.maxY / r.maxY;
    result.maxX = l.maxX / r.maxX;
    return result;
}

// VERIFY: bfdfabff auto
// OFFSET: 0x004F5D90
bool operator!=(const CRect& l, const CRect& r) {
    return r.minY != l.minY || r.minX != l.minX || r.maxY != l.maxY || r.maxX != l.maxX;
}

