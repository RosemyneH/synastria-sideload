#include <cmath>
#include "Engine/Source/Tempest/C4Plane.h"

// VERIFY: 77db94c7 manual
// OFFSET: 0x005FEE40
C4Plane::C4Plane() {
    this->n.x = 0.0f;
    this->n.y = 0.0f;
    this->n.z = 1.0f;
    this->d = 0.0f;
}

// VERIFY: 1c5486e7 manual
// OFFSET: 0x007912C0
void C4Plane::From3Pos(const C3Vector& p0, const C3Vector& p1, const C3Vector& p2) {
    float ex = p2.x - p0.x;
    float ey = p2.y - p0.y;
    float ez = p2.z - p0.z;
    float fx = p1.x - p0.x;
    float fy = p1.y - p0.y;
    float fz = p1.z - p0.z;

    this->n.x = fy * ez - fz * ey;
    this->n.y = fz * ex - ez * fx;
    this->n.z = ey * fx - ex * fy;

    float scale = 1.0f / sqrt(this->n.z * this->n.z + this->n.y * this->n.y + this->n.x * this->n.x);
    this->n.x = this->n.x * scale;
    this->n.y = this->n.y * scale;
    this->n.z = scale * this->n.z;
    this->d = -(this->n.z * p0.z + this->n.x * p0.x + this->n.y * p0.y);
}
