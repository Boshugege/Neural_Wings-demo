#include "ResourceManager.h"
#include <iostream>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace
{
    bool TryLoadPreprocessedGifAtlas(const std::string &gifPath, Texture2D &outTexture, int &outFrameCount)
    {
        namespace fs = std::filesystem;
        fs::path basePath = fs::path(gifPath).replace_extension("");
        fs::path atlasPath = basePath.string() + ".atlas.png";
        fs::path metaPath = basePath.string() + ".atlas.json";

        if (!fs::exists(atlasPath) || !fs::exists(metaPath))
            return false;

        std::ifstream metaFile(metaPath);
        if (!metaFile.is_open())
            return false;

        json meta;
        try
        {
            metaFile >> meta;
        }
        catch (...)
        {
            return false;
        }

        if (!meta.contains("frameCount") || !meta["frameCount"].is_number_integer())
            return false;

        outFrameCount = meta["frameCount"].get<int>();
        if (outFrameCount <= 0)
            return false;

        Texture2D tex = LoadTexture(atlasPath.string().c_str());
        if (tex.id == 0)
            return false;

        outTexture = tex;
        return true;
    }

    bool IsGifPath(const std::string &path)
    {
        namespace fs = std::filesystem;
        std::string ext = fs::path(path).extension().string();
        for (char &c : ext)
            c = (char)tolower(c);
        return ext == ".gif";
    }
} // namespace

ResourceManager::~ResourceManager()
{
    UnloadAll();
}

Sound ResourceManager::GetSound(const std::string &path)
{
    auto it = m_sounds.find(path);
    if (it != m_sounds.end())
        return it->second;

    Sound s = LoadSound(path.c_str());
    if (s.frameCount > 0)
    {
        m_sounds[path] = s;
        std::cout << "[ResourceManager]: Sound loaded: " << path << std::endl;
    }
    else
    {
        std::cerr << "[ResourceManager]: Failed to load sound: " << path << std::endl;
    }
    return s;
}
Music ResourceManager::GetMusic(const std::string &path)
{
    auto it = m_musics.find(path);
    if (it != m_musics.end())
        return it->second;

    Music m = LoadMusicStream(path.c_str());
    if (m.frameCount > 0)
    {
        m_musics[path] = m;
        std::cout << "[ResourceManager]: Music stream loaded: " << path << std::endl;
    }
    else
    {
        std::cerr << "[ResourceManager]: Failed to load music: " << path << std::endl;
    }
    return m;
}
void ResourceManager::UpdateMusic()
{
    for (auto &pair : m_musics)
    {
        if (IsMusicStreamPlaying(pair.second))
        {
            UpdateMusicStream(pair.second);
        }
    }
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
Texture2D ResourceManager::GetTexture2D(const std::string &path, int *outFrameCount)
{
    auto it = m_textures.find(path);
    if (it != m_textures.end())
    {
        if (outFrameCount)
            *outFrameCount = m_textureFrameCounts[it->second.id];
        return it->second;
    }
    // gif贴图
    if (IsGifPath(path))
    {
        Texture2D preprocessed = {0};
        int frameCount = 0;
        if (TryLoadPreprocessedGifAtlas(path, preprocessed, frameCount))
        {
            m_textures[path] = preprocessed;
            m_textureFrameCounts[preprocessed.id] = frameCount;
            if (outFrameCount)
                *outFrameCount = frameCount;
            std::cout << "[ResourceManager]: Loaded preprocessed GIF atlas: " << path << " (" << frameCount << " frames)" << std::endl;
            return preprocessed;
        }

        std::cerr << "[ResourceManager] Missing preprocessed GIF atlas for: " << path << " (expect .atlas.png + .atlas.json)" << std::endl;
        return Texture2D{0};
    }
    // 静态贴图
    Texture2D textures = LoadTexture(path.c_str());

    if (textures.id != 0)
    {
        m_textures[path] = textures;
        m_textureFrameCounts[textures.id] = 1;
        if (outFrameCount)
            *outFrameCount = 1;
        std::cout << "[ResourceManager] Loaded textures " << path << std::endl;
    }
    else
        std::cerr << "[ResourceManager] Failed to load texture: " << path << std::endl;
    return textures;
}
void ResourceManager::GameWorldUnloadAll()
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

    for (auto &pair : m_musics)
    {

        StopMusicStream(pair.second);
        UnloadMusicStream(pair.second);
    }
    m_musics.clear();

    for (auto &pair : m_sounds)
    {
        UnloadSound(pair.second);
    }
    m_sounds.clear();

    m_shaders.clear();
    std::cout << "[ResourceManager] Unloaded all resources" << std::endl;
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
    for (auto &pair : m_musics)
    {
        StopMusicStream(pair.second);
        UnloadMusicStream(pair.second);
    }
    m_musics.clear();
    for (auto &pair : m_sounds)
    {
        UnloadSound(pair.second);
    }
    m_sounds.clear();

    m_shaders.clear();
    std::cout << "[ResourceManager] Unloaded all resources" << std::endl;
}
