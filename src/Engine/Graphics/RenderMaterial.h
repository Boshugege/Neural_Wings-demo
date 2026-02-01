#pragma once
#include "ShaderWrapper.h"
#include <memory>
#include "Engine/Math/Math.h"
struct RenderMaterial
{
    std::shared_ptr<ShaderWrapper> shader;

    Vector4f baseColor = Vector4f(255.0, 255.0, 255.0, 255.0);

    int blendMode = 1;

    // TODO:动态存储
    // std::unordered_map<std::string,float> customFloats;
};