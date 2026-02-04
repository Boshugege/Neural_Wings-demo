#include "TFBManager.h"
#include "rlgl.h"

#if defined(PLATFORM_WEB)
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#else
#include "external/glad.h"
#endif

void TFBManager::Simulate(ShaderWrapper &shader, GPUParticleBuffer &buffer, int count, float dt)
{
    if (count <= 0)
        return;

    shader.Begin();

    // TODO:仿照RenderMaterial加入用户自定义uniform功能
    shader.SetFloat("u_deltaTime", dt);

    // 关闭光栅化，仅vs
    glEnable(GL_RASTERIZER_DISCARD);

    // 绑定读写缓冲
    glBindVertexArray(buffer.GetReadVAO());
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer.GetWriteVBO());

    glBeginTransformFeedback(GL_POINTS);
    // draw call
    glDrawArrays(GL_POINTS, 0, count);
    glEndTransformFeedback();

    // 恢复状态
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
    glBindVertexArray(0);
    glDisable(GL_RASTERIZER_DISCARD);

    shader.End();

    // 交换缓冲区，本帧输出作为下帧输入
    buffer.Swap();
}