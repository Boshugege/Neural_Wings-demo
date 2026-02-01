#include "Renderer.h"
#include "Engine/Core/GameWorld.h"
#include "Camera/CameraManager.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/Utils/JsonParser.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
using json = nlohmann::json;

#define M_PI 3.14159265358979323846

RenderView Renderer::ParseViews(const json &viewData)
{
    RenderView view;
    if (viewData.contains("name"))
        // 与camerajson中名字一致
        view.cameraName = viewData["name"];
    else
    {
        std::cerr << "[Renderer]: View config file missing 'name' field" << std::endl;
        return view;
    }
    if (viewData.contains("viewport"))
        view.viewport = JsonParser::ToRectangle(viewData["viewport"]);
    else
        view.viewport = Rectangle{0.0, 0.0, (float)GetScreenHeight(), (float)GetScreenWidth()};

    view.clearBackground = viewData.value("clearBackground", false);
    if (viewData.contains("backgroundColor"))
        view.backgroundColor = JsonParser::ToColor(viewData["backgroundColor"]);
    else
        view.backgroundColor = WHITE;
    return view;
}

bool Renderer::LoadViewConfig(const std::string &configPath)
{
    std::ifstream configFile(configPath);
    if (!configFile.is_open())
    {
        std::cerr << "[Renderer]: Failed to open view config file: " << configPath << std::endl;
        return false;
    }
    try
    {
        json data = json::parse(configFile);
        if (data.contains("views"))
        {
            this->ClearRenderViews();
            for (const auto &viewData : data["views"])
            {
                RenderView view = ParseViews(viewData);
                this->AddRenderView(view);
            }
        }
        return true;
    }
    catch (std::exception &e)
    {
        std::cerr << "[Renderer]: Failed to parse view config file: " << configPath << std::endl;
        std::cerr << e.what() << std::endl;
        return false;
    }
}

void Renderer::AddRenderView(const RenderView &view)
{
    m_renderViews.push_back(view);
}

void Renderer::ClearRenderViews()
{
    m_renderViews.clear();
}

void Renderer::RenderScene(GameWorld &gameWorld, CameraManager &cameraManager)
{
    for (const auto &view : m_renderViews)
    {
        mCamera *camera = cameraManager.GetCamera(view.cameraName);
        if (camera)
        {
            BeginScissorMode(view.viewport.x, view.viewport.y, view.viewport.width, view.viewport.height);
            // 透明底？
            if (view.clearBackground)
            {
                ClearBackground(view.backgroundColor);
            }
            Camera3D rawCamera = camera->GetRawCamera();
            BeginMode3D(rawCamera);
            DrawWorldObjects(gameWorld, rawCamera, *camera, view.viewport.width / view.viewport.height);
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
void Renderer::DrawWorldObjects(GameWorld &world, Camera3D &rawCamera, mCamera &camera, float aspect)
{
    Matrix4f matView = GetCameraMatrix(rawCamera);
    Matrix4f matProj;
    if (rawCamera.projection == CAMERA_PERSPECTIVE)
    {
        matProj = MatrixPerspective(rawCamera.fovy * M_PI / 180.0f, aspect, camera.getNearPlane(), camera.getFarPlane());
    }
    else
    {
        float top = rawCamera.fovy * 0.5f;
        float right = top * aspect;
        matProj = MatrixOrtho(-right, right, -top, top, camera.getNearPlane(), camera.getFarPlane());
    }
    Matrix4f VP = matProj * matView;

    for (const auto &gameObject : world.GetGameObjects())
    {
        if (gameObject->HasComponent<TransformComponent>() && gameObject->HasComponent<RenderComponent>())
        {
            const auto &tf = gameObject->GetComponent<TransformComponent>();
            const auto &render = gameObject->GetComponent<RenderComponent>();

            float angle = 0.0f;
            Quat4f rotation = tf.rotation;
            Vector3f axis = rotation.getAxisAngle(&angle);
            angle *= 180.0f / M_PI;

            bool useShader = (render.defaultMaterial.shader != nullptr && render.defaultMaterial.shader->IsValid());
            if (useShader)
            {

                Matrix4f S = Matrix4f(Matrix3f(tf.scale & render.scale));
                Matrix4f R = Matrix4f(tf.rotation.toMatrix());
                Matrix4f T = Matrix4f::translation(tf.position);
                Matrix4f M = T * R * S;

                Matrix4f MVP = VP * M;

                // 同模型各个mesh的passes
                for (int i = 0; i < render.model.meshCount; i++)
                {
                    Mesh &mesh = render.model.meshes[i];
                    const std::vector<RenderMaterial> *passes = nullptr;
                    auto it = render.meshPasses.find(i);
                    if (it != render.meshPasses.end())
                    {
                        passes = &it->second;
                    }
                    if (passes != nullptr && !passes->empty())
                    {
                        // 单mesh多pass
                        for (size_t p = 0; p < passes->size(); p++)
                        {
                            const RenderMaterial &pass = (*passes)[p];

                            RenderSinglePass(mesh, render.model, i, pass, MVP, M, camera, world);
                        }
                    }
                    else
                    {
                        RenderSinglePass(mesh, render.model, i, render.defaultMaterial, MVP, M, camera, world);
                    }
                }
            }
            else
            {
                Color tint = {render.defaultMaterial.baseColor.x(), render.defaultMaterial.baseColor.y(), render.defaultMaterial.baseColor.z(), render.defaultMaterial.baseColor.w()};
                DrawModelEx(
                    render.model,
                    tf.position,
                    axis,
                    angle,
                    tf.scale & render.scale,
                    tint);
            }
            if (render.showWires)
                DrawModelWiresEx(
                    render.model,
                    tf.position,
                    axis,
                    angle,
                    tf.scale & render.scale,
                    BLACK);

            if (render.showAxes)
                DrawCoordinateAxes(tf.position, tf.rotation, 2.0f, 0.05f);
            if (render.showCenter)
                DrawSphereEx(tf.position, 0.1f, 8, 8, RED);
            if (render.showAngVol && gameObject->HasComponent<RigidbodyComponent>())
            {
                const auto &rb = gameObject->GetComponent<RigidbodyComponent>();
                DrawVector(tf.position, rb.angularVelocity, 1.0f, 0.05f);
            }
            if (render.showVol && gameObject->HasComponent<RigidbodyComponent>())
            {
                const auto &rb = gameObject->GetComponent<RigidbodyComponent>();
                DrawVector(tf.position, rb.velocity, 1.0f, 0.05f);
            }
        }
    }
    // TODO: debug
    DrawGrid(20, 10.0f);
    DrawCoordinateAxes(Vector3f(0.0f), Quat4f::IDENTITY, 2.0f, 0.05f);
}

void Renderer::RenderSinglePass(const Mesh &mesh, const Model &model, const int &meshIdx, const RenderMaterial &pass, const Matrix4f &MVP, const Matrix4f &M, const mCamera &camera, GameWorld &gameWorld)
{
    rlEnableDepthTest();
    rlEnableDepthMask();
    rlDisableBackfaceCulling();
    rlSetCullFace(RL_CULL_FACE_BACK);
    BeginBlendMode(BLEND_ALPHA);

    float gameTime = gameWorld.GetTimeManager().GetGameTime();
    float realTime = gameWorld.GetTimeManager().GetRealTime();

    if (pass.shader != nullptr && pass.shader->IsValid())
    {
        int matIdex = model.meshMaterial[meshIdx];
        Material tempRaylibMaterial = model.materials[matIdex];

        pass.shader->Begin();
        if (pass.blendMode == -1)
        {
            rlSetBlendMode(BLEND_CUSTOM);
            rlSetBlendFactors(RL_ONE, RL_ZERO, RL_FUNC_ADD);
        }
        else
            BeginBlendMode(pass.blendMode);

        pass.shader->SetMat4("u_mvp", MVP);
        pass.shader->SetMat4("transform", M);
        pass.shader->SetVec3("viewPos", camera.Position());
        pass.shader->SetFloat("realTime", realTime);
        pass.shader->SetFloat("gameTime", gameTime);
        Vector4f color = pass.baseColor / 255.0f;
        pass.shader->SetVec4("baseColor", color);

        for (auto const &[name, value] : pass.customFloats)
            pass.shader->SetFloat(name, value);
        for (auto const &[name, value] : pass.customVector3)
            pass.shader->SetVec3(name, value);
        for (auto const &[name, value] : pass.customVector4)
            pass.shader->SetVec4(name, value);

        tempRaylibMaterial.shader = pass.shader->GetShader();

        if (pass.cullFace >= 0)
        {
            rlEnableBackfaceCulling();
            rlSetCullFace(pass.cullFace);
        }
        if (!pass.depthTest)
            rlDisableDepthTest();
        if (!pass.depthWrite)
            rlDisableDepthMask();

        DrawMesh(mesh, tempRaylibMaterial, M);

        pass.shader->End();

        EndBlendMode();
        rlEnableDepthTest();
        rlEnableDepthMask();
        rlDisableBackfaceCulling();
        rlSetCullFace(RL_CULL_FACE_BACK);
    }
}