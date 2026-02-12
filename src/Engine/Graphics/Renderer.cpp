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

void Renderer::Init(const std::string &configViewPath, GameWorld &gameWorld)
{
    m_postProcesser = std::make_unique<PostProcesser>();
    m_renderViewer = std::make_unique<RenderViewer>();
    this->LoadViewConfig(configViewPath, gameWorld);
}

bool Renderer::LoadViewConfig(const std::string &configPath, GameWorld &gameWorld)
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
            m_renderViewer->ParseViewConfig(data["views"]);
        if (data.contains("postProcess"))
            m_postProcesser->ParsePostProcessPasses(data["postProcess"], gameWorld);
        return true;
    }
    catch (std::exception &e)
    {
        std::cerr << "[Renderer]: Failed to parse view config file: " << configPath << std::endl;
        std::cerr << e.what() << std::endl;
        return false;
    }
}

#include <utility>
#include "rlgl.h"

#if defined(PLATFORM_WEB)
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#else
#include "external/glad.h"
#endif
Renderer::Renderer()
{
    // if (m_dummyDepth.id > 0)
    //     rlUnloadTexture(m_dummyDepth.id);
    // m_dummyDepth.id = rlLoadTextureDepth(GetScreenWidth(), GetScreenHeight(), false);
    // m_dummyDepth.width = GetScreenWidth();
    // m_dummyDepth.height = GetScreenHeight();
    // m_dummyDepth.mipmaps = 1;
    // m_dummyDepth.format = 19;
    // rlTextureParameters(m_dummyDepth.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_NEAREST);
    // rlTextureParameters(m_dummyDepth.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_NEAREST);
}
Renderer::~Renderer()
{
    // rlUnloadTexture(m_dummyDepth.id);
}
// void Renderer::CopyDepthBuffer(RenderTexture2D sourceRT, Texture2D targetDepth)
// {
//     if (targetDepth.id == 0)
//         return;
//     glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceRT.id);
//     glActiveTexture(GL_TEXTURE0);
//     glBindTexture(GL_TEXTURE_2D, targetDepth.id);
//     glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 0, 0, targetDepth.width, targetDepth.height, 0);

//     glBindTexture(GL_TEXTURE_2D, 0);
//     glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
// }
void Renderer::RawRenderScene(GameWorld &gameWorld, CameraManager &cameraManager)
{

    auto &m_RTPool = m_postProcesser->GetRTPool();
    auto &itScene = m_RTPool["inScreen"];
    BeginTextureMode(itScene);
    {

        rlEnableDepthMask();
        // ClearBackground(BLUE);
        for (const auto &view : m_renderViewer->GetRenderViews())
        {
            mCamera *camera = cameraManager.GetCamera(view.cameraName);
            if (camera)
            {

                int x1 = (int)view.viewport.x;
                int y1 = (int)view.viewport.y;
                int x2 = (int)view.viewport.width;
                int y2 = (int)view.viewport.height;

                int vx = x1;
                int vy = y1;
                int vw = x2 - x1;
                int vh = y2 - y1;

                BeginScissorMode(vx, vy, vw, vh); //  透明底？
                if (view.clearBackground)
                {
                    ClearBackground(view.backgroundColor);
                }
                float aspect = (float)vw / (float)vh;

                Camera3D rawCamera = camera->GetRawCamera();
                BeginMode3D(rawCamera);

                rlViewport(vx, itScene.texture.height - (vy + vh), vw, vh);

                rlMatrixMode(RL_PROJECTION);
                rlLoadIdentity();
                if (rawCamera.projection == CAMERA_PERSPECTIVE)
                {
                    double top = 0.01 * tan(rawCamera.fovy * 0.5 * DEG2RAD);
                    double right = top * aspect;
                    rlFrustum(-right, right, -top, top, 0.01, 1000.0);
                }
                else
                {
                    float top = rawCamera.fovy * 0.5f;
                    float right = top * aspect;
                    rlOrtho(-right, right, -top, top, 0.01, 1000.0);
                }
                rlMatrixMode(RL_MODELVIEW);

                DrawWorldObjects(gameWorld, rawCamera, *camera, aspect);

                // debug
                // for (const auto &view1 : m_renderViewer->GetRenderViews())
                // {
                //     if (view1.cameraName != view.cameraName)
                //     {
                //         mCamera *camera = cameraManager.GetCamera(view1.cameraName);
                //         if (camera->GetMountTarget() != nullptr)
                //             DrawVector(camera->getPosition(), camera->getDirection(), 1.0f, 0.05f);
                //     }
                // }
                EndMode3D();

                // （debug）为视口绘制边框
                // DrawRectangleLinesEx(view.viewport, 2, GRAY);
                EndScissorMode();
            }
        }
        rlViewport(0, 0, itScene.texture.width, itScene.texture.height);
    }
    EndTextureMode();
}
void Renderer::RawRenderParticle(GameWorld &gameWorld, CameraManager &cameraManager)
{
    // rlDisableDepthMask();
    // 粒子的BeginTextureMode在每个emitter，每个emitter有自己的输出RT，最后到后处理中处理
    for (const auto &view : m_renderViewer->GetRenderViews())
    {
        mCamera *camera = cameraManager.GetCamera(view.cameraName);
        if (camera)
        {
            BeginScissorMode((int)view.viewport.x, (int)view.viewport.y, (int)view.viewport.width, (int)view.viewport.height);

            Camera3D rawCamera = camera->GetRawCamera();
            BeginMode3D(rawCamera);

            DrawParticle(gameWorld, *camera, view.viewport.width / view.viewport.height);

            EndMode3D();
            EndScissorMode();
        }
    }
}

void Renderer::RenderScene(GameWorld &gameWorld, CameraManager &cameraManager)
{
    // RT图出入口
    auto &m_RTPool = m_postProcesser->GetRTPool();
    for (auto &[name, rt] : m_RTPool)
    {
        BeginTextureMode(rt);
        ClearBackground(BLACK);
        EndTextureMode();
    }
    bool isPosetProcess = true;
    if (m_RTPool.empty())
    {
        isPosetProcess = false;
        m_postProcesser->DefaultSetup();
    }

    auto &itScene = m_RTPool.find("inScreen");
    auto &itFinal = m_RTPool.find("outScreen");
    if (itScene == m_RTPool.end())
    {
        std::cerr << "[Renderer]: No inScreen render target found!!!" << std::endl;
        return;
    }
    if (itFinal == m_RTPool.end())
    {
        std::cerr << "[Renderer]: No outScreen render target found!!!" << std::endl;
        return;
    }

    RawRenderScene(gameWorld, cameraManager);
    RawRenderParticle(gameWorld, cameraManager);

    m_postProcesser->PostProcess(gameWorld, cameraManager);

    // 最终输出
    ClearBackground(BLACK);
    if (!isPosetProcess)
        itFinal->second.texture = itScene->second.texture;

    Rectangle src = {0, 0, (float)itFinal->second.texture.width, (float)-itFinal->second.texture.height};
    DrawTexturePro(itFinal->second.texture, src,
                   {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                   {0, 0}, 0, WHITE);
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
            Quat4f rotation = tf.GetWorldRotation();
            Vector3f axis = rotation.getAxisAngle(&angle);
            angle *= (float)180.0f / (float)M_PI;

            bool useShader = (render.defaultMaterial.shader != nullptr && render.defaultMaterial.shader->IsValid());
            if (useShader)
            {

                Matrix4f S = Matrix4f(Matrix3f(tf.GetWorldScale() & render.scale));
                Matrix4f R = Matrix4f(tf.GetWorldRotation().toMatrix());
                Matrix4f T = Matrix4f::translation(tf.GetWorldPosition());
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
                Color tint = {(unsigned char)render.defaultMaterial.baseColor.x(), (unsigned char)render.defaultMaterial.baseColor.y(), (unsigned char)render.defaultMaterial.baseColor.z(), (unsigned char)render.defaultMaterial.baseColor.w()};
                DrawModelEx(
                    render.model,
                    tf.GetWorldPosition(),
                    axis,
                    angle,
                    tf.GetWorldScale() & render.scale,
                    tint);
            }
            if (render.showWires)
                DrawModelWiresEx(
                    render.model,
                    tf.GetWorldPosition(),
                    axis,
                    angle,
                    tf.GetWorldScale() & render.scale,
                    BLACK);

            if (render.showAxes)
                DrawCoordinateAxes(tf.GetWorldPosition(), tf.GetWorldRotation(), 2.0f, 0.05f);
            if (render.showCenter)
                DrawSphereEx(tf.GetWorldPosition(), 0.1f, 8, 8, RED);
            if (render.showAngVol && gameObject->HasComponent<RigidbodyComponent>())
            {
                const auto &rb = gameObject->GetComponent<RigidbodyComponent>();
                DrawVector(tf.GetWorldPosition(), rb.angularVelocity, 1.0f, 0.05f);
            }
            if (render.showVol && gameObject->HasComponent<RigidbodyComponent>())
            {
                const auto &rb = gameObject->GetComponent<RigidbodyComponent>();
                DrawVector(tf.GetWorldPosition(), rb.velocity, 1.0f, 0.05f);
            }
        }
    }
    // TODO: debug
    DrawGrid(20, 10.0f);

    DrawCoordinateAxes(Vector3f(0.0f), Quat4f::IDENTITY, 2.0f, 0.05f);
}

void Renderer::RenderSinglePass(const Mesh &mesh, const Model &model, const int &meshIdx, const RenderMaterial &pass, const Matrix4f &MVP, const Matrix4f &M, const mCamera &camera, GameWorld &gameWorld)
{
    rlDrawRenderBatchActive();

    rlEnableDepthTest();
    rlEnableDepthMask();
    rlDisableBackfaceCulling();
    rlSetCullFace(RL_CULL_FACE_BACK);
    rlEnableColorBlend();
    float gameTime = gameWorld.GetTimeManager().GetGameTime();
    float realTime = gameWorld.GetTimeManager().GetRealTime();

    if (pass.shader != nullptr && pass.shader->IsValid())
    {
        int matIdex = model.meshMaterial[meshIdx];
        Material tempRaylibMaterial = model.materials[matIdex];

        pass.shader->Begin();

        switch (pass.blendMode)
        {
        case BLEND_OPIQUE:
            rlDisableColorBlend();
            break;
        case BLEND_MULTIPLIED:
            rlSetBlendMode(BLEND_CUSTOM);
            rlSetBlendFactors(RL_DST_COLOR, RL_ZERO, RL_FUNC_ADD);
            break;
        case BLEND_SCREEN:
            rlSetBlendMode(BLEND_CUSTOM);
            rlSetBlendFactors(RL_ONE, RL_ONE_MINUS_SRC_COLOR, RL_FUNC_ADD);
            break;
        case BLEND_SUBTRACT:
            rlSetBlendMode(BLEND_CUSTOM);
            rlSetBlendFactors(RL_ONE, RL_ONE, RL_FUNC_REVERSE_SUBTRACT);
            break;
        default:
            BeginBlendMode(pass.blendMode);
            break;
        }

        pass.shader->SetAll(MVP, M, camera.Position(), realTime, gameTime, pass.baseColor, pass.customFloats, pass.customVector2, pass.customVector3, pass.customVector4);

        tempRaylibMaterial.shader = pass.shader->GetShader();

        int texUnit = 1;
        if (pass.useDiffuseMap)
        {
            pass.shader->SetTexture("u_diffuseMap", pass.diffuseMap, texUnit);
            if (pass.diffuseIsAnimated)
            {
                pass.shader->SetInt("u_diffuseMap_frameCount", pass.diffuseframeCount);
                pass.shader->SetFloat("u_diffuseMap_animSpeed", pass.diffuseanimSpeed);
            }
            else
            {
                pass.shader->SetInt("u_diffuseMap_frameCount", 1);
                pass.shader->SetFloat("u_diffuseMap_animSpeed", 0.0f);
            }
            tempRaylibMaterial.maps[MATERIAL_MAP_DIFFUSE].texture = pass.diffuseMap;
            texUnit++;
        }
        for (auto const &[name, text] : pass.customTextures)
        {
            pass.shader->SetTexture(name, text, texUnit);
            if (pass.isAnimated.at(name))
            {
                pass.shader->SetInt(name + "_frameCount", pass.frameCount.at(name));
                pass.shader->SetFloat(name + "_animSpeed", pass.animSpeed.at(name));
            }
            else
            {
                pass.shader->SetInt(name + "_frameCount", 1);
                pass.shader->SetFloat(name + "_animSpeed", 0.0f);
            }
            texUnit++;
        }

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
        rlDrawRenderBatchActive();
        pass.shader->End();
        EndBlendMode();
    }
    rlEnableColorBlend();
    rlEnableDepthTest();
    rlEnableDepthMask();
    rlDisableBackfaceCulling();
    rlSetCullFace(RL_CULL_FACE_BACK);
}

void Renderer::DrawParticle(GameWorld &gameWorld, mCamera &camera, float aspect)
{
    auto &m_RTPool = m_postProcesser->GetRTPool();

    auto &particleSys = gameWorld.GetParticleSystem();
    float gameTime = gameWorld.GetTimeManager().GetGameTime();
    float realTime = gameWorld.GetTimeManager().GetRealTime();
    Camera3D rawCamera = camera.GetRawCamera();
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
    particleSys.Render(m_RTPool, realTime, gameTime, VP, matProj, gameWorld, camera);
}
// Debug
#include <iostream>
void Renderer::DrawCoordinateAxes(Vector3f position, Quat4f rotation, float axisLength, float thickness)
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

void Renderer::DrawVector(Vector3f position, Vector3f direction, float axisLength, float thickness)
{

    int sides = 8;                               // 圆柱体面数，8面够圆了，太多影响性能
    float coneHeight = axisLength * 0.2f;        // 箭头长度占总长的 20%
    float cylinderLen = axisLength - coneHeight; // 剩余部分是圆柱体
    float coneRadius = thickness * 2.5f;         // 箭头底部半径比轴粗一些

    Vector3f end = position + direction * cylinderLen;
    Vector3f tip = position + direction * axisLength;
    DrawCylinderEx(position, end, thickness, thickness, sides, RED);
    DrawCylinderEx(end, tip, coneRadius, 0.0f, sides, WHITE);
}