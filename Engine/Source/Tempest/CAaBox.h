#ifndef ENGINE_SOURCE_TEMPEST_CAABOX_H
#define ENGINE_SOURCE_TEMPEST_CAABOX_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C3Vector.h"

class C44Matrix;

class CAaBox {
    public:
    static CAaBox Bounding(const C3Vector* points, uint32_t count);
    static CAaBox Transform(const CAaBox& box, const C44Matrix& m);

    /* 0000 */ C3Vector b;
    /* 000C */ C3Vector t;

    CAaBox() = default;
    CAaBox(const C3Vector& b, const C3Vector& t);
    explicit CAaBox(const C3Vector& point);
    bool ContainsPoint(const C3Vector& point) const;
    bool ContainsPointExclusive(const C3Vector& point) const;
    bool Intersects(const CAaBox& box) const;
    bool IntersectsXY(const CAaBox& box) const;
    bool IsEmpty() const;
    bool IsInverted() const;
    bool IsValid() const;
    void Expand(float amount);
    void Expand(const C3Vector& amount);
    C3Vector Center() const;
    void Set(float min, float max);
    CAaBox& operator*=(float scale);
    void operator+=(const C3Vector& v);
    CAaBox operator|=(const CAaBox& box);
};

CAaBox operator|(const CAaBox& l, const CAaBox& r);

#endif
