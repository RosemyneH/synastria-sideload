#ifndef ENGINE_SOURCE_TEMPEST_C4QUATERNION_H
#define ENGINE_SOURCE_TEMPEST_C4QUATERNION_H

#include <cstddef>
#include <cstdint>

class C33Matrix;
class C3Vector;
class C44Matrix;
class C4Quaternion;

struct C4QuaternionCompressed {
    /* 0000 */ int64_t m_packed;

    C4QuaternionCompressed& operator=(const C4Quaternion& q);
};

class C4Quaternion {
    public:
    static C4Quaternion Nlerp(float t, const C4Quaternion& a, const C4Quaternion& b);
    static C4Quaternion Slerp(float t, const C4Quaternion& a, const C4Quaternion& b);

    /* 0000 */ float x;
    /* 0004 */ float y;
    /* 0008 */ float z;
    /* 000C */ float w;

    C4Quaternion() = default;
    C4Quaternion(const C4QuaternionCompressed& compressed);
    void FromAngleAxis(float angle, const C3Vector& axis);
    void FromRotationMatrix(const C33Matrix& m);
    void FromRotationMatrix(const C44Matrix& m);
    void NormalizeApprox();
};

C4Quaternion operator*(const C4Quaternion& l, const C4Quaternion& r);

#endif
