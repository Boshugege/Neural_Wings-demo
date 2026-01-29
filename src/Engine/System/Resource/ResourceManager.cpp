#include "ResourceManager.h"
#include <iostream>

ResourceManager::~ResourceManager()
{
    UnloadAll();
}

Model ResourceManager::GetModel(const std::string &path)
{

    auto it = m_models.find(path);
    if (it != m_models.end())
        return it->second;

    Model model = {0};
    if (path.find("primitive://") == 0)
    {
        if (path.find("primitive://cube") != std::string::npos)
        {
            model = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
        }
        else if (path.find("primitive://sphere") != std::string::npos)
        {
            model = LoadModelFromMesh(GenMeshSphere(1.0f, 32, 32));
        }
    }
    else
    {
        model = LoadModel(path.c_str());
    }

    if (model.meshCount > 0)
    {
        m_models[path] = model;
        std::cout << "[ResourceManager] Loaded model: " << path << std::endl;
    }
    else
        std::cerr << "[ResourceManager] Failed to load model: " << path << std::endl;
    return model;
}
Texture2D ResourceManager::GetTexture(const std::string &path)
{
    auto it = m_textures.find(path);
    if (it != m_textures.end())
        return it->second;
    Texture2D textures = LoadTexture(path.c_str());

    if (textures.id != 0)
    {
        m_textures[path] = textures;
        std::cout << "[ResourceManager] Loaded model: " << path << std::endl;
    }
    else
        std::cerr << "[ResourceManager] Failed to load model: " << path << std::endl;
    return textures;
}
void ResourceManager::UnloadAll()
{
    for (auto &pair : m_models)
    {
        UnloadModel(pair.second);
    }
    m_models.clear();
    for (auto &pair : m_textures)
    {
        UnloadTexture(pair.second);
    }
    m_textures.clear();
    std::cout << "[ResourceManager] Unloaded all resources" << std::endl;
}