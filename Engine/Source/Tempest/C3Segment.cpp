#include "Engine/Source/Tempest/C3Segment.h"

// VERIFY: 92f6ed53 auto
// OFFSET: 0x0078F4D0
C3Vector C3Segment::Lerp(float t) const {
    float y = (this->t.y - this->b.y) * t;
    float z = (this->t.z - this->b.z) * t;
    return C3Vector((this->t.x - this->b.x) * t + this->b.x, y + this->b.y, z + this->b.z);
}
