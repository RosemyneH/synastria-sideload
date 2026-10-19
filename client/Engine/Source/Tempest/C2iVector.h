#ifndef ENGINE_SOURCE_TEMPEST_C2IVECTOR_H
#define ENGINE_SOURCE_TEMPEST_C2IVECTOR_H

#include <cstddef>
#include <cstdint>

class C2iVector {
    public:
    /* 0000 */ int32_t x;
    /* 0004 */ int32_t y;

    uint32_t MajorAxis() const;
};

bool operator<(const C2iVector& l, const C2iVector& r);

#endif
