#include "ParticleSystem.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/ParticleEmitterComponent.h"
#include "Engine/Core/Components/TransformComponent.h"
#include "rlgl.h"

ParticleSystem::ParticleSystem(GameWorld *world)
{
    owner_world = world;
    m_TFBManager = std::make_unique<TFBManager>();
}
ParticleSystem::~ParticleSystem()
{
    m_emitterBuffers.clear();
};

GPUParticleBuffer *ParticleSystem::GetOrCreateBuffer(std::shared_ptr<ParticleEmitter> emitter)
{
    auto it = m_emitterBuffers.find(emitter);
    if (it == m_emitterBuffers.end())
    {
        // auto a = GPUParticleBuffer(emitter->GetMaxParticles());
        auto buff = std::make_unique<GPUParticleBuffer>(emitter->GetMaxParticles(), emitter->GetRenderPasses());
        GPUParticleBuffer *buffPtr = buff.get();
        m_emitterBuffers[emitter] = std::move(buff);
        return buffPtr;
    }

    if (it->second->GetMaxParticles() != emitter->GetMaxParticles())
    {
        std::cout << "[ParticleSystem]: Resize GPU Buffer: " << it->second->GetMaxParticles() << " -> " << emitter->GetMaxParticles() << std::endl;
        it->second = std::make_unique<GPUParticleBuffer>(emitter->GetMaxParticles(), emitter->GetRenderPasses());
        emitter->ResetInsertionIndex();
    }
    return it->second.get();
}
#include <nlohmann/json.hpp>
#include <fstream>
#include "Engine/Utils/JsonParser.h"
using json = nlohmann::json;
void ParticleSystem::LoadEffectLibrary(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[ParicleSystem]: Failed to open effect library file: " << path << std::endl;
        return;
    }
    json effectLib = json::parse(file);
    for (auto &[name, params] : effectLib.items())
        m_effectLibray[name] = params;
}
void ParticleSystem::InternalSpawn(const std::string &effectName, const ParticleParams &params)
{
    auto it = m_effectLibray.find(effectName);
    if (it == m_effectLibray.end())
    {
        std::cerr << "[ParicleSystem]: Failed to find effect: " << effectName << std::endl;
        return;
    }
    json config = it->second;
    for (auto &initConfig : config["initializers"])
    {
        for (auto &[key, value] : initConfig["params"].items())
            if (value.is_number())
                initConfig["params"][key] = params.Get<float>(key, value);
            else if (value.is_array())
            {
                switch (value.size())
                {
                case 2:
                    initConfig["params"][key] = JsonParser::Vector2fToJson(params.Get<Vector2f>(key, Vector2f(value[0], value[1])));
                    break;
                case 3:
                    initConfig["params"][key] = JsonParser::Vector3fToJson(params.Get<Vector3f>(key, Vector3f(value[0], value[1], value[2])));
                    break;
                case 4:
                    initConfig["params"][key] = JsonParser::Vector4fToJson(params.Get<Vector4f>(key, Vector4f(value[0], value[1], value[2], value[3])));
                    break;
                default:
                    std::cerr << "[ParicleSystem]: Invalid array size in effect library: " << std::endl;
                    break;
                }
            }
    }

    auto emitter = std::make_shared<ParticleEmitter>(config, owner_world->GetParticleFactory(), owner_world->GetResourceManager());
    emitter->simSpace = SimulationSpace::WORLD;

    Vector3f pos = params.Get<Vector3f>("spawnPos", Vector3f(0, 0, 0));
    Quat4f rot = Quat4f::dirToQuat(params.Get<Vector3f>("direction", Vector3f(0, 0, 1)));
    Vector3f scale = Vector3f::ONE;
    auto tf = TransformComponent(pos, rot, scale);

    if (config.value("isBurst", false))
    {
        GPUParticleBuffer *buffer = GetOrCreateBuffer(emitter);
        emitter->Burst(tf, *buffer);
        emitter->SetEmissionRate(0.0f);
    }
    else
    {
        emitter->SetEmissionRate(config.value("emissionRate", 0.0f));
    }
    this->RegisterOrphan(emitter, tf);
}

void ParticleSystem::Update(GameWorld &gameWorld, float dt)
{
    auto entities = gameWorld.GetEntitiesWith<ParticleEmitterComponent, TransformComponent>();
    // 实体携带粒子
    for (auto *entity : entities)
    {
        auto &ec = entity->GetComponent<ParticleEmitterComponent>();
        auto &ownerTf = entity->GetComponent<TransformComponent>();
        if (!ec.activate)
            continue;
        for (auto &emitter : ec.emitters)
        {
            // 生成GPU粒子
            GPUParticleBuffer *buffer = GetOrCreateBuffer(emitter);
            emitter->Update(dt, ownerTf, *buffer);
            if (emitter->GetUpdateShader())
            {
                // emitter->PrepareForces(ownerTf); 计算力的在随体系与世界系的变换
                m_TFBManager->Simulate(*(emitter->GetUpdateShader()), *buffer, (int)emitter->GetMaxParticles(), dt);
            }
        }
    }

    // 遗留粒子
    for (auto it = m_orphans.begin(); it != m_orphans.end();)
    {
        GPUParticleBuffer *buffer = GetOrCreateBuffer(it->emitter);

        if (it->emitter->GetUpdateShader())
        {
            // it->emitter->PrepareForces(it->lastTransform);计算力的在随体系与世界系的变换
            m_TFBManager->Simulate(*(it->emitter->GetUpdateShader()), *buffer, (int)it->emitter->GetMaxParticles(), dt);
        }
        if (it->emitter->IsFinished())
        {
            m_emitterBuffers.erase(it->emitter);
            it = m_orphans.erase(it);
        }
        else
            ++it;
    }
}

void ParticleSystem::RegisterOrphan(std::shared_ptr<ParticleEmitter> emitter, const TransformComponent &lastTf)
{
    emitter->SetEmissionRate(0.0f);
    m_orphans.push_back({emitter, lastTf});
}

#if defined(PLATFORM_WEB)
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#else
#include "external/glad.h"
#endif
#include "string"
void ParticleSystem::Render(std::unordered_map<std::string, RenderTexture2D> &RTPool, float realTime, float gameTime, const Matrix4f &VP, GameWorld &gameWorld, mCamera &camera)
{
    rlDrawRenderBatchActive();
    rlEnableDepthTest();
    rlDisableDepthMask();
    rlDisableBackfaceCulling();

    auto &sceneDepth = RTPool["inScreen"].depth;

    auto entities = gameWorld.GetEntitiesWith<ParticleEmitterComponent, TransformComponent>();
    for (auto *entity : entities)
    {
        auto &ec = entity->GetComponent<ParticleEmitterComponent>();
        auto &ownerTf = entity->GetComponent<TransformComponent>();
        if (!ec.activate)
            continue;
        for (auto &emitter : ec.emitters)
        {
            GPUParticleBuffer *buffer = GetOrCreateBuffer(emitter);
            if (!buffer)
                continue;
            Matrix4f renderModelMat = emitter->GetRenderMatrix(ownerTf);

            emitter->Render(RTPool, *buffer, sceneDepth, renderModelMat, camera.Position(),
                            realTime, gameTime, VP, camera);
        }
    }
    // 遗留粒子
    for (auto &orphan : m_orphans)
    {
        GPUParticleBuffer *buffer = GetOrCreateBuffer(orphan.emitter);
        if (!buffer)
            continue;

        Matrix4f renderModelMat = orphan.emitter->GetRenderMatrix(orphan.lastTransform);

        orphan.emitter->Render(RTPool, *buffer, sceneDepth, renderModelMat, camera.Position(),
                               realTime, gameTime, VP, camera);
    }
    glDepthMask(GL_TRUE);
}
