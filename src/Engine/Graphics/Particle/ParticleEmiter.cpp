#include "ParticleEmitter.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <vector>
using json = nlohmann::json;

ParticleEmitter::ParticleEmitter(const json &config, const ParticleFactory &factory, ResourceManager &rm)
{
    LoadFromConfig(config, factory, rm);
}
void ParticleEmitter::ResetInsertionIndex()
{
    m_insertionIndex = 0;
}
void ParticleEmitter::LoadFromConfig(const json &config, const ParticleFactory &factory, ResourceManager &rm)
{
    if (config.contains("maxParticles"))
    {
        size_t maxP = config["maxParticles"];
        if (m_maxParticles != maxP)
        {
            m_maxParticles = maxP;
            ResetInsertionIndex();
            m_spawnBuffer.clear();
        }
    }
    m_emissionRate = config.value("emissionRate", m_emissionRate);
    m_maxLife = config.value("maxLife", m_maxLife);
    if (config.contains("space"))
    {
        std::string space = config["space"];
        simSpace = (space == "WORLD") ? SimulationSpace::WORLD : SimulationSpace::LOCAL;
    }
    if (config.contains("updateShader"))
    {
        std::string vsPath = config["updateShader"];
        std::vector<std::string> varyings =
            {
                "outPosition",
                "outVelocity",
                "outAcceleration",
                "outColor",
                "outSizeRotation",
                "outLifeRand",
            };
        m_updateShader = rm.GetTFBShader(vsPath, varyings);
    }
    else
    {
        std::cerr << "[ParticleEmitter]: No update shader specified!" << std::endl;
        return;
    }
    if (config.contains("initializers"))
    {
        m_initializers.clear();
        for (const auto &initConfig : config["initializers"])
        {
            std::string name = initConfig["name"];
            auto initObj = factory.CreatorInitializer(name);
            if (initObj)
            {
                if (initConfig.contains("params"))
                    initObj->LoadConfig(initConfig["params"]);
                m_initializers.push_back(std::move(initObj));
            }
        }
    }
    if (config.contains("passes"))
    {
        m_passes.clear();
        auto &matData = config["passes"];
        for (const auto &pass : matData)
        {
            RenderMaterial mat;
            mat.LoadFromConfig(pass, rm);
            m_passes.push_back(mat);
        }
    }
}
void ParticleEmitter::Update(float deltaTime, const TransformComponent &ownerTf, GPUParticleBuffer &particleBuffer)
{
    m_lastDeltaTime += deltaTime;
    m_accumulator += deltaTime;
    int spawnCounts = (int)(m_accumulator * m_emissionRate);
    m_accumulator -= spawnCounts / (m_emissionRate);
    if (spawnCounts > 0)
    {
        m_lastDeltaTime = 0.0f;
        Spawn(spawnCounts, ownerTf, particleBuffer);
    }
}

void ParticleEmitter::Burst(const TransformComponent &ownerTf, GPUParticleBuffer &particleBuffer)
{
    size_t count = 0;
    for (auto &init : m_initializers)
        count += init->BurstCount();
    if (count > 0)
        Spawn(count, ownerTf, particleBuffer);
}

void ParticleEmitter::Spawn(int spawnCounts, const TransformComponent &ownerTf, GPUParticleBuffer &particleBuffer)
{

    // TODO
    // if (spawnCounts > 8192)
    //     spawnCounts = 8192; // 防止卡顿
    m_spawnBuffer.assign(spawnCounts, GPUParticle());

    // 初始化
    for (auto &init : m_initializers)
        init->Initialize(m_spawnBuffer, 0, spawnCounts);

    // 随体系世界系处理
    if (simSpace == SimulationSpace::WORLD)
    {
        for (auto &particle : m_spawnBuffer)
        {

            particle.position = (ownerTf.GetWorldRotation() * (particle.position & ownerTf.GetWorldScale())) + ownerTf.GetWorldPosition();
            particle.velocity = ownerTf.GetWorldRotation() * particle.velocity;
            particle.acceleration = ownerTf.GetWorldRotation() * particle.acceleration;
        }
    }

    // 写入GPU
    // 循环写入，新粒子覆盖旧粒子
    if (m_insertionIndex + spawnCounts <= m_maxParticles)
    {
        particleBuffer.UpdateSubData(m_spawnBuffer, m_insertionIndex);
        m_insertionIndex += spawnCounts;
    }
    else
    {
        // 拆分两段
        size_t firstPartCount = m_maxParticles - m_insertionIndex;
        size_t secondPartCount = spawnCounts - firstPartCount;
        std::vector<GPUParticle> firstPart(m_spawnBuffer.begin(), m_spawnBuffer.begin() + firstPartCount);
        std::vector<GPUParticle> secondPart(m_spawnBuffer.begin() + firstPartCount, m_spawnBuffer.end());
        particleBuffer.UpdateSubData(firstPart, m_insertionIndex);
        particleBuffer.UpdateSubData(secondPart, 0);

        m_insertionIndex = secondPartCount;
    }
    if (m_insertionIndex >= m_maxParticles)
        m_insertionIndex = 0;
}

void ParticleEmitter::AddInitializer(std::shared_ptr<IParticleInitializer> initializer)
{
    m_initializers.push_back(initializer);
}

size_t ParticleEmitter::GetMaxParticles() const
{
    return m_maxParticles;
}
float ParticleEmitter::GetEmissionRate() const
{
    return m_emissionRate;
}
void ParticleEmitter::SetEmissionRate(float emissionRate)
{
    m_emissionRate = emissionRate;
}
bool ParticleEmitter::IsFinished() const
{
    return (m_emissionRate <= 0.001f) && (m_lastDeltaTime > m_maxLife);
}
void ParticleEmitter::SetMaxLife(float maxLife)
{
    m_maxLife = maxLife;
}

std::shared_ptr<ShaderWrapper> ParticleEmitter::GetUpdateShader() const
{
    return m_updateShader;
}

std::vector<RenderMaterial> &ParticleEmitter::GetRenderPasses()
{
    return m_passes;
}

#include "rlgl.h"

#if defined(PLATFORM_WEB)
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#else
#include "external/glad.h"
#endif
#include <string>

void ParticleEmitter::RenderSignlePass(size_t passIndex, const RenderMaterial &pass, std::unordered_map<std::string, RenderTexture2D> &RTPool, GPUParticleBuffer &gpuBuffer, const Texture2D &sceneDepth, const Matrix4f &modelMat,
                                       const Vector3f &viewPos, float realTime, float gameTime,
                                       const Matrix4f &VP, const Matrix4f &matProj, const mCamera &camera)
{
    if (!pass.shader || !pass.shader->IsValid())
        return;

    std::string outputRT = pass.outputRT;
    auto itRT = RTPool.find(pass.outputRT);
    if (itRT == RTPool.end())
        return;

    BeginTextureMode(itRT->second);
    {
        rlDrawRenderBatchActive();
        rlEnableVertexArray(0);
        rlSetTexture(0);
        pass.shader->Begin();

        Vector3f right = camera.Right();
        Vector3f up = camera.Up();
        Vector3f dir = camera.Direction();

        int texUnit = 1;

        pass.shader->SetTexture("dataTex", m_dataTexture, texUnit++);
        pass.shader->SetInt("maxParticles", m_maxParticles);

        for (const auto &[name, texture] : pass.customTextures)
        {
            if (texture.id > 0)
            {
                pass.shader->SetTexture(name, texture, texUnit);
                if (pass.isAnimated.at(name))
                {
                    pass.shader->SetInt(name + "_frameCount", pass.frameCount.at(name));
                    pass.shader->SetFloat(name + "_animSpeed", pass.animSpeed.at(name));
                }
                else
                {
                    pass.shader->SetInt(name + "_frameCount", 1);
                    pass.shader->SetFloat(name + "_animSpeed", 0.0f);
                }

                texUnit++;
            }
        }
        if (sceneDepth.id > 0)
        {
            pass.shader->SetTexture("sceneDepth", sceneDepth, texUnit);
            texUnit++;
        }

        pass.shader->SetVec2("resolution", Vector2f(GetScreenWidth(), GetScreenHeight()));
        pass.shader->SetFloat("near", camera.getNearPlane());
        pass.shader->SetFloat("far", camera.getFarPlane());

        pass.shader->SetVec3("cameraDir", dir);
        pass.shader->SetVec3("cameraRight", right);
        pass.shader->SetVec3("cameraUp", up);
        pass.shader->SetMat4("vp", VP);
        pass.shader->SetMat4("proj", matProj);
        pass.shader->SetMat4("model", modelMat);
        pass.shader->SetAll(Matrix4f::identity(), Matrix4f::identity(), viewPos, realTime, gameTime,
                            pass.baseColor,
                            pass.customFloats,
                            pass.customVector2,
                            pass.customVector3,
                            pass.customVector4);

        rlDisableBackfaceCulling();

        if (pass.depthTest)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
        }
        else
            glDisable(GL_DEPTH_TEST);

        if (pass.depthWrite)
            glDepthMask(GL_TRUE);
        else
            glDepthMask(GL_FALSE);

        glEnable(GL_BLEND);
        switch (pass.blendMode)
        {
        case BLEND_OPIQUE:
            glDisable(GL_BLEND);
            break;
        case BLEND_MULTIPLIED:
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            glBlendEquation(GL_FUNC_ADD);
            break;
        case BLEND_SCREEN:
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
            glBlendEquation(GL_FUNC_ADD);
            break;
        case BLEND_SUBTRACT:
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
            break;
        case BLEND_ADDITIVE:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            break;
        default:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            break;
        }

        gpuBuffer.BindForRender(passIndex);

        glDisable(GL_RASTERIZER_DISCARD);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)m_maxParticles);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glActiveTexture(GL_TEXTURE0);
        rlEnableVertexArray(0);
        rlSetTexture(0);
        rlDisableTexture();
        rlEnableDepthMask();
        EndBlendMode();

        pass.shader->End();
        rlEnableColorBlend();
    }
    EndTextureMode();
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error: " << err << std::endl;
    }
}
void ParticleEmitter::Render(std::unordered_map<std::string, RenderTexture2D> &RTPool, GPUParticleBuffer &gpuBuffer, const Texture2D &sceneDepth, const Matrix4f &modelMat,
                             const Vector3f &viewPos, float realTime, float gameTime,
                             const Matrix4f &VP, const Matrix4f &matProj, const mCamera &camera)
{
    for (size_t i = 0; i < m_passes.size(); ++i)
    {
        RenderSignlePass(i, m_passes[i], RTPool, gpuBuffer, sceneDepth, modelMat, viewPos, realTime, gameTime, VP, matProj, camera);
    }
}

Matrix4f ParticleEmitter::GetRenderMatrix(const TransformComponent &parentTf) const
{
    if (simSpace == SimulationSpace::WORLD)
        return Matrix4f::identity();
    else
        return parentTf.GetLocalMatrix(); // local space
}

unsigned int ParticleEmitter::GetDataTextureID() const
{
    return m_dataTexture.id;
}
void ParticleEmitter::EnsureDataTextureSize(size_t maxParticles)
{
    if (m_dataTexture.id == 0 || maxParticles != m_dataTexture.height)
    {
        m_dataTexture.id = rlLoadTexture(nullptr, 6, maxParticles, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);
        m_dataTexture.width = 6;
        m_dataTexture.height = maxParticles;
        m_dataTexture.mipmaps = 1;
        m_dataTexture.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;

        SetTextureFilter(m_dataTexture, TEXTURE_FILTER_POINT);
    }
}
Texture2D &ParticleEmitter::GetDataTexture()
{
    return m_dataTexture;
}