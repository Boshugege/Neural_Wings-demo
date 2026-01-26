#pragma once
#include "raylib.h"
#include "raymath.h"
class Vector2f
{
public:
    static const Vector2f ZERO;
    static const Vector2f UP;
    static const Vector2f RIGHT;

    Vector2f(const Vector2& v) {
        m_data[0] = v.x;
        m_data[1] = v.y;
    }
    Vector2f(const Vector2f &other);
    Vector2f(float val = 0.0f);
    Vector2f(float x, float y);

    Vector2f &operator=(const Vector2f &other);

    Vector2f &operator+=(const Vector2f &other);
    Vector2f &operator-=(const Vector2f &other);
    Vector2f &operator*=(float scalar);
    Vector2f &operator/=(float scalar);

    const float &operator[](int i) const;
    float &operator[](int i);

    float &x();
    float &y();

    float x() const;
    float y() const;

    Vector2f xy() const;
    Vector2f yx() const;
    Vector2f xx() const;
    Vector2f yy() const;

    // returns (-y,x)
    Vector2f Normal() const;

    float Length() const;
    float LengthSquared() const;
    void Normalize();
    Vector2f Normalized() const;

    void Negate();

    static float Distance(const Vector2f &a, const Vector2f &b);
    static Vector2f Lerp(const Vector2f &a, const Vector2f &b, float t);

    operator Vector2() const{return {m_data[0], m_data[1]};}
private:
    float m_data[2];
};

// 逐分量
Vector2f operator+(const Vector2f &v0, const Vector2f &v1);
Vector2f operator-(const Vector2f &v0, const Vector2f &v1);
Vector2f operator&(const Vector2f &v0, const Vector2f &v1); // 逐分量乘法
Vector2f operator/(const Vector2f &v0, const Vector2f &v1); // 逐分量除法

float operator*(const Vector2f &v0, const Vector2f &v1); // 点乘
float operator^(const Vector2f &v0, const Vector2f &v1); // 叉乘

Vector2f operator-(const Vector2f &v);

Vector2f operator*(float f, const Vector2f &v);
Vector2f operator*(const Vector2f &v, float f);
Vector2f operator/(const Vector2f &v, float f);

bool operator==(const Vector2f &v0, const Vector2f &v1);
bool operator!=(const Vector2f &v0, const Vector2f &v1);
