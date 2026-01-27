#include "Vector3f.h"
#include "Vector2f.h"
#include <cmath>
#include <stdexcept>

const Vector3f Vector3f::ZERO = Vector3f(0.0f, 0.0f, 0.0f);
const Vector3f Vector3f::UP = Vector3f(0.0f, 1.0f, 0.0f);
const Vector3f Vector3f::RIGHT = Vector3f(1.0f, 0.0f, 0.0f);
const Vector3f Vector3f::FORWARD = Vector3f(0.0f, 0.0f, -1.0f);
Vector3f::Vector3f(const Vector3f &other)
{
    m_data[0] = other.m_data[0];
    m_data[1] = other.m_data[1];
    m_data[2] = other.m_data[2];
}
Vector3f::Vector3f(float val)
{
    m_data[0] = val;
    m_data[1] = val;
    m_data[2] = val;
}
Vector3f::Vector3f(float x, float y, float z)
{
    m_data[0] = x;
    m_data[1] = y;
    m_data[2] = z;
}

Vector3f &Vector3f::operator=(const Vector3f &other)
{
    if (this != &other)
    {
        m_data[0] = other[0];
        m_data[1] = other[1];
        m_data[2] = other[2];
    }
    return *this;
}

const float &Vector3f::operator[](int i) const
{
    return m_data[i];
}

float &Vector3f::operator[](int i)
{
    return m_data[i];
}
float &Vector3f::x()
{
    return m_data[0];
}

float &Vector3f::y()
{
    return m_data[1];
}
float &Vector3f::z()
{
    return m_data[2];
}
float Vector3f::x() const
{
    return m_data[0];
}

float Vector3f::y() const
{
    return m_data[1];
}
float Vector3f::z() const
{
    return m_data[2];
}

Vector2f Vector3f::xy() const
{
    return Vector2f(m_data[0], m_data[1]);
}

Vector2f Vector3f::xz() const
{
    return Vector2f(m_data[0], m_data[2]);
}

Vector2f Vector3f::yz() const
{
    return Vector2f(m_data[1], m_data[2]);
}

Vector3f Vector3f::xyz() const
{
    return Vector3f(m_data[0], m_data[1], m_data[2]);
}

Vector3f Vector3f::yzx() const
{
    return Vector3f(m_data[1], m_data[2], m_data[0]);
}

Vector3f Vector3f::zxy() const
{
    return Vector3f(m_data[2], m_data[0], m_data[1]);
}

float Vector3f::Length() const
{
    return std::sqrt(LengthSquared());
}

float Vector3f::LengthSquared() const
{
    return m_data[0] * m_data[0] + m_data[1] * m_data[1] + m_data[2] * m_data[2];
}

void Vector3f::Normalize()
{
    float len = Length();
    if (len > 0.0f)
    {
        m_data[0] /= len;
        m_data[1] /= len;
        m_data[2] /= len;
    }
}

Vector3f Vector3f::Normalized() const
{
    Vector3f result(*this);
    result.Normalize();
    return result;
}
Vector2f Vector3f::Homogenized() const
{
    return Vector2f(
        m_data[0] / m_data[2],
        m_data[1] / m_data[2]);
}
void Vector3f::Negate()
{
    m_data[0] = -m_data[0];
    m_data[1] = -m_data[1];
    m_data[2] = -m_data[2];
}

Vector3f Vector3f::RotateByAxixAngle(const Vector3f &axis, float angle)
{

    // Euler-Rodrigues Formula
    Vector3f result = this->xyz();
    Vector3f v = this->xyz();
    float length = axis.Length();
    if (length == 0.0f)
        length = 1.0f;
    Vector3f Naxis = axis.Normalized();

    angle /= 2.0f;
    Vector3f w = Naxis * sinf(angle);

    Vector3f wv = w ^ v;

    Vector3f wwv = w ^ wv;

    wv *= cosf(angle)*2;

    wwv *= 2;

    result += wv + wwv;

    *this = result;

    return result;
}

Vector3f &Vector3f::operator+=(const Vector3f &other)
{
    m_data[0] += other.m_data[0];
    m_data[1] += other.m_data[1];
    m_data[2] += other.m_data[2];
    return *this;
}

Vector3f &Vector3f::operator-=(const Vector3f &other)
{
    m_data[0] -= other.m_data[0];
    m_data[1] -= other.m_data[1];
    m_data[2] -= other.m_data[2];
    return *this;
}

Vector3f &Vector3f::operator*=(float scalar)
{
    m_data[0] *= scalar;
    m_data[1] *= scalar;
    m_data[2] *= scalar;
    return *this;
}

Vector3f &Vector3f::operator/=(float scalar)
{
    m_data[0] /= scalar;
    m_data[1] /= scalar;
    m_data[2] /= scalar;
    return *this;
}

// 逐分量
Vector3f operator+(const Vector3f &v0, const Vector3f &v1)
{
    return Vector3f(v0.x() + v1.x(), v0.y() + v1.y(), v0.z() + v1.z());
}
Vector3f operator-(const Vector3f &v0, const Vector3f &v1)
{
    return Vector3f(v0.x() - v1.x(), v0.y() - v1.y(), v0.z() - v1.z());
}
Vector3f operator&(const Vector3f &v0, const Vector3f &v1)
{
    return Vector3f(v0.x() * v1.x(), v0.y() * v1.y(), v0.z() * v1.z());
}
Vector3f operator/(const Vector3f &v0, const Vector3f &v1)
{
    return Vector3f(v0.x() / v1.x(), v0.y() / v1.y(), v0.z() / v1.z());
}

float operator*(const Vector3f &v0, const Vector3f &v1)
{
    return v0.x() * v1.x() + v0.y() * v1.y() + v0.z() * v1.z();
}
Vector3f operator^(const Vector3f &v0, const Vector3f &v1)
{
    return Vector3f(v0.y() * v1.z() - v0.z() * v1.y(),
                    v0.z() * v1.x() - v0.x() * v1.z(),
                    v0.x() * v1.y() - v0.y() * v1.x());
}

Vector3f operator-(const Vector3f &v)
{
    return Vector3f(-v.x(), -v.y(), -v.z());
}

Vector3f operator*(float f, const Vector3f &v)
{
    return Vector3f(f * v.x(), f * v.y(), f * v.z());
}
Vector3f operator*(const Vector3f &v, float f)
{
    return Vector3f(v.x() * f, v.y() * f, v.z() * f);
}
Vector3f operator/(const Vector3f &v, float f)
{
    if (f == 0.0f)
    {
        throw std::runtime_error("Division by zero in Vector3f operator/");
    }
    return Vector3f(v.x() / f, v.y() / f, v.z() / f);
}

float Vector3f::Distance(const Vector3f &a, const Vector3f &b)
{
    return (a - b).Length();
}

Vector3f Vector3f::Lerp(const Vector3f &a, const Vector3f &b, float t)
{
    return t * b + (1 - t) * a;
}

bool operator==(const Vector3f &v0, const Vector3f &v1)
{
    return Vector3f::Distance(v0, v1) < 1e-5f;
}

bool operator!=(const Vector3f &v0, const Vector3f &v1)
{
    return !(v0 == v1);
}