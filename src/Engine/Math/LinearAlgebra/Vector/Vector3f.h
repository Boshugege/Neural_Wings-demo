#pragma once
#include "raylib.h"
#include "raymath.h"
#include <nlohmann/json.hpp>
#include <iostream>
class Vector2f;

using json = nlohmann::json;
class Vector3f
{
public:
    static const Vector3f ZERO;
    static const Vector3f UP;
    static const Vector3f RIGHT;
    static const Vector3f FORWARD;

    Vector3f(const Vector3 &v)
    {
        m_data[0] = v.x;
        m_data[1] = v.y;
        m_data[2] = v.z;
    }
    Vector3f(const Vector3f &other);
    Vector3f(float val = 0.0f);
    Vector3f(float x, float y, float z);
    Vector3f(const json &arr)
    {
        if (arr.size() >= 3)
        {
            m_data[0] = arr[0];
            m_data[1] = arr[1];
            m_data[2] = arr[2];
        }
    }

    Vector3f &operator=(const Vector3f &other);

    Vector3f &operator+=(const Vector3f &other);
    Vector3f &operator-=(const Vector3f &other);
    Vector3f &operator*=(float scalar);
    Vector3f &operator/=(float scalar);

    const float &operator[](int i) const;
    float &operator[](int i);

    float &x();
    float &y();
    float &z();

    float x() const;
    float y() const;
    float z() const;

    Vector2f xy() const;
    Vector2f xz() const;
    Vector2f yz() const;

    Vector3f xyz() const;
    Vector3f yzx() const;
    Vector3f zxy() const;

    float Length() const;
    float LengthSquared() const;
    void Normalize();
    Vector3f Normalized() const;

    Vector2f Homogenized() const;

    void Negate();
    Vector3f RotateByAxixAngle(const Vector3f &axis, float angle);

    static float Distance(const Vector3f &a, const Vector3f &b);
    static Vector3f Lerp(const Vector3f &a, const Vector3f &b, float t);

    static Vector3f Min(const Vector3f &a, const Vector3f &b);
    static Vector3f Max(const Vector3f &a, const Vector3f &b);
    void print() const { std::cout << m_data[0] << ", " << m_data[1] << ", " << m_data[2] << std::endl; };
    operator Vector3() const { return {m_data[0], m_data[1], m_data[2]}; }

private:
    float m_data[3];
};

// 逐分量
Vector3f operator+(const Vector3f &v0, const Vector3f &v1);
Vector3f operator-(const Vector3f &v0, const Vector3f &v1);
Vector3f operator&(const Vector3f &v0, const Vector3f &v1); // 逐分量乘法
Vector3f operator/(const Vector3f &v0, const Vector3f &v1); // 逐分量除法

float operator*(const Vector3f &v0, const Vector3f &v1);    // 点乘
Vector3f operator^(const Vector3f &v0, const Vector3f &v1); // 叉乘

Vector3f operator-(const Vector3f &v);

Vector3f operator*(float f, const Vector3f &v);
Vector3f operator*(const Vector3f &v, float f);
Vector3f operator/(const Vector3f &v, float f);

bool operator==(const Vector3f &v0, const Vector3f &v1);
bool operator!=(const Vector3f &v0, const Vector3f &v1);
