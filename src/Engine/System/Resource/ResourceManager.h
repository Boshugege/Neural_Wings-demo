#pragma once
#include "raylib.h"
#include <unordered_map>
#include <string>
class ResourceManager
{
public:
    ResourceManager() = default;
    ~ResourceManager();

    Model GetModel(const std::string &path);
    Texture2D GetTexture(const std::string &path);
    void UnloadAll();

private:
    std::unordered_map<std::string, Model> m_models;
    std::unordered_map<std::string, Texture2D> m_textures;
};