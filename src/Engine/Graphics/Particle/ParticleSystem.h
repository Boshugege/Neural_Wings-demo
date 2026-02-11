#pragma once
#include "ParticleEmitter.h"
#include "TFBManager.h"
#include "GPUParticleBuffer.h"

#include <unordered_map>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class GameWorld;
class mCamera;

class ParticleSystem
{
public:
    ParticleSystem(GameWorld *world);
    ~ParticleSystem();

    void Update(GameWorld &gameWorld, float dt);
    void Render(std::unordered_map<std::string, RenderTexture2D> &RTPool, float realTime, float gameTime, const Matrix4f &VP, const Matrix4f &matProj, GameWorld &gameWorld, mCamera &camera);
    // 若gameobject销毁，保留world发射器，并记录其最后位置
    void RegisterOrphan(std::shared_ptr<ParticleEmitter> emitter, const TransformComponent &lastTransform);

    // 直接生成粒子
    template <typename... Args>
    void Spawn(const std::string &effectName, const Vector3f &pos, Args &&...args)
    {
        ParticleParams p;
        p.Set("spawnPos", pos);
        FillParams(p, std::forward<Args>(args)...);
        InternalSpawn(effectName, p);
    }
    void FillParams(ParticleParams &p) {}
    template <typename T, typename... Args>
    void FillParams(ParticleParams &p, const std::string &key, T &&value, Args &&...args)
    {
        p.Set(key, std::forward<T>(value));
        FillParams(p, std::forward<Args>(args)...);
    }
    void LoadEffectLibrary(const std::string &path);

private:
    GameWorld *owner_world = nullptr;
    void InternalSpawn(const std::string &effectName, const ParticleParams &params);
    std::unordered_map<std::string, json> m_effectLibray;

    std::unordered_map<std::shared_ptr<ParticleEmitter>, std::unique_ptr<GPUParticleBuffer>> m_emitterBuffers;
    struct OrphanEmitter
    {
        std::shared_ptr<ParticleEmitter> emitter;
        TransformComponent lastTransform;
    };
    std::vector<OrphanEmitter> m_orphans;
    std::unique_ptr<TFBManager> m_TFBManager;

    // 确保缓冲存在
    GPUParticleBuffer *GetOrCreateBuffer(std::shared_ptr<ParticleEmitter> emitter);
};