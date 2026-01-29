#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "Engine/Math/Math.h"

using json = nlohmann::json;

class JsonParser
{
public:
    static Vector3f ToVector3f(const json &j, Vector3f defaultValue = Vector3f::ZERO)
    {
        if (j.is_array() && j.size() == 3)
        {
            return Vector3f(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
        }
        return defaultValue;
    }
    static Color ToColor(const json &j, Color defaultValue = WHITE)
    {
        if (j.is_array() && j.size() == 4)
        {
            return Color{(unsigned char)j[0], (unsigned char)j[1], (unsigned char)j[2], (unsigned char)j[3]};
        }
        return defaultValue;
    }
    static Rectangle ToRectangle(const json &j)
    {
        if (j.is_array() && j.size() == 4)
        {
            float sw = (float)GetScreenWidth();
            float sh = (float)GetScreenHeight();
            float x = j[0].get<float>();
            float y = j[1].get<float>();
            float w = j[2].get<float>();
            float h = j[3].get<float>();
            return Rectangle{x <= 1 ? x * sw : x,
                             y <= 1 ? y * sh : y,
                             w <= 1 ? w * sw : w,
                             h <= 1 ? h * sh : h};
        }
    }
};