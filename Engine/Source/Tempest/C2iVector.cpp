#include <cstdlib>
#include "Engine/Source/Tempest/C2iVector.h"

// VERIFY: 22f22c58 auto
// OFFSET: 0x00984C70
uint32_t C2iVector::MajorAxis() const {
    return abs(this->x) <= abs(this->y);
}

// VERIFY: f2fd55ef auto
// OFFSET: 0x006839F0
bool operator<(const C2iVector& l, const C2iVector& r) {
    return l.x < r.x && l.y < r.y;
}
