#include "ResourceManager.h"
#include <iostream>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "raylib.h"

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

#include "rlgl.h"
#include "raymath.h"
#if defined(PLATFORM_WEB)
#include <GLES3/gl3.h>
#else
#include "external/glad.h"
#endif

TextureCubemap ResourceManager::GetCubemap(const std::string &path)
{
    if (m_cubemaps.count(path))
        return m_cubemaps[path];
    Image img = LoadImage(path.c_str());
    if (img.data == nullptr)
    {
        std::cerr << "[ResourceManager] Failed to load image: " << path << std::endl;
        return {0};
    }
    TextureCubemap cubemap = {0};

    // TextureCubemap cubemap = LoadTextureCubemap(img, CUBEMAP_LAYOUT_AUTO_DETECT);
    bool isPanorama = (img.width / 2 == img.height);
    if (isPanorama)
    {
        Shader shdr = LoadShader("assets/shaders/utils/cubemap.vs", "assets/shaders/utils/cubemap.fs");
        int mapLoc = GetShaderLocation(shdr, "equirectangularMap");
        int value = 0;
        SetShaderValue(shdr, mapLoc, &value, SHADER_UNIFORM_INT);
        Texture2D panorama = LoadTextureFromImage(img);

        cubemap = GenTextureCubemap(shdr, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R32G32B32);

        UnloadTexture(panorama);
        UnloadShader(shdr);
        std::cout << "[ResourceManager] Generated Cubemap from Panorama: " << path << std::endl;
    }
    else
    {
        cubemap = LoadTextureCubemap(img, CUBEMAP_LAYOUT_AUTO_DETECT);
    }
    UnloadImage(img);
    if (cubemap.id > 0)
    {
        SetTextureFilter(cubemap, TEXTURE_FILTER_BILINEAR);

        m_cubemaps[path] = cubemap;
        std::cout << "[ResourceManager] Loaded Cubemap: " << path << std::endl;
    }
    else
    {
        std::cerr << "[ResourceManager] Failed to load Cubemap: " << path << std::endl;
    }
    return cubemap;
}
TextureCubemap ResourceManager::GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format)
{
    TextureCubemap cubemap = {0};
    rlDisableDepthTest();
    rlDisableBackfaceCulling();

    unsigned int rId = rlLoadTextureCubemap(nullptr, size, format, 1);
    cubemap.id = rId;
    cubemap.width = size;
    cubemap.height = size;
    cubemap.mipmaps = 1;
    cubemap.format = format;

    unsigned int fbo = rlLoadFramebuffer();
    rlEnableFramebuffer(fbo);
    rlViewport(0, 0, size, size);

    rlEnableShader(shader.id);
    int matViewLoc = rlGetLocationUniform(shader.id, "matView");
    int matProjLoc = rlGetLocationUniform(shader.id, "matProjection");
    Matrix matProj = MatrixPerspective(90.0f * DEG2RAD, 1.0f, 0.01f, 1000.0f);
    rlSetUniformMatrix(matProjLoc, matProj);

    rlActiveTextureSlot(0);
    rlEnableTexture(panorama.id);

    Matrix fboViews[6] = {
        // Target, Up
        MatrixLookAt({0, 0, 0}, {1, 0, 0}, {0, -1, 0}),  // Right
        MatrixLookAt({0, 0, 0}, {-1, 0, 0}, {0, -1, 0}), // Left
        MatrixLookAt({0, 0, 0}, {0, 1, 0}, {0, 0, 1}),   // Top
        MatrixLookAt({0, 0, 0}, {0, -1, 0}, {0, 0, -1}), // Bottom
        MatrixLookAt({0, 0, 0}, {0, 0, 1}, {0, -1, 0}),  // Front
        MatrixLookAt({0, 0, 0}, {0, 0, -1}, {0, -1, 0})  // Back
    };

    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    UploadMesh(&cube, false);

    for (int i = 0; i < 6; i++)
    {
        rlSetUniformMatrix(matViewLoc, fboViews[i]);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap.id, 0);

        rlClearColor(0, 0, 0, 0);
        rlClearScreenBuffers();

        rlEnableVertexArray(cube.vaoId);
        rlDrawVertexArrayElements(0, cube.triangleCount * 3, 0);
        rlDisableVertexArray();
    }

    rlDisableShader();
    rlDisableTexture();
    rlDisableFramebuffer();
    rlUnloadFramebuffer(fbo);

    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
    UnloadMesh(cube);

    return cubemap;
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

    for (auto &pair : m_cubemaps)
        UnloadTexture(pair.second);
    m_cubemaps.clear();

    m_shaders.clear();
    std::cout << "[ResourceManager] Unloaded all resources" << std::endl;
}
