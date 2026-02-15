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
    std::shared_ptr<ShaderWrapper> GetShader(const std::string &vsPath, const std::string &fsPath);
    std::shared_ptr<ShaderWrapper> GetTFBShader(const std::string &vsPath, const std::vector<std::string> &varyings);

    Sound GetSound(const std::string &path);
    Music GetMusic(const std::string &path);
    void UpdateMusic();

    void GameWorldUnloadAll();

    void UnloadAll();

private:
    std::unordered_map<std::string, Model> m_models;
    std::unordered_map<std::string, std::shared_ptr<ShaderWrapper>> m_shaders;

    std::unordered_map<std::string, Texture2D> m_textures;
    // 纹理帧数map
    std::unordered_map<unsigned int, int> m_textureFrameCounts;

    std::unordered_map<std::string, Sound> m_sounds;
    std::unordered_map<std::string, Music> m_musics;
};