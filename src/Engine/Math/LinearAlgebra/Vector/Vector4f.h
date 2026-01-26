#pragma once
#include "raylib.h"
#include "raymath.h"
class Vector2f;
class Vector3f;
class Vector4f
{
public:
	Vector4f(const Vector4 &v)
	{
		m_data[0] = v.x;
		m_data[1] = v.y;
		m_data[2] = v.z;
		m_data[3] = v.w;
	}
	Vector4f(float f = 0.f);
	Vector4f(float fx, float fy, float fz, float fw);
	Vector4f(float f[4]);

	Vector4f(const Vector2f &xy, float z, float w);
	Vector4f(float x, const Vector2f &yz, float w);
	Vector4f(float x, float y, const Vector2f &zw);
	Vector4f(const Vector2f &xy, const Vector2f &zw);

	Vector4f(const Vector3f &xyz, float w);
	Vector4f(float x, const Vector3f &yzw);

	Vector4f(const Vector4f &other);

	Vector4f &operator=(const Vector4f &rv);
	const float &operator[](int i) const;
	float &operator[](int i);

	float &x();
	float &y();
	float &z();
	float &w();

	float x() const;
	float y() const;
	float z() const;
	float w() const;

	Vector2f xy() const;
	Vector2f yz() const;
	Vector2f zw() const;
	Vector2f wx() const;

	Vector3f xyz() const;
	Vector3f yzw() const;
	Vector3f zwx() const;
	Vector3f wxy() const;

	Vector3f xyw() const;
	Vector3f yzx() const;
	Vector3f zwy() const;
	Vector3f wxz() const;

	Vector4f &operator+=(const Vector4f &other);
	Vector4f &operator-=(const Vector4f &other);
	Vector4f &operator*=(float scalar);
	Vector4f &operator/=(float scalar);

	float Length() const;
	float LengthSquared() const;
	void Normalize();
	Vector4f Normalized() const;

	void Homogenize();
	Vector4f Homogenized() const;

	void Negate();

	static float Distance(const Vector4f &a, const Vector4f &b);
	static Vector4f Lerp(const Vector4f &a, const Vector4f &b, float t);

	operator Vector4() const { return {m_data[0], m_data[1], m_data[2], m_data[3]}; }

private:
	float m_data[4];
};

Vector4f operator+(const Vector4f &v0, const Vector4f &v1);
Vector4f operator-(const Vector4f &v0, const Vector4f &v1);
Vector4f operator&(const Vector4f &v0, const Vector4f &v1);
Vector4f operator/(const Vector4f &v0, const Vector4f &v1);

Vector4f operator-(const Vector4f &v);

float operator*(const Vector4f &v0, const Vector4f &v1); // 点乘

Vector4f operator*(float f, const Vector4f &v);
Vector4f operator*(const Vector4f &v, float f);
Vector4f operator/(const Vector4f &v, float f);

bool operator==(const Vector4f &v0, const Vector4f &v1);
bool operator!=(const Vector4f &v0, const Vector4f &v1);
