#include "Vector4.h"
#include <cmath>

Vector4::Vector4(float x, float y, float z, float w)
    : x(x), y(y), z(z), w(w)
{
}

Vector4 Vector4::operator+(const Vector4 &other) const
{
    return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}

Vector4 Vector4::operator-(const Vector4 &other) const
{
    return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}

Vector4 Vector4::operator*(float scalar) const
{
    return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vector4 Vector4::operator/(float scalar) const
{
    return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
}

Vector4 &Vector4::operator+=(const Vector4 &other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

Vector4 &Vector4::operator-=(const Vector4 &other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

Vector4 &Vector4::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

Vector4 &Vector4::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

float Vector4::Length() const
{
    return std::sqrt(LengthSquared());
}

float Vector4::LengthSquared() const
{
    return x * x + y * y + z * z + w * w;
}

void Vector4::Normalize()
{
    float len = Length();
    if (len > 0.0f)
    {
        x /= len;
        y /= len;
        z /= len;
        w /= len;
    }
}

Vector4 Vector4::Normalized() const
{
    Vector4 result(*this);
    result.Normalize();
    return result;
}

float Vector4::Dot(const Vector4 &other) const
{
    return x * other.x + y * other.y + z * other.z + w * other.w;
}

float Vector4::Distance(const Vector4 &a, const Vector4 &b)
{
    return (a - b).Length();
}

Vector4 Vector4::Lerp(const Vector4 &a, const Vector4 &b, float t)
{
    return Vector4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t);
}
