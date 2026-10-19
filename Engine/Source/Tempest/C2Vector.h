#ifndef ENGINE_SOURCE_TEMPEST_C2VECTOR_H
#define ENGINE_SOURCE_TEMPEST_C2VECTOR_H

#include <cstddef>
#include <cstdint>

class C3Vector;

class C2Vector {
    public:
    /* 0000 */ float x;
    /* 0004 */ float y;

    C2Vector();
    C2Vector(float x, float y);
    C2Vector(const C3Vector& v);
    void Normalize();
    void NormalizeSafe();
};

#endif
