#include "Vector3.h"
#include <cmath>

Vector3::Vector3(float x, float y, float z)
    : x(x), y(y), z(z)
{
}

Vector3 Vector3::operator+(const Vector3 &other) const
{
    return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::operator-(const Vector3 &other) const
{
    return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 Vector3::operator*(float scalar) const
{
    return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3 Vector3::operator/(float scalar) const
{
    return Vector3(x / scalar, y / scalar, z / scalar);
}

Vector3 &Vector3::operator+=(const Vector3 &other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vector3 &Vector3::operator-=(const Vector3 &other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vector3 &Vector3::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vector3 &Vector3::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

float Vector3::Length() const
{
    return std::sqrt(LengthSquared());
}

float Vector3::LengthSquared() const
{
    return x * x + y * y + z * z;
}

void Vector3::Normalize()
{
    float len = Length();
    if (len > 0.0f)
    {
        x /= len;
        y /= len;
        z /= len;
    }
}

Vector3 Vector3::Normalized() const
{
    Vector3 result(*this);
    result.Normalize();
    return result;
}

float Vector3::Dot(const Vector3 &other) const
{
    return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::Cross(const Vector3 &other) const
{
    return Vector3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x);
}

float Vector3::Distance(const Vector3 &a, const Vector3 &b)
{
    return (a - b).Length();
}

Vector3 Vector3::Lerp(const Vector3 &a, const Vector3 &b, float t)
{
    return Vector3(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t);
}
