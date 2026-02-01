#pragma once
#include "ShaderWrapper.h"
#include <memory>
#include "Engine/Math/Math.h"
#include <unordered_map>
#include <string>
struct RenderMaterial
{
    std::shared_ptr<ShaderWrapper> shader;

    Vector4f baseColor = Vector4f(255.0, 255.0, 255.0, 255.0);

    int blendMode = -1; // 覆盖颜色
    bool depthTest = true;
    bool depthWrite = false;

    int cullFace = -1; // 0 for front,1 for back

    std::unordered_map<std::string, float> customFloats;
    std::unordered_map<std::string, Vector3f> customVector3;
    std::unordered_map<std::string, Vector4f> customVector4;
};