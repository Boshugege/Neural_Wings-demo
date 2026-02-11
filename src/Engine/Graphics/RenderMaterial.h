#pragma once
#include "ShaderWrapper.h"
#include <memory>
#include "Engine/Math/Math.h"
#include "rlgl.h"
#include "Engine/System/Resource/ResourceManager.h"
#include "Engine/Utils/JsonParser.h"
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#define BLEND_OPIQUE -1
#define BLEND_MULTIPLIED -2
#define BLEND_SCREEN -3
#define BLEND_SUBTRACT -4

struct RenderMaterial
{
    std::string outputRT = "inScreen"; // 可以输出到RT供给postprocess使用

    std::shared_ptr<ShaderWrapper> shader;

    Vector4f baseColor = Vector4f(255.0, 255.0, 255.0, 255.0);

    bool diffuseIsAnimated = false;
    int diffuseframeCount = 1;
    float diffuseanimSpeed = 0.0;
    Texture2D diffuseMap = {0};
    bool useDiffuseMap = false;

    std::unordered_map<std::string, bool> isAnimated;
    std::unordered_map<std::string, int> frameCount;
    std::unordered_map<std::string, float> animSpeed;
    std::unordered_map<std::string, Texture2D> customTextures;

    int blendMode = BLEND_OPIQUE; // 覆盖颜色
    bool depthTest = true;
    bool depthWrite = false;

    int cullFace = -1; // 0 for front,1 for back

    std::unordered_map<std::string, float> customFloats;
    std::unordered_map<std::string, Vector2f> customVector2;
    std::unordered_map<std::string, Vector3f> customVector3;
    std::unordered_map<std::string, Vector4f> customVector4;

    void LoadFromConfig(const json &config, ResourceManager &rm)
    {

        depthWrite = config.value("depthWrite", true);
        depthTest = config.value("depthTest", true);
        if (config.contains("outputRT"))
        {
            outputRT = config["outputRT"];
        }
        if (config.contains("textures"))
        {
            auto &texData = config["textures"];
            for (auto &[texName, texInfo] : texData.items())
            {
                int frame = 1;
                Texture2D tex = rm.GetTexture2D(texInfo[0], &frame);
                if (tex.id > 0)
                {
                    if (texName == "u_diffuseMap")
                    {
                        if (frame > 1)
                        {
                            diffuseIsAnimated = true;
                            diffuseframeCount = frame;
                            diffuseanimSpeed = (float)texInfo[1];
                        }
                        else
                            diffuseIsAnimated = false;

                        diffuseMap = tex;
                        useDiffuseMap = true;
                    }
                    else
                    {
                        if (frame > 1)
                        {
                            isAnimated[texName] = true;
                            frameCount[texName] = frame;
                            animSpeed[texName] = (float)texInfo[1];
                        }
                        else
                            isAnimated[texName] = false;
                        customTextures[texName] = tex;
                    }
                }
            }
            // 若有贴图，使用对应shader或者默认贴图shader
            shader = rm.GetShader(config.value("vs", "assets/shaders/texture/default_texture.vs"), config.value("fs", "assets/shaders/texture/default_texture.fs"));
        }
        else if (config.contains("fs"))
            // 否则使用不带贴图的shader
            shader = rm.GetShader(config.value("vs", "assets/shaders/default.vs"), config["fs"]);

        if (config.contains("color"))
            baseColor = JsonParser::ToVector4f(config["color"]);
        if (config.contains("blendMode"))
        {
            std::string blend = config["blendMode"];
            if (blend == "ADDITIVE")
                blendMode = BlendMode::BLEND_ADDITIVE;
            else if (blend == "ALPHA")
                blendMode = BlendMode::BLEND_ALPHA;
            else if (blend == "NONE")
                blendMode = BLEND_OPIQUE;
            else if (blend == "MULTIPLY")
                blendMode = BLEND_MULTIPLIED;
            else if (blend == "SCREEN")
                blendMode = BLEND_SCREEN;
            else if (blend == "SUBTRACT")
                blendMode = BLEND_SUBTRACT;
            else
                std::cerr << "[RenderMaterial]: Unknown blend mode: " << blendMode << std::endl;
        }

        if (config.contains("cullFace"))
        {
            if (config["cullFace"] == "FRONT")
                cullFace = RL_CULL_FACE_FRONT;
            else if (config["cullFace"] == "BACK")
                cullFace = RL_CULL_FACE_BACK;
            else
                cullFace = -1;
        }

        if (config.contains("uniforms"))
        {
            auto &uniformsData = config["uniforms"];
            for (auto &[uName, uValue] : uniformsData.items())
            {
                if (uValue.is_number())
                    customFloats[uName] = uValue;
                else if (uValue.is_array() && uValue.size() == 2)
                    customVector2[uName] = JsonParser::ToVector2f(uValue);
                else if (uValue.is_array() && uValue.size() == 3)
                    customVector3[uName] = JsonParser::ToVector3f(uValue);
                else if (uValue.is_array() && uValue.size() == 4)
                    customVector4[uName] = JsonParser::ToVector4f(uValue);
                else
                    std::cerr << "[RenderMaterial]: Unknown uniform type: " << uName << std::endl;
            }
        }
    }
};
