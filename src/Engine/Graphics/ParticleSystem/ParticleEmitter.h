#pragma once
#include "GPUParticleBuffer.h"
#include "IParticleInitializer.h"
#include "Engine/Core/Components/TransformComponent.h"
#include <memory>
#include <vector>

enum class SimulationSpace
{
    LOCAL = 0,
    WORLD = 1
};

class ParticleEmitter
{
public:
    SimulationSpace simSpace = SimulationSpace::WORLD; // 默认世界系

    ParticleEmitter(size_t maxParticles, float emissionRate);

    void Update(float deltaTime, const TransformComponent &tf, GPUParticleBuffer &particleBuffer);
    void AddInitializer(std::shared_ptr<IParticleInitializer> initializer);

private:
    float m_emissionRate;
    float m_accumulator = 0.0f;
    size_t m_insertionIndex = 0; // 循环缓冲区写指针
    size_t m_maxParticles;

    std::vector<std::shared_ptr<IParticleInitializer>> m_initializers;
    std::vector<GPUParticle> m_spawnBuffer; // 临时缓冲，传给GPU前组装数据
};