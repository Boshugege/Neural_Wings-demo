#include "Skybox.h"
#include "rlgl.h"
#include "Engine/System/Resource/ResourceManager.h"

void Skybox::Load(const std::string &texturePath, ResourceManager &rm)
{
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    m_cubeModel = LoadModelFromMesh(cube);
    m_shader = rm.GetShader("assets/shaders/skybox/skybox.vs", "assets/shaders/skybox/skybox.fs");
    m_texture = rm.GetCubemap(texturePath);
}
void Skybox::Draw(const Camera3D &camera, float aspect)
{
    if (!m_shader || !m_shader->IsValid())
        return;
    rlDisableDepthMask();
    rlDisableBackfaceCulling();

    m_shader->Begin();

    Matrix matView = GetCameraMatrix(camera);
    Matrix matProj = MatrixPerspective(camera.fovy * DEG2RAD, aspect,
                                       0.01, 1000.0);

    m_shader->SetMat4("matView", matView);
    m_shader->SetMat4("matProj", matProj);

    m_shader->SetVec4("tintColor", m_tintColor);

    int texUnit = 0;
    m_shader->SetCubeMap("skyboxMap", m_texture, texUnit);
    // int loc = m_shader->GetLocation("skyboxMap");
    // if (loc >= 0)
    // {
    //     int texUnit = 0;
    //     rlActiveTextureSlot(texUnit);
    //     rlEnableTextureCubemap(m_texture.id);
    //     SetShaderValue(m_shader->GetShader(), loc, &texUnit, SHADER_UNIFORM_INT);
    // }

    Material tempRaylibMaterial = m_cubeModel.materials[0];
    tempRaylibMaterial.shader = m_shader->GetShader();

    m_cubeModel.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = m_texture;
    DrawMesh(m_cubeModel.meshes[0], tempRaylibMaterial, MatrixScale(100.0f, 100.0f, 100.0f));
    rlDisableTextureCubemap();

    m_shader->End();
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}
void Skybox::Unload()
{
    if (m_cubeModel.meshCount > 0)
        UnloadModel(m_cubeModel);
}