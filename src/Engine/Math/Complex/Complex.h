#pragma once

struct Complex {
    float real = 0.0f;
    float imag = 0.0f;

    Complex() = default;
    Complex(float real, float imag);

    Complex operator+(const Complex& other) const;
    Complex operator-(const Complex& other) const;
    Complex operator*(const Complex& other) const;
    Complex operator/(const Complex& other) const;

    Complex& operator+=(const Complex& other);
    Complex& operator-=(const Complex& other);
    Complex& operator*=(const Complex& other);
    Complex& operator/=(const Complex& other);

    Complex operator*(float scalar) const;
    Complex operator/(float scalar) const;

    Complex& operator*=(float scalar);
    Complex& operator/=(float scalar);

    float Magnitude() const;
    float Abs() const;
    float Norm() const;
    float Argument() const;

    Complex Conjugate() const;
    Complex Reciprocal() const;

    static Complex FromPolar(float magnitude, float angle);
    static Complex Zero();
    static Complex One();
    static Complex I();
};
