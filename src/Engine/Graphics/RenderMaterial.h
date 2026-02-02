#pragma once
#include "ShaderWrapper.h"
#include <memory>
#include "Engine/Math/Math.h"
#include <unordered_map>
#include <string>

#define BLEND_OPIQUE -1
#define BLEND_MULTIPLIED -2
#define BLEND_SCREEN -3
#define BLEND_SUBTRACT -4

struct RenderMaterial
{
    std::shared_ptr<ShaderWrapper> shader;

    Vector4f baseColor = Vector4f(255.0, 255.0, 255.0, 255.0);
    Texture2D diffuseMap = {0};
    bool useDiffuseMap = false;
    std::unordered_map<std::string, Texture2D> customTextures;

    int blendMode = BLEND_OPIQUE; // 覆盖颜色
    bool depthTest = true;
    bool depthWrite = false;

    int cullFace = -1; // 0 for front,1 for back

    std::unordered_map<std::string, float> customFloats;
    std::unordered_map<std::string, Vector2f> customVector2;
    std::unordered_map<std::string, Vector3f> customVector3;
    std::unordered_map<std::string, Vector4f> customVector4;
};

struct PostRenderMaterial
{
    std::shared_ptr<ShaderWrapper> shader;

    Vector4f baseColor = Vector4f(255.0, 255.0, 255.0, 255.0);

    int blendMode = BLEND_OPIQUE; // 覆盖颜色

    std::unordered_map<std::string, Texture2D> customTextures;

    std::unordered_map<std::string, float> customFloats;
    std::unordered_map<std::string, Vector2f> customVector2;
    std::unordered_map<std::string, Vector3f> customVector3;
    std::unordered_map<std::string, Vector4f> customVector4;
};