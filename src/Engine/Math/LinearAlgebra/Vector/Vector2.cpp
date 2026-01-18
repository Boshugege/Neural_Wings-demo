#include "Vector2.h"
#include <cmath>

Vector2::Vector2(float x, float y)
    : x(x), y(y)
{
}

Vector2 Vector2::operator+(const Vector2 &other) const
{
    return Vector2(x + other.x, y + other.y);
}

Vector2 Vector2::operator-(const Vector2 &other) const
{
    return Vector2(x - other.x, y - other.y);
}

Vector2 Vector2::operator*(float scalar) const
{
    return Vector2(x * scalar, y * scalar);
}

Vector2 Vector2::operator/(float scalar) const
{
    return Vector2(x / scalar, y / scalar);
}

Vector2 &Vector2::operator+=(const Vector2 &other)
{
    x += other.x;
    y += other.y;
    return *this;
}

Vector2 &Vector2::operator-=(const Vector2 &other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

Vector2 &Vector2::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}

Vector2 &Vector2::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}

float Vector2::Length() const
{
    return std::sqrt(LengthSquared());
}

float Vector2::LengthSquared() const
{
    return x * x + y * y;
}

void Vector2::Normalize()
{
    float len = Length();
    if (len > 0.0f)
    {
        x /= len;
        y /= len;
    }
}

Vector2 Vector2::Normalized() const
{
    Vector2 result(*this);
    result.Normalize();
    return result;
}

float Vector2::Dot(const Vector2 &other) const
{
    return x * other.x + y * other.y;
}

float Vector2::Distance(const Vector2 &a, const Vector2 &b)
{
    return (a - b).Length();
}

Vector2 Vector2::Lerp(const Vector2 &a, const Vector2 &b, float t)
{
    return Vector2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}
