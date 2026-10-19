#ifndef ENGINE_SOURCE_TEMPEST_C33MATRIX_H
#define ENGINE_SOURCE_TEMPEST_C33MATRIX_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C3Vector.h"

class C44Matrix;
class C4Quaternion;

class C33Matrix {
    public:
    static C33Matrix Rotation(float angle);
    static C33Matrix Rotation(float angle, const C3Vector& axis, bool normalized);

    /* 0000 */ float a0;
    /* 0004 */ float a1;
    /* 0008 */ float a2;
    /* 000C */ float b0;
    /* 0010 */ float b1;
    /* 0014 */ float b2;
    /* 0018 */ float c0;
    /* 001C */ float c1;
    /* 0020 */ float c2;

    C33Matrix() = default;
    C33Matrix(float a0, float a1, float a2, float b0, float b1, float b2, float c0, float c1, float c2);
    C33Matrix(const C44Matrix& m);
    C33Matrix(const C4Quaternion& q);
    C33Matrix Adjoint() const;
    float Determinant() const;
    void FromEulerAnglesZYX(float yaw, float pitch, float roll);
    C33Matrix Inverse(float determinant) const;
    void Rotate(float angle, const C3Vector& axis, bool normalized);
    void RotateAroundY(float angle);
    void Scale(const C3Vector& scale);
    void Scale(float scale);
    void Scale(float x, float y, float z);
    bool ToEulerAnglesZYX(float& yaw, float& pitch, float& roll) const;
};

C33Matrix operator*(const C33Matrix& l, const C33Matrix& r);
C3Vector operator*(const C3Vector& v, const C33Matrix& m);
C3Vector operator*=(C3Vector& v, const C33Matrix& m);

#endif
