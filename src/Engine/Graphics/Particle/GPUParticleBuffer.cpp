#include "GPUParticleBuffer.h"
#include "rlgl.h"
#include "Engine/Graphics/ShaderWrapper.h"
#if defined(PLATFORM_WEB)
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#else
#include "external/glad.h"
#endif
#include "Engine/Graphics/RenderMaterial.h"

GPUParticleBuffer::GPUParticleBuffer(size_t maxParticles, const std::vector<RenderMaterial> &passes)
{
    m_maxParticles = maxParticles;
    SetupBuffers();
    SetupQuadVBO();
    // 为每个shader生成vao
    for (const auto &mat : passes)
    {
        if (mat.shader && mat.shader->IsValid())
            AddRenderVAO(mat.shader->GetShader());
        else
        {
            m_multiRenderVAOs.push_back({0, 0});
            std::cerr << "[GPUParticleBuffer] Warning: Pass has invalid shader, VAO 0 created." << std::endl;
        }
    }
}

GPUParticleBuffer::~GPUParticleBuffer()
{
    glDeleteBuffers(2, m_vbos);
    glDeleteVertexArrays(2, m_vaos);
}
void GPUParticleBuffer::SetupQuadVBO()
{
    if (m_quadVBO != 0)
        return;
    float quadVertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f};
    glGenBuffers(1, &m_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
#define BIND_ATTRIB(loc, member, type, count) \
    glEnableVertexAttribArray(loc);           \
    glVertexAttribPointer(loc, count, type, GL_FALSE, sizeof(GPUParticle), (void *)offsetof(GPUParticle, member))

#define BIND_ATTRIB_SAFE(shader, name, member, type, count, divisor)                           \
    {                                                                                          \
        int loc = GetShaderLocationAttrib(shader, name);                                       \
        if (loc >= 0)                                                                          \
        {                                                                                      \
            glEnableVertexAttribArray(loc);                                                    \
            glVertexAttribPointer(loc, count, type, GL_FALSE,                                  \
                                  sizeof(GPUParticle), (void *)offsetof(GPUParticle, member)); \
            if (divisor > 0)                                                                   \
                glVertexAttribDivisor(loc, divisor);                                           \
        }                                                                                      \
    }
#define BIND_ATTRIB_I_SAFE(shader, name, member, divisor)                                       \
    {                                                                                           \
        int loc = GetShaderLocationAttrib(shader, name);                                        \
        if (loc >= 0)                                                                           \
        {                                                                                       \
            glEnableVertexAttribArray(loc);                                                     \
            glVertexAttribIPointer(loc, 1, GL_UNSIGNED_INT,                                     \
                                   sizeof(GPUParticle), (void *)offsetof(GPUParticle, member)); \
            if (divisor > 0)                                                                    \
                glVertexAttribDivisor(loc, divisor);                                            \
        }                                                                                       \
    }

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
        BIND_ATTRIB(1, velocity, GL_FLOAT, 3);
        BIND_ATTRIB(2, acceleration, GL_FLOAT, 3);
        BIND_ATTRIB(3, color, GL_FLOAT, 4);
        BIND_ATTRIB(4, size, GL_FLOAT, 2);
        BIND_ATTRIB(5, rotation, GL_FLOAT, 1);
        BIND_ATTRIB(6, life, GL_FLOAT, 2);

        glEnableVertexAttribArray(7);
        glVertexAttribIPointer(7, 1, GL_UNSIGNED_INT, stride, (void *)offsetof(GPUParticle, randomID));

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

void GPUParticleBuffer::AddRenderVAO(const Shader &renderShader)
{
    std::array<unsigned int, 2> newVAOs = {0, 0};
    glGenVertexArrays(2, newVAOs.data());
    size_t stride = sizeof(GPUParticle);

    for (int i = 0; i < 2; ++i)
    {
        glBindVertexArray(newVAOs[i]);
        // 绑定顶点属性
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        {

            int vPosLoc = GetShaderLocationAttrib(renderShader, "vertexPosition");
            if (vPosLoc >= 0)
            {
                glEnableVertexAttribArray(vPosLoc);
                glVertexAttribPointer(vPosLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
            }
            int vTexLoc = GetShaderLocationAttrib(renderShader, "vertexTexCoord");
            if (vTexLoc >= 0)
            {
                glEnableVertexAttribArray(vTexLoc);
                glVertexAttribPointer(vTexLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
            }
            glBindBuffer(GL_ARRAY_BUFFER, m_vbos[i]);

            BIND_ATTRIB_SAFE(renderShader, "pPosition", position, GL_FLOAT, 3, 1);
            BIND_ATTRIB_SAFE(renderShader, "pVelocity", velocity, GL_FLOAT, 3, 1);
            BIND_ATTRIB_SAFE(renderShader, "pAcceleration", acceleration, GL_FLOAT, 3, 1);
            BIND_ATTRIB_SAFE(renderShader, "pColor", color, GL_FLOAT, 4, 1);
            BIND_ATTRIB_SAFE(renderShader, "pSize", size, GL_FLOAT, 2, 1);
            BIND_ATTRIB_SAFE(renderShader, "pRotation", rotation, GL_FLOAT, 1, 1);
            BIND_ATTRIB_SAFE(renderShader, "pLife", life, GL_FLOAT, 2, 1);
            BIND_ATTRIB_I_SAFE(renderShader, "pRandomID", randomID, 1);

            glBindVertexArray(0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    m_multiRenderVAOs.push_back(newVAOs);
}
// void GPUParticleBuffer::SetupRenderVBO(const Shader &renderShader)

// {

//     // 格式 Pos(3)+UV(2)
//     // XY平面，两个三角形组成
//     /*
//     0 3
//     1 2
//     */
//     float quadVertices[] =
//         {
//             //
//             -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // 1
//             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // 2
//             -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,  // 0
//                                             //
//             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // 2
//             0.5f, 0.5f, 0.0f, 1.0f, 1.0f,   // 3
//             -0.5f, 0.5f, 0.0f, 0.0f, 1.0f   // 0
//         };
//     if (m_quadVBO == 0)
//         glGenBuffers(1, &m_quadVBO);
//     if (m_renderVAOS[0] == 0)
//         glGenVertexArrays(2, m_renderVAOS);

//     glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
//     for (int i = 0; i < 2; ++i)
//     {
//         glBindVertexArray(m_renderVAOS[i]);
//         // 绑定顶点属性
//         glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);

//         int vPosLoc = GetShaderLocationAttrib(renderShader, "vertexPosition");
//         if (vPosLoc >= 0)
//         {
//             glEnableVertexAttribArray(vPosLoc);
//             glVertexAttribPointer(vPosLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
//         }
//         int vTexLoc = GetShaderLocationAttrib(renderShader, "vertexTexCoord");
//         if (vTexLoc >= 0)
//         {
//             glEnableVertexAttribArray(vTexLoc);
//             glVertexAttribPointer(vTexLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
//         }
//         glBindBuffer(GL_ARRAY_BUFFER, m_vbos[i]);

//         BIND_ATTRIB_SAFE(renderShader, "pPosition", position, GL_FLOAT, 3, 1);
//         BIND_ATTRIB_SAFE(renderShader, "pVelocity", velocity, GL_FLOAT, 3, 1);
//         BIND_ATTRIB_SAFE(renderShader, "pAcceleration", acceleration, GL_FLOAT, 3, 1);
//         BIND_ATTRIB_SAFE(renderShader, "pColor", color, GL_FLOAT, 4, 1);
//         BIND_ATTRIB_SAFE(renderShader, "pSize", size, GL_FLOAT, 2, 1);
//         BIND_ATTRIB_SAFE(renderShader, "pRotation", rotation, GL_FLOAT, 1, 1);
//         BIND_ATTRIB_SAFE(renderShader, "pLife", life, GL_FLOAT, 2, 1);
//         BIND_ATTRIB_I_SAFE(renderShader, "pRandomID", randomID, 1);

//         glBindVertexArray(0);
//         glBindBuffer(GL_ARRAY_BUFFER, 0);
//     }
// }
void GPUParticleBuffer::BindForRender(size_t passIndex)
{
    if (passIndex < m_multiRenderVAOs.size())
        glBindVertexArray(m_multiRenderVAOs[passIndex][m_readIdx]);
}
