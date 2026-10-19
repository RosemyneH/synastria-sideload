#ifndef ENGINE_SOURCE_TEMPEST_CRANGE_H
#define ENGINE_SOURCE_TEMPEST_CRANGE_H

#include <cstddef>
#include <cstdint>

class CRndSeed;

class CRange {
    public:
    /* 0000 */ float m_l;
    /* 0004 */ float m_h;

    float Random(CRndSeed& seed);
};

class CiRange {
    public:
    /* 0000 */ int32_t m_l;
    /* 0004 */ int32_t m_h;
};

#endif
