#include "Vector2f.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

const Vector2f Vector2f::ZERO = Vector2f( 0, 0 );

const Vector2f Vector2f::UP = Vector2f( 0, 1 );

const Vector2f Vector2f::RIGHT = Vector2f( 1, 0 );
Vector2f::Vector2f(float val)
{
    m_data[0] = val;
    m_data[1] = val;
}
Vector2f::Vector2f(const Vector2f &other)
{
    m_data[0] = other.m_data[0];
    m_data[1] = other.m_data[1];
}
Vector2f::Vector2f(float x, float y)
{
    m_data[0] = x;
    m_data[1] = y;
}


Vector2f& Vector2f::operator = ( const Vector2f& rv )
{
 	if( this != &rv )
	{
        m_data[0] = rv[0];
        m_data[1] = rv[1];
    }
    return *this;
}

const float& Vector2f::operator [] ( int i ) const
{
    return m_data[i];
}

float& Vector2f::operator [] ( int i )
{
    return m_data[i];
}
float& Vector2f::x()
{
    return m_data[0];
}

float& Vector2f::y()
{
    return m_data[1];
}
float Vector2f::x() const
{
    return m_data[0];
}

float Vector2f::y() const
{
    return m_data[1];
}
Vector2f Vector2f::xy() const
{
    return *this;
}
Vector2f Vector2f::yx() const
{
    return Vector2f(m_data[1], m_data[0]);
}
Vector2f Vector2f::xx() const
{
    return Vector2f(m_data[0], m_data[0]);
}
Vector2f Vector2f::yy() const
{
    return Vector2f(m_data[1], m_data[1]);
}
Vector2f Vector2f::Normal() const
{
    return Vector2f(-m_data[1], m_data[0]);
}

void Vector2f::Negate()
{
    m_data[0] = -m_data[0];
    m_data[1] = -m_data[1];
}


Vector2f operator+(const Vector2f &v0, const Vector2f &v1)
{
    return Vector2f(v0.x() + v1.x(), v0.y() + v1.y());
}

Vector2f operator-(const Vector2f &v0, const Vector2f &v1)
{
    return Vector2f(v0.x() - v1.x(), v0.y() - v1.y());
}

Vector2f operator&(const Vector2f &v0, const Vector2f &v1)
{
    return Vector2f(v0.x() * v1.x(), v0.y() * v1.y());
}
Vector2f operator/(const Vector2f &v0, const Vector2f &v1)
{
    return Vector2f(v0.x() / v1.x(), v0.y() / v1.y());
}

float operator * ( const Vector2f& v0, const Vector2f& v1  )
{
    return v0.x() * v1.x() + v0.y() * v1.y();
}
float operator ^ ( const Vector2f& v0, const Vector2f& v1 )
{
    return v0.x() * v1.y() - v0.y() * v1.x();
}

Vector2f operator - ( const Vector2f& v )
{
    return Vector2f(-v.x(), -v.y());
}

Vector2f operator * ( float f, const Vector2f& v )
{
    return Vector2f(v.x() * f, v.y() * f);
}
Vector2f operator * ( const Vector2f& v, float f )
{
    return Vector2f(v.x() * f, v.y() * f);
}
Vector2f operator / ( const Vector2f& v, float f )
{
    if (f == 0.0f)
    {
        // TODO:无限远向量
        throw std::runtime_error("Division by zero in Vector2f operator/");
    }
    return Vector2f(v.x() / f, v.y() / f);
}

bool operator == ( const Vector2f& v0, const Vector2f& v1 )
{
    return Vector2f::Distance(v0, v1) < 1e-5f;
}
bool operator != ( const Vector2f& v0, const Vector2f& v1 )
{
    return !(v0 == v1);
}

Vector2f &Vector2f::operator+=(const Vector2f &other)
{
    m_data[0] += other.m_data[0];
    m_data[1] += other.m_data[1];
    return *this;
}

Vector2f &Vector2f::operator-=(const Vector2f &other)
{
    m_data[0] -= other.m_data[0];
    m_data[1] -= other.m_data[1];
    return *this;
}

Vector2f &Vector2f::operator*=(float scalar)
{
    m_data[0] *= scalar;
    m_data[1] *= scalar;
    return *this;
}

Vector2f &Vector2f::operator/=(float scalar)
{
    m_data[0] /= scalar;
    m_data[1] /= scalar;
    return *this;
}

float Vector2f::Length() const
{
    return std::sqrt(LengthSquared());
}

float Vector2f::LengthSquared() const
{
    return m_data[0] * m_data[0] + m_data[1] * m_data[1];
}

void Vector2f::Normalize()
{
    float len = Length();
    if (len > 0.0f)
    {
        m_data[0] /= len;
        m_data[1] /= len;
    }
}

Vector2f Vector2f::Normalized() const
{
    Vector2f result(*this);
    result.Normalize();
    return result;
}


float Vector2f::Distance(const Vector2f &a, const Vector2f &b)
{
    return (a - b).Length();
}

Vector2f Vector2f::Lerp(const Vector2f &a, const Vector2f &b, float t)
{
    // Clamp t to [0, 1]
    t = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
    return t*b + (1 - t)*a;
}
