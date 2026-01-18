#pragma once

struct Vector4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Vector4() = default;
    Vector4(float x, float y, float z, float w);

    Vector4 operator+(const Vector4 &other) const;
    Vector4 operator-(const Vector4 &other) const;
    Vector4 operator*(float scalar) const;
    Vector4 operator/(float scalar) const;

    Vector4 &operator+=(const Vector4 &other);
    Vector4 &operator-=(const Vector4 &other);
    Vector4 &operator*=(float scalar);
    Vector4 &operator/=(float scalar);

    float Length() const;
    float LengthSquared() const;
    void Normalize();
    Vector4 Normalized() const;

    float Dot(const Vector4 &other) const;

    static float Distance(const Vector4 &a, const Vector4 &b);
    static Vector4 Lerp(const Vector4 &a, const Vector4 &b, float t);
};
