#include "Complex.h"
#include "Engine/Math/Core/MathCommon.h"
#include <cmath>

Complex::Complex(float real, float imag)
    : real(real), imag(imag)
{
}

Complex Complex::operator+(const Complex &other) const
{
    return Complex(real + other.real, imag + other.imag);
}

Complex Complex::operator-(const Complex &other) const
{
    return Complex(real - other.real, imag - other.imag);
}

Complex Complex::operator*(const Complex &other) const
{
    return Complex(
        real * other.real - imag * other.imag,
        real * other.imag + imag * other.real);
}

Complex Complex::operator/(const Complex &other) const
{
    float denominator = other.Norm();
    if (denominator <= MATH_EPSILON)
    {
        return Complex::Zero();
    }
    Complex numerator = *this * other.Conjugate();
    return Complex(numerator.real / denominator, numerator.imag / denominator);
}

Complex &Complex::operator+=(const Complex &other)
{
    real += other.real;
    imag += other.imag;
    return *this;
}

Complex &Complex::operator-=(const Complex &other)
{
    real -= other.real;
    imag -= other.imag;
    return *this;
}

Complex &Complex::operator*=(const Complex &other)
{
    *this = *this * other;
    return *this;
}

Complex &Complex::operator/=(const Complex &other)
{
    *this = *this / other;
    return *this;
}

Complex Complex::operator*(float scalar) const
{
    return Complex(real * scalar, imag * scalar);
}

Complex Complex::operator/(float scalar) const
{
    return Complex(real / scalar, imag / scalar);
}

Complex &Complex::operator*=(float scalar)
{
    real *= scalar;
    imag *= scalar;
    return *this;
}

Complex &Complex::operator/=(float scalar)
{
    real /= scalar;
    imag /= scalar;
    return *this;
}

float Complex::Magnitude() const
{
    return std::sqrt(Norm());
}

float Complex::Abs() const
{
    return Magnitude();
}

float Complex::Norm() const
{
    return real * real + imag * imag;
}

float Complex::Argument() const
{
    return std::atan2(imag, real);
}

Complex Complex::Conjugate() const
{
    return Complex(real, -imag);
}

Complex Complex::Reciprocal() const
{
    float n = Norm();
    if (n <= MATH_EPSILON)
    {
        return Complex::Zero();
    }
    return Complex(real / n, -imag / n);
}

Complex Complex::FromPolar(float magnitude, float angle)
{
    return Complex(
        magnitude * std::cos(angle),
        magnitude * std::sin(angle));
}

Complex Complex::Zero()
{
    return Complex(0.0f, 0.0f);
}

Complex Complex::One()
{
    return Complex(1.0f, 0.0f);
}

Complex Complex::I()
{
    return Complex(0.0f, 1.0f);
}
