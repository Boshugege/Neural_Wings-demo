#include "Renderer.h"
#include "Engine/Core/GameWorld.h"
#include "CameraManager.h"
#include "Engine/Core/Components/TransformComponent.h"
#include "Engine/Core/Components/RenderComponent.h"
#include "Engine/Core/Components/RigidBodyComponent.h"
#define M_PI 3.14159265358979323846

void Renderer::AddRenderView(const RenderView &view)
{
    m_renderViews.push_back(view);
}

void Renderer::ClearRenderViews()
{
    m_renderViews.clear();
}

void Renderer::RenderScene(const GameWorld &world, CameraManager &cameraManager)
{
    for (const auto &view : m_renderViews)
    {
        Camera3D *camera = cameraManager.GetCamera(view.cameraName);
        if (camera)
        {
            BeginScissorMode(view.viewport.x, view.viewport.y, view.viewport.width, view.viewport.height);
            // 透明底？
            if (view.clearBackground)
            {
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

#include <iostream>
void DrawCoordinateAxes(Vector3f position, Quat4f rotation, float axisLength, float thickness)
{
    // 定义基准轴
    Vector3f baseRight = {1.0f, 0.0f, 0.0f};
    Vector3f baseUp = {0.0f, 1.0f, 0.0f};
    Vector3f baseForward = {0.0f, 0.0f, 1.0f};

    // 计算旋转后的局部轴方向
    // 如果你使用了 raymath.h，也可以直接用 Vector3RotateByQuaternion(baseRight, rotation)
    Vector3f localRight = rotation * baseRight;
    Vector3f localUp = rotation * baseUp;
    Vector3f localForward = rotation * baseForward;

    // 预计算一些常量
    int sides = 8;                               // 圆柱体面数，8面够圆了，太多影响性能
    float coneHeight = axisLength * 0.2f;        // 箭头长度占总长的 20%
    float cylinderLen = axisLength - coneHeight; // 剩余部分是圆柱体
    float coneRadius = thickness * 2.5f;         // 箭头底部半径比轴粗一些

    // === X 轴 (红色) ===
    Vector3f endX = position + localRight * cylinderLen;
    Vector3f tipX = position + localRight * axisLength;
    DrawCylinderEx(position, endX, thickness, thickness, sides, RED); // 轴身
    DrawCylinderEx(endX, tipX, coneRadius, 0.0f, sides, RED);         // 箭头

    // === Y 轴 (绿色) ===
    Vector3f endY = position + localUp * cylinderLen;
    Vector3f tipY = position + localUp * axisLength;
    DrawCylinderEx(position, endY, thickness, thickness, sides, GREEN);
    DrawCylinderEx(endY, tipY, coneRadius, 0.0f, sides, GREEN);

    // === Z 轴 (蓝色) ===
    Vector3f endZ = position + localForward * cylinderLen;
    Vector3f tipZ = position + localForward * axisLength;
    DrawCylinderEx(position, endZ, thickness, thickness, sides, BLUE);
    DrawCylinderEx(endZ, tipZ, coneRadius, 0.0f, sides, BLUE);
}

void DrawVector(Vector3f position, Vector3f direction, float axisLength, float thickness)
{

    int sides = 8;                               // 圆柱体面数，8面够圆了，太多影响性能
    float coneHeight = axisLength * 0.2f;        // 箭头长度占总长的 20%
    float cylinderLen = axisLength - coneHeight; // 剩余部分是圆柱体
    float coneRadius = thickness * 2.5f;         // 箭头底部半径比轴粗一些

    Vector3f end = position + direction * cylinderLen;
    Vector3f tip = position + direction * axisLength;
    DrawCylinderEx(position, end, thickness, thickness, sides, BLUE);
    DrawCylinderEx(end, tip, coneRadius, 0.0f, sides, BLACK);
}
void Renderer::DrawWorldObjects(const GameWorld &world)
{
    for (const auto &gameObject : world.GetGameObjects())
    {
        if (gameObject->HasComponent<TransformComponent>() && gameObject->HasComponent<RenderComponent>())
        {
            const auto &transform = gameObject->GetComponent<TransformComponent>();
            const auto &render = gameObject->GetComponent<RenderComponent>();

            float angle = 0.0f;
            Quat4f rotation = transform.rotation;
            Vector3f axis = rotation.getAxisAngle(&angle);
            angle *= 180.0f / M_PI;
            DrawModelEx(
                render.model,
                transform.position,
                axis,
                angle,
                transform.scale,
                render.tint);

            // TODO: debug
            DrawCoordinateAxes(transform.position, transform.rotation, 2.0f, 0.05f);
            if(gameObject->HasComponent<RigidbodyComponent>())
            {
                const auto &rb = gameObject->GetComponent<RigidbodyComponent>();
                DrawVector(transform.position, rb.angularVelocity, 1.0f, 0.05f);
            }
        }
    }
    // TODO: debug
    DrawGrid(20, 10.0f);
    DrawCoordinateAxes(Vector3f(0.0f), Quat4f::IDENTITY, 2.0f, 0.05f);
}