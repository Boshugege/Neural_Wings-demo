#pragma once

struct Vector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vector3() = default;
    Vector3(float x, float y, float z);

    Vector3 operator+(const Vector3 &other) const;
    Vector3 operator-(const Vector3 &other) const;
    Vector3 operator*(float scalar) const;
    Vector3 operator/(float scalar) const;

    Vector3 &operator+=(const Vector3 &other);
    Vector3 &operator-=(const Vector3 &other);
    Vector3 &operator*=(float scalar);
    Vector3 &operator/=(float scalar);

    float Length() const;
    float LengthSquared() const;
    void Normalize();
    Vector3 Normalized() const;

    float Dot(const Vector3 &other) const;
    Vector3 Cross(const Vector3 &other) const;

    static float Distance(const Vector3 &a, const Vector3 &b);
    static Vector3 Lerp(const Vector3 &a, const Vector3 &b, float t);
};
