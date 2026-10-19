#ifndef ENGINE_SOURCE_TEMPEST_C44MATRIX_H
#define ENGINE_SOURCE_TEMPEST_C44MATRIX_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C3Vector.h"
#include "Engine/Source/Tempest/C4Vector.h"

class C33Matrix;
class C34Matrix;
class C4Quaternion;

class C44Matrix {
    public:
    static void FromQuaternion(const C4Quaternion& q, C44Matrix& m);
    static C44Matrix Rotation(float angle, const C3Vector& axis, bool normalized);
    static C44Matrix RotationAroundX(float angle);
    static C44Matrix RotationAroundY(float angle);
    static C44Matrix RotationAroundZ(float angle);

    /* 0000 */ float a0;
    /* 0004 */ float a1;
    /* 0008 */ float a2;
    /* 000C */ float a3;
    /* 0010 */ float b0;
    /* 0014 */ float b1;
    /* 0018 */ float b2;
    /* 001C */ float b3;
    /* 0020 */ float c0;
    /* 0024 */ float c1;
    /* 0028 */ float c2;
    /* 002C */ float c3;
    /* 0030 */ float d0;
    /* 0034 */ float d1;
    /* 0038 */ float d2;
    /* 003C */ float d3;

    C44Matrix();
    explicit C44Matrix(float scale);
    C44Matrix(float a0, float a1, float a2, float a3, float b0, float b1, float b2, float b3, float c0, float c1, float c2, float c3, float d0, float d1, float d2, float d3);
    C44Matrix(const C33Matrix& m);
    C44Matrix(const C34Matrix& m);
    C44Matrix(const C3Vector& x, const C3Vector& y, const C3Vector& z, const C3Vector& w);
    C44Matrix(const C4Quaternion& q);
    C44Matrix Adjoint() const;
    C44Matrix AffineInverse() const;
    C44Matrix AffineInverse(float scale) const;
    float Determinant() const;
    void Identity();
    C44Matrix Inverse() const;
    C44Matrix Inverse(float determinant) const;
    void Rotate(const C4Quaternion& q);
    void RotateAroundX(float angle);
    void RotateAroundY(float angle);
    void RotateAroundZ(float angle);
    void Scale(const C3Vector& scale);
    void Scale(float scale);
    void Translate(const C3Vector& move);
    C44Matrix Transpose() const;
    C44Matrix& operator*=(const C44Matrix& m);
    C44Matrix& operator*=(float s);
    C44Matrix& operator=(const C44Matrix& m);
    C44Matrix& operator=(const C4Quaternion& q);
};

bool operator!=(const C44Matrix& l, const C44Matrix& r);
bool operator==(const C44Matrix& l, const C44Matrix& r);
C44Matrix operator*(const C44Matrix& l, const C44Matrix& r);
C44Matrix operator*(const C44Matrix& l, float s);
C3Vector operator*(const C3Vector& v, const C44Matrix& m);
C3Vector operator*(const C44Matrix& m, const C3Vector& v);
C4Vector operator*(const C4Vector& v, const C44Matrix& m);
C3Vector operator*=(C3Vector& v, const C44Matrix& m);
C44Matrix operator+(const C44Matrix& l, const C44Matrix& r);
C3Vector TransformDirection(const C3Vector& v, const C44Matrix& m);

#endif
