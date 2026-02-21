#pragma once
#include "IComponent.h"
#include "Engine/Math/Math.h"

enum class LightType
{
    Directional,
    Point
};

struct LightComponent : public IComponent
{
    LightType type = LightType::Directional;
    Vector3f color = {255.0f, 255.0f, 255.0f};
    float intensity = 1.0f;
    bool castShadows = true; // 是否产生阴影
    float shadowBias = 0.005f;

    // directional light
    Vector3f direction = {0.5f, -1.0f, 0.5f};
    // point light
    float range = 10.0f;
    float attenuation = 1.0f; // 衰减因子

    int shadowIndex = -1;
};