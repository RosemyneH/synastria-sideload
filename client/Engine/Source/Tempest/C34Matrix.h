#ifndef ENGINE_SOURCE_TEMPEST_C34MATRIX_H
#define ENGINE_SOURCE_TEMPEST_C34MATRIX_H

#include <cstddef>
#include <cstdint>

class C34Matrix {
    public:
    /* 0000 */ float a0;
    /* 0004 */ float a1;
    /* 0008 */ float a2;
    /* 000C */ float b0;
    /* 0010 */ float b1;
    /* 0014 */ float b2;
    /* 0018 */ float c0;
    /* 001C */ float c1;
    /* 0020 */ float c2;
    /* 0024 */ float d0;
    /* 0028 */ float d1;
    /* 002C */ float d2;

    void RotateAroundZ(float angle);
};

C34Matrix operator*(const C34Matrix& l, const C34Matrix& r);

#endif
