#pragma once
#include <cmath>

constexpr float MATH_PI = 3.14159265358979323846f;
constexpr float MATH_EPSILON = 1e-6f;

inline float Deg2Rad(float degrees)
{
    return degrees * (MATH_PI / 180.0f);
}

inline float Rad2Deg(float radians)
{
    return radians * (180.0f / MATH_PI);
}

inline bool NearlyEqual(float a, float b, float epsilon = MATH_EPSILON)
{
    return std::fabs(a - b) <= epsilon;
}

inline float Clamp(float value, float minValue, float maxValue)
{
    return value < minValue ? minValue : (value > maxValue ? maxValue : value);
}
