#include <cmath>
#include "Engine/Source/Tempest/C34Matrix.h"

// VERIFY: bd72ae6c auto
// OFFSET: 0x00984310
C34Matrix operator*(const C34Matrix& l, const C34Matrix& r) {
    C34Matrix result;
    result.a0 = l.a1 * r.b0 + r.c0 * l.a2 + l.a0 * r.a0;
    result.a1 = r.c1 * l.a2 + l.a0 * r.a1 + l.a1 * r.b1;
    result.a2 = l.a1 * r.b2 + r.a2 * l.a0 + r.c2 * l.a2;
    result.b0 = l.b0 * r.a0 + r.c0 * l.b2 + r.b0 * l.b1;
    result.b1 = l.b0 * r.a1 + r.c1 * l.b2 + r.b1 * l.b1;
    result.b2 = l.b2 * r.c2 + l.b1 * r.b2 + l.b0 * r.a2;
    result.c0 = r.c0 * l.c2 + l.c1 * r.b0 + l.c0 * r.a0;
    result.c1 = l.c0 * r.a1 + r.c1 * l.c2 + r.b1 * l.c1;
    result.c2 = l.c2 * r.c2 + l.c1 * r.b2 + l.c0 * r.a2;
    result.d0 = l.d1 * r.b0 + l.d2 * r.c0 + r.a0 * l.d0 + r.d0;
    result.d1 = l.d2 * r.c1 + l.d1 * r.b1 + l.d0 * r.a1 + r.d1;
    result.d2 = l.d2 * r.c2 + l.d1 * r.b2 + r.a2 * l.d0 + r.d2;
    return result;
}

// VERIFY: 7cf7c9c4 manual
// OFFSET: 0x00984450
void C34Matrix::RotateAroundZ(float angle) {
    float c = cos(angle);
    float s = sin(angle);

    C34Matrix rotation;
    rotation.a0 = c;
    rotation.a1 = s;
    rotation.a2 = 0.0f;
    rotation.b0 = -s;
    rotation.b1 = c;
    rotation.b2 = 0.0f;
    rotation.c0 = 0.0f;
    rotation.c1 = 0.0f;
    rotation.c2 = 1.0f;
    rotation.d0 = 0.0f;
    rotation.d1 = 0.0f;
    rotation.d2 = 0.0f;

    *this = rotation * *this;
}
