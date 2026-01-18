#pragma once

#include "Engine/Math/LinearAlgebra/Vector/Vector3.h"

struct Matrix3
{
    float m[3][3];

    Matrix3();
    Matrix3(float m00, float m01, float m02,
            float m10, float m11, float m12,
            float m20, float m21, float m22);

    static Matrix3 Identity();
    static Matrix3 Zero();

    Matrix3 operator*(const Matrix3 &other) const;
    Vector3 operator*(const Vector3 &v) const;

    Matrix3 Transposed() const;
    float Determinant() const;
    bool IsInvertible() const;
    Matrix3 Inverted() const;
};
