#include "ResourceManager.h"
#include <iostream>

ResourceManager::~ResourceManager()
{
    UnloadAll();
}

std::shared_ptr<ShaderWrapper> ResourceManager::GetShader(const std::string &vsPath, const std::string &fsPath)
{
    std::string key = vsPath + fsPath;
    auto it = m_shaders.find(key);
    if (it != m_shaders.end())
        return it->second;
    auto shader = std::make_shared<ShaderWrapper>(vsPath, fsPath);
    m_shaders[key] = shader;
    return shader;
}

std::shared_ptr<ShaderWrapper> ResourceManager::GetTFBShader(const std::string &vsPath, const std::vector<std::string> &varyings)
{
    std::string key = vsPath + "_tfb_";
    for (const auto &varying : varyings)
        key += varying;
    if (m_shaders.count(key))
        return m_shaders[key];
    auto shader = std::make_shared<ShaderWrapper>(vsPath, varyings);
    m_shaders[key] = shader;
    return shader;
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
Texture2D ResourceManager::GetTexture2D(const std::string &path)
{
    auto it = m_textures.find(path);
    if (it != m_textures.end())
        return it->second;
    Texture2D textures = LoadTexture(path.c_str());

    if (textures.id != 0)
    {
        m_textures[path] = textures;
        std::cout << "[ResourceManager] Loaded textures " << path << std::endl;
    }
    else
        std::cerr << "[ResourceManager] Failed to load texture: " << path << std::endl;
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
    m_shaders.clear();
    std::cout << "[ResourceManager] Unloaded all resources" << std::endl;
}