#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter(size_t maxParticles, float emissionRate)
{
    m_maxParticles = maxParticles;
    m_emissionRate = emissionRate;
}

void ParticleEmitter::Update(float deltaTime, const TransformComponent &tf, GPUParticleBuffer &particleBuffer)
{
    m_accumulator += deltaTime;
    int spawnCounts = (int)(m_accumulator * m_emissionRate);
    m_accumulator -= spawnCounts / (m_emissionRate);
    if (spawnCounts > 0)
    {
        if (spawnCounts > 100)
            spawnCounts = 100; // 防止卡顿
        m_spawnBuffer.assign(spawnCounts, GPUParticle());

        // 初始化
        for (auto &init : m_initializers)
            init->Initialize(m_spawnBuffer, tf, (int)simSpace);

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
}
void ParticleEmitter::AddInitializer(std::shared_ptr<IParticleInitializer> initializer)
{
    m_initializers.push_back(initializer);
}