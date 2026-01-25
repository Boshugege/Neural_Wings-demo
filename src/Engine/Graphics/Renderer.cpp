#include "Renderer.h"
#include "Engine/Core/GameWorld.h"
#include "CameraManager.h"
#include "Engine/Core/Components/TransformComponent.h"
#include "Engine/Core/Components/RenderComponent.h"

void Renderer::AddRenderView(const RenderView& view) {
    m_renderViews.push_back(view);
}

void Renderer::ClearRenderViews() {
    m_renderViews.clear();
}

void Renderer::RenderScene(const GameWorld& world, CameraManager& cameraManager) {
    for (const auto& view : m_renderViews) {
        Camera3D* camera = cameraManager.GetCamera(view.cameraName);
        if (camera) {
            BeginScissorMode(view.viewport.x, view.viewport.y, view.viewport.width, view.viewport.height);
            //透明底？
            if (view.clearBackground) {
                ClearBackground(view.backgroundColor);
            }

            BeginMode3D(*camera);
            DrawWorldObjects(world);
            EndMode3D();
            
            // TODO：（debug）为视口绘制边框
            DrawRectangleLinesEx(view.viewport, 2, GRAY);

            EndScissorMode();
        }
    }
}


// TODO: 数学库替换 
float QuaternionToAxisAngle(Quaternion q, Vector3 *axis) {
    float angle = 2.0f * std::acosf(q.w);
    float s = std::sqrtf(1.0f - q.w * q.w);
    if (s < 0.001f) {
        if (axis) {
            axis->x = q.x;
            axis->y = q.y;
            axis->z = q.z;
        }
    } else {
        if (axis) {
            axis->x = q.x / s;
            axis->y = q.y / s;
            axis->z = q.z / s;
        }
    }
    return angle * RAD2DEG;
}

void Renderer::DrawWorldObjects(const GameWorld& world) {
    for (const auto& gameObject : world.GetGameObjects()) {
        if (gameObject->HasComponent<TransformComponent>() && gameObject->HasComponent<RenderComponent>()) {
            const auto& transform = gameObject->GetComponent<TransformComponent>();
            const auto& render = gameObject->GetComponent<RenderComponent>();

            DrawModelEx(
                render.model,
                transform.position,
                { transform.rotation.x, transform.rotation.y, transform.rotation.z }, // TODO: 从四元数转化为旋转角度
                QuaternionToAxisAngle(transform.rotation, nullptr),
                transform.scale,
                render.tint
            );
        }
    }
    // TODO: debug
    DrawGrid(20, 10.0f);
}