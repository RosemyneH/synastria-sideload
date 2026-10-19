#include <cmath>
#include "Engine/Source/Tempest/C2Vector.h"
#include "Engine/Source/Tempest/C3Vector.h"

// VERIFY: b7ad0a59 auto
// OFFSET: 0x004079C0
C2Vector::C2Vector() {
    this->x = 0.0f;
    this->y = 0.0f;
}

// INLINED
C2Vector::C2Vector(float x, float y) {
    this->x = x;
    this->y = y;
}

// VERIFY: 955bf3fe manual
// OFFSET: 0x0048B820
void C2Vector::NormalizeSafe() {
    float length = this->y * this->y + this->x * this->x;

    if (length > 0.00000023841858f) {
        float scale = 1.0f / sqrt(length);
        this->x = this->x * scale;
        this->y = scale * this->y;
    }
}

// VERIFY: 1302d485 manual
// OFFSET: 0x004C4DF0
C2Vector::C2Vector(const C3Vector& v) {
    this->x = v.x;
    this->y = v.y;
}

// VERIFY: 0582b503 manual
// OFFSET: 0x006FF6A0
void C2Vector::Normalize() {
    float scale = 1.0f / sqrt(this->x * this->x + this->y * this->y);
    this->x = this->x * scale;
    this->y = scale * this->y;
}
