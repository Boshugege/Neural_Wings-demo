#include "LightingManager.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Graphics/Renderer.h"

#include "rlgl.h"

#if defined(PLATFORM_WEB)
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#else
#include "external/glad.h"
#endif

LightingManager::~LightingManager()
{
    for (auto &rt : m_shadowMaps)
    {
        if (rt.id > 0)
            UnloadRenderTexture(rt);
    }
    m_shadowMaps.clear();

    for (auto &psm : m_pointShadowMaps)
    {
        if (psm.fboId > 0)
        {
            glDeleteFramebuffers(1, &psm.fboId);
            psm.fboId = 0;
        }
        if (psm.cubemapId > 0)
        {
            glDeleteTextures(1, &psm.cubemapId);
            psm.cubemapId = 0;
        }
    }
    m_pointShadowMaps.clear();
}
void LightingManager::Update(GameWorld &world)
{
    m_activeLights.clear();
    m_activeCasters.clear();
    m_activePointCasters.clear();

    auto entities = world.GetEntitiesWith<LightComponent, TransformComponent>();

    int shadowCount = 0;
    int pointShadowCount = 0;
    for (auto *entity : entities)
    {
        if (m_activeLights.size() >= MAX_LIGHTS)
            break;

        auto &light = entity->GetComponent<LightComponent>();
        auto &tf = entity->GetComponent<TransformComponent>();

        LightInfo info;
        info.data = &light;
        info.worldPosition = tf.GetWorldPosition();
        info.shadowIndex = -1;

        if (light.type == LightType::Directional)
            info.worldDirection = tf.GetWorldRotation() * (light.direction).Normalized();
        else
            info.worldDirection = light.direction;

        if (light.castShadows)
        {
            if (light.type == LightType::Directional && shadowCount < MAX_SHADOW_CASTERS)
            {

                ShadowCasterData caster;
                caster.textureIndex = shadowCount;
                caster.light = &light;
                caster.lightDir = info.worldDirection;
                caster.owner = entity;

                m_activeCasters.push_back(caster);

                info.shadowIndex = shadowCount;

                shadowCount++;
            }
            else if (light.type == LightType::Point && pointShadowCount < MAX_POINT_SHADOWS)
            {
                ShadowCasterData caster;
                caster.textureIndex = pointShadowCount;
                caster.light = &light;

                caster.owner = entity;
                m_activePointCasters.push_back(caster);
                info.shadowIndex = pointShadowCount;

                pointShadowCount++;
            }
        }
        light.shadowIndex = info.shadowIndex;
        m_activeLights.push_back(info);
    }
}
void LightingManager::UploadToShader(std::shared_ptr<ShaderWrapper> shader, const Vector3f &viewPos, int texUnit)
{
    if (!shader || !shader->IsValid())
        return;
    shader->SetInt("lightCounts", (int)m_activeLights.size());
    shader->SetVec3("viewPos", viewPos);

    for (int i = 0; i < m_activeLights.size(); ++i)
    {
        std::string base = "lights[" + std::to_string(i) + "]";
        const auto &info = m_activeLights[i];

        shader->SetInt(base + ".type", (int)info.data->type);
        shader->SetVec3(base + ".position", info.worldPosition);
        shader->SetVec3(base + ".direction", info.worldDirection);
        shader->SetVec3(base + ".color", info.data->color / 255.0f);
        shader->SetFloat(base + ".intensity", info.data->intensity);
        shader->SetFloat(base + ".range", info.data->range);
        shader->SetInt(base + ".shadowIndex", info.shadowIndex);
        if (info.shadowIndex >= 0)
        {
            shader->SetFloat(base + ".shadowBias", info.data->shadowBias);
        }
    }

    // 避开材质贴图
    int shadowUnitBase = texUnit + 1;
    for (int i = 0; i < m_activeCasters.size(); ++i)
    {
        std::string baseVP = "lightVPs[" + std::to_string(i) + "]";
        std::string baseMap = "shadowMaps[" + std::to_string(i) + "]";

        shader->SetMat4(baseVP, m_activeCasters[i].lightVP);
        shader->SetTexture(baseMap, m_shadowMaps[m_activeCasters[i].textureIndex].depth, shadowUnitBase + i);
    }

    int pointShadowUnitBase = shadowUnitBase + m_activeCasters.size();
    for (int i = 0; i < m_pointShadowMaps.size(); ++i)
    {
        std::string baseMap = "pointShadowMaps[" + std::to_string(i) + "]";
        shader->SetCubeMap(baseMap, m_pointShadowMaps[i].cubemapId, pointShadowUnitBase + i);
    }
}

void LightingManager::InitShadowMaps(int width, int height, ResourceManager &rm)
{
    m_shadowMaps.resize(MAX_SHADOW_CASTERS);
    for (int i = 0; i < MAX_SHADOW_CASTERS; ++i)
    {
        m_shadowMaps[i] = Renderer::LoadRT(width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    }
    m_depthShader = rm.GetShader("assets/shaders/lighting/depth.vs", "assets/shaders/lighting/depth.fs");

    m_pointShadowMaps.resize(MAX_POINT_SHADOWS);
    for (int i = 0; i < MAX_POINT_SHADOWS; ++i)
    {
        PointShadowMap &psm = m_pointShadowMaps[i];
        psm.resolution = 1024;

        glGenFramebuffers(1, &psm.fboId);
        glGenTextures(1, &psm.cubemapId);

        glBindTexture(GL_TEXTURE_CUBE_MAP, psm.cubemapId);

        for (unsigned int face = 0; face < 6; ++face)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT,
                         psm.resolution, psm.resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, psm.fboId);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, psm.cubemapId, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    m_pointDepthShader = rm.GetShader("assets/shaders/lighting/point_depth.vs", "assets/shaders/lighting/point_depth.fs");
}

void LightingManager::RenderShadowMaps(GameWorld &world, const Vector3f &centerPos)
{
    if (!m_depthShader)
        return;

    rlEnableBackfaceCulling();
    rlSetCullFace(RL_CULL_FACE_FRONT);
    rlEnableDepthTest();
    rlEnableDepthMask();

    auto renderables = world.GetEntitiesWith<RenderComponent, TransformComponent>();

    for (auto &caster : m_activeCasters)
    {
        caster.lightVP = CalculateDirectionalLightVP(caster.lightDir, centerPos);

        BeginTextureMode(m_shadowMaps[caster.textureIndex]);
        ClearBackground(WHITE);

        m_depthShader->Begin();
        m_depthShader->SetMat4("lightVP", caster.lightVP);
        for (auto *obj : renderables)
        {

            auto &render = obj->GetComponent<RenderComponent>();
            auto &tf = obj->GetComponent<TransformComponent>();
            if (!render.castShadows)
                continue;
            Matrix4f modelMat = tf.GetWorldMatrix();
            m_depthShader->SetMat4("model", modelMat);

            for (int i = 0; i < render.model.meshCount; i++)
            {

                Material tempRaylibMaterial = render.model.materials[i];

                tempRaylibMaterial.shader = m_depthShader->GetShader();

                DrawMesh(render.model.meshes[i], tempRaylibMaterial, modelMat);
            }
        }
        m_depthShader->End();
        EndTextureMode();
    }

    rlSetCullFace(RL_CULL_FACE_BACK);

    if (m_pointDepthShader && m_pointDepthShader->IsValid())
    {
        rlEnableDepthTest();
        rlEnableDepthMask();
        rlDisableBackfaceCulling();

        Vector3 dirs[6] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
        Vector3 ups[6] = {{0, -1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, {0, -1, 0}, {0, -1, 0}};
        for (int i = 0; i < m_activePointCasters.size(); ++i)
        {
            auto &caster = m_activePointCasters[i];
            PointShadowMap &psm = m_pointShadowMaps[i];

            Vector3 lightPos = caster.light->owner->GetComponent<TransformComponent>().GetWorldPosition();
            float farPlane = caster.light->range;

            glViewport(0, 0, psm.resolution, psm.resolution);
            glBindFramebuffer(GL_FRAMEBUFFER, psm.fboId);

            Matrix matProj = MatrixPerspective(90.0f * DEG2RAD, 1.0f, 0.1f, farPlane);

            m_pointDepthShader->Begin();
            {
                m_pointDepthShader->SetVec3("lightPos", Vector3f(lightPos.x, lightPos.y, lightPos.z));
                m_pointDepthShader->SetFloat("farPlane", farPlane);

                for (int face = 0; face < 6; ++face)
                {
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, psm.cubemapId, 0);

                    glClear(GL_DEPTH_BUFFER_BIT);

                    Vector3 target = Vector3Add(lightPos, dirs[face]);
                    Matrix matView = MatrixLookAt(lightPos, target, ups[face]);
                    Matrix matVP = MatrixMultiply(matView, matProj);

                    m_pointDepthShader->SetMat4("lightVP", Matrix4f(matVP));

                    for (auto *obj : renderables)
                    {
                        auto &render = obj->GetComponent<RenderComponent>();
                        auto &tf = obj->GetComponent<TransformComponent>();
                        if (!render.castShadows || obj == caster.owner)
                            continue;
                        Matrix4f modelMat = tf.GetWorldMatrix();

                        m_pointDepthShader->SetMat4("model", modelMat);

                        Material tempRaylibMaterial;
                        for (int m = 0; m < render.model.meshCount; m++)
                        {
                            tempRaylibMaterial = render.model.materials[m];
                            tempRaylibMaterial.shader = m_pointDepthShader->GetShader();
                            DrawMesh(render.model.meshes[m], tempRaylibMaterial, modelMat);
                        }
                    }
                }
            }
            m_pointDepthShader->End();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    }
}
Matrix4f LightingManager::CalculateDirectionalLightVP(const Vector3f &lightDir, const Vector3f &centerPos)
{
    Vector3f lightPos = centerPos - lightDir * 40.0f;
    Vector3f up = {0.0f, 1.0f, 0.0f};
    Vector3f dirNormalized = Vector3Normalize(lightDir);

    if (fabs(up * dirNormalized) > 0.99f)
    {
        up = {0.0f, 0.0f, 1.0f};
    }
    Matrix matView = MatrixLookAt(lightPos, centerPos, up);

    float size = 100.0f;
    float nearPlane = 0.1f;
    float farPlane = 500.0f;
    Matrix matProj = MatrixOrtho(-size, size, -size, size, nearPlane, farPlane);

    return Matrix4f(MatrixMultiply(matView, matProj));
}

RenderTexture2D *LightingManager::GetShadowMap(int index)
{
    if (index >= 0 && index < m_shadowMaps.size())
        return &m_shadowMaps[index];
}