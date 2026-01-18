#include "Matrix3.h"
#include "Engine/Math/Core/MathCommon.h"
#include <cmath>

Matrix3::Matrix3()
{
    *this = Matrix3::Identity();
}

Matrix3::Matrix3(float m00, float m01, float m02,
                 float m10, float m11, float m12,
                 float m20, float m21, float m22)
{
    m[0][0] = m00;
    m[0][1] = m01;
    m[0][2] = m02;
    m[1][0] = m10;
    m[1][1] = m11;
    m[1][2] = m12;
    m[2][0] = m20;
    m[2][1] = m21;
    m[2][2] = m22;
}

Matrix3 Matrix3::Identity()
{
    return Matrix3(
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f);
}

Matrix3 Matrix3::Zero()
{
    return Matrix3(
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f);
}

Matrix3 Matrix3::operator*(const Matrix3 &other) const
{
    Matrix3 result = Matrix3::Zero();
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            result.m[r][c] =
                m[r][0] * other.m[0][c] +
                m[r][1] * other.m[1][c] +
                m[r][2] * other.m[2][c];
        }
    }
    return result;
}

Vector3 Matrix3::operator*(const Vector3 &v) const
{
    return Vector3(
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);
}

Matrix3 Matrix3::Transposed() const
{
    return Matrix3(
        m[0][0], m[1][0], m[2][0],
        m[0][1], m[1][1], m[2][1],
        m[0][2], m[1][2], m[2][2]);
}

float Matrix3::Determinant() const
{
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
           m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
           m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

bool Matrix3::IsInvertible() const
{
    return std::fabs(Determinant()) > MATH_EPSILON;
}

Matrix3 Matrix3::Inverted() const
{
    float det = Determinant();
    if (std::fabs(det) <= MATH_EPSILON)
    {
        return Matrix3::Zero();
    }

    float invDet = 1.0f / det;
    Matrix3 result;
    result.m[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
    result.m[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * invDet;
    result.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;
    result.m[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * invDet;
    result.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
    result.m[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * invDet;
    result.m[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
    result.m[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * invDet;
    result.m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;
    return result;
}
