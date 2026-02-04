#include "GPUParticleBuffer.h"
#include "rlgl.h"

#if defined(PLATFORM_WEB)
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#else
#include "external/glad.h"
#endif

GPUParticleBuffer::GPUParticleBuffer(size_t maxParticles)
{
    m_maxParticles = maxParticles;
    SetupBuffers();
}

GPUParticleBuffer::~GPUParticleBuffer()
{
    glDeleteBuffers(2, m_vbos);
    glDeleteVertexArrays(2, m_vaos);
}

#define BIND_ATTRIB(loc, member, type, count) \
    glEnableVertexAttribArray(loc);           \
    glVertexAttribPointer(loc, count, type, GL_FALSE, sizeof(GPUParticle), (void *)offsetof(GPUParticle, member))

void GPUParticleBuffer::SetupBuffers()
{
    glGenVertexArrays(2, m_vaos);
    glGenBuffers(2, m_vbos);
    size_t totalSize = m_maxParticles * sizeof(GPUParticle);

    for (size_t i = 0; i < 2; i++)
    {
        glBindVertexArray(m_vaos[i]);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbos[i]);

        // 分配显存
        glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_DYNAMIC_DRAW);
        size_t stride = sizeof(GPUParticle);

        // 设置属性指针
        BIND_ATTRIB(0, position, GL_FLOAT, 3);
        BIND_ATTRIB(1, spaceFlag, GL_FLOAT, 1);
        BIND_ATTRIB(2, velocity, GL_FLOAT, 3);
        BIND_ATTRIB(3, acceleration, GL_FLOAT, 3);
        BIND_ATTRIB(4, color, GL_FLOAT, 4);
        BIND_ATTRIB(5, size, GL_FLOAT, 2);
        BIND_ATTRIB(6, rotation, GL_FLOAT, 1);
        BIND_ATTRIB(7, life, GL_FLOAT, 2);

        glEnableVertexAttribArray(8);
        glVertexAttribIPointer(8, 1, GL_UNSIGNED_INT, stride, (void *)offsetof(GPUParticle, randomID));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void GPUParticleBuffer::Swap()
{
    m_readIdx ^= 1;
}

void GPUParticleBuffer::UpdateSubData(const std::vector<GPUParticle> &newParticles, size_t offset)
{
    if (newParticles.empty())
        return;

    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[m_readIdx]);
    glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(GPUParticle), newParticles.size() * sizeof(GPUParticle), newParticles.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
