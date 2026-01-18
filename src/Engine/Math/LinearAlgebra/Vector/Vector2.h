#pragma once

struct Vector2
{
    float x = 0.0f;
    float y = 0.0f;

    Vector2() = default;
    Vector2(float x, float y);

    Vector2 operator+(const Vector2 &other) const;
    Vector2 operator-(const Vector2 &other) const;
    Vector2 operator*(float scalar) const;
    Vector2 operator/(float scalar) const;

    Vector2 &operator+=(const Vector2 &other);
    Vector2 &operator-=(const Vector2 &other);
    Vector2 &operator*=(float scalar);
    Vector2 &operator/=(float scalar);

    float Length() const;
    float LengthSquared() const;
    void Normalize();
    Vector2 Normalized() const;

    float Dot(const Vector2 &other) const;

    static float Distance(const Vector2 &a, const Vector2 &b);
    static Vector2 Lerp(const Vector2 &a, const Vector2 &b, float t);
};
