#include "Engine/Source/Tempest/CFacet.h"

// VERIFY: 9b1e41ee manual
// OFFSET: 0x005FED80
CFacet::CFacet(float value) {
    this->plane.n.x = 0.0f;
    this->plane.n.y = 0.0f;
    this->plane.n.z = 1.0f;
    this->plane.d = 0.0f;
    this->Set(value);
}

// VERIFY: 990c102f manual
// OFFSET: 0x00983940
CFacet::CFacet(const C3Vector& v0, const C3Vector& v1, const C3Vector& v2) {
    this->v[0] = v0;
    this->v[1] = v1;
    this->v[2] = v2;
    this->plane.From3Pos(v0, v1, v2);
}

// VERIFY: 943bd1be manual
// OFFSET: 0x009838D0
void CFacet::Set(float value) {
    this->v[2].x = value;
    this->v[1].x = value;
    this->v[0].x = value;
    this->v[2].y = value;
    this->v[1].y = value;
    this->v[0].y = value;
    this->plane.n.x = 0.0f;
    this->plane.n.y = 0.0f;
    this->v[2].z = value;
    this->v[1].z = value;
    this->v[0].z = value;
    this->plane.n.z = 1.0f;
    this->plane.d = (this->v[0].y + this->v[0].x) * -0.0f - this->v[0].z;
}
