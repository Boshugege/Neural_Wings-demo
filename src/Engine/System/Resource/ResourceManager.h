#pragma once
#include "raylib.h"
#include "Engine/Graphics/ShaderWrapper.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
class ResourceManager
{
public:
    ResourceManager() = default;
    ~ResourceManager();

    Model GetModel(const std::string &path);
    Texture2D GetTexture2D(const std::string &path, int *outFrameCount = nullptr);
    TextureCubemap GetCubemap(const std::string &path);
    std::shared_ptr<ShaderWrapper> GetShader(const std::string &vsPath, const std::string &fsPath);
    std::shared_ptr<ShaderWrapper> GetTFBShader(const std::string &vsPath, const std::vector<std::string> &varyings);
    void UnloadAll();

private:
    TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format);

    std::unordered_map<std::string, Model> m_models;
    std::unordered_map<std::string, std::shared_ptr<ShaderWrapper>> m_shaders;

    std::unordered_map<std::string, Texture2D> m_textures;
    std::unordered_map<std::string, TextureCubemap> m_cubemaps;
    // 纹理帧数map
    std::unordered_map<unsigned int, int> m_textureFrameCounts;
};