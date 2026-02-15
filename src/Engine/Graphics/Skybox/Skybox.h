#pragma once
#include "raylib.h"
#include "Engine/Graphics/ShaderWrapper.h"
#include "Engine/System/Resource/ResourceManager.h"
#include <memory>

class ResourceManager;
class Skybox
{
public:
    void Load(const std::string &texturePath, ResourceManager &rm);
    void Draw(const Camera3D &camera, float aspect);
    void Unload();
    void SetTintColor(Vector4f color) { m_tintColor = color; }

    TextureCubemap &GetTexture() { return m_texture; }

private:
    Model m_cubeModel = {0};
    std::shared_ptr<ShaderWrapper> m_shader;
    TextureCubemap m_texture = {0};
    Vector4f m_tintColor = {1.0f, 1.0f, 1.0f, 1.0f};
};