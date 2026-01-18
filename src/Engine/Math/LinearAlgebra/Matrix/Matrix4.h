#pragma once

#include "Engine/Math/LinearAlgebra/Vector/Vector3.h"
#include "Engine/Math/LinearAlgebra/Vector/Vector4.h"

struct Matrix4
{
    float m[4][4];

    Matrix4();
    Matrix4(float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33);

    static Matrix4 Identity();
    static Matrix4 Zero();
    static Matrix4 Translation(const Vector3 &t);
    static Matrix4 Scale(const Vector3 &s);

    Matrix4 operator*(const Matrix4 &other) const;
    Vector4 operator*(const Vector4 &v) const;

    Matrix4 Transposed() const;
};
