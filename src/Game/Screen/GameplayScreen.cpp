#include "GameplayScreen.h"
#include "raylib.h"
#include "Game/Screen/MyScreenState.h"
#include "Game/Systems/Physics/SolarStage.h"
#include "Game/Scripts/RotatorScript.h"
#include "raymath.h"
#include "Engine/Engine.h"
#include <iostream>
GameplayScreen::GameplayScreen()
    : m_nextScreenState(SCREEN_STATE_NONE)
{
    const std::string &cameraConfigPath = "assets/config/cameras_config.json";
    const std::string &sceneConfigPath = "assets/scenes/test_scene.json";
    const std::string &inputConfigPath = "assets/config/input_config.json";
    const std::string &renderViewConfigPath = "assets/view/test_view.json";
    m_world = std::make_unique<GameWorld>([this](ScriptingFactory &scriptingFactory, PhysicsStageFactory &physicsStageFactory)
                                          { this->ConfigCallback(scriptingFactory, physicsStageFactory); },
                                          cameraConfigPath,
                                          sceneConfigPath,
                                          inputConfigPath,
                                          renderViewConfigPath);
}
GameplayScreen::~GameplayScreen()
{
    OnExit();
}

void GameplayScreen::ConfigCallback(ScriptingFactory &scriptingFactory, PhysicsStageFactory &physicsStageFactory)
{
    // 注册后才可使用json配置
    physicsStageFactory.Register("SolarStage", []()
                                 { return std::make_unique<SolarStage>(); });
    physicsStageFactory.Register("CollisionStage", []()
                                 { return std::make_unique<CollisionStage>(); });
    scriptingFactory.Register("RotatorScript", []()
                              { return std::make_unique<RotatorScript>(); });
}

// 当进入游戏场景时调用
void GameplayScreen::OnEnter()
{
    DisableCursor();
}

// 当离开游戏场景时调用
void GameplayScreen::OnExit()
{
    EnableCursor();
    // m_sceneManager->UnloadCurrentScene();
}
// TODO:主循环

// 在固定时间步更新（未来的物理和网络逻辑将在这里）
void GameplayScreen::FixedUpdate(float fixedDeltaTime)
{
    // m_networkManager->Update(fixedDeltaTime);
    // auto *mainCam = m_cameraManager->GetMainCamera();
    // Vector3 mainPos = mainCam->position;
    // mainPos = Vector3Add(mainPos, Vector3Scale(mainCam->direction, 0.2f));

    // mainCam->UpdateFromDirection(mainPos, mainCam->direction, mainCam->up);

    m_world->FixedUpdate(fixedDeltaTime);
}

void GameplayScreen::Update(float deltaTime)
{
    m_nextScreenState = SCREEN_STATE_NONE;

    if (!m_world->Update(deltaTime))
        m_nextScreenState = MAIN_MENU;

    auto &m_inputManager = m_world->GetInputManager();
    auto &m_cameraManager = m_world->GetCameraManager();
    m_inputManager.Update();
    if (m_inputManager.IsActionPressed("Exit"))
    {
        m_nextScreenState = MAIN_MENU;
    }

    // TODO:Camera更新
    if (auto *mainCam = m_cameraManager.GetMainCamera())
    {
        Vector3f mainPos = mainCam->Position();

        if (m_inputManager.IsActionDown("Forward"))
        {
            DrawText("Forward!", 200, 200, 20, GREEN);
            mainPos += mainCam->Direction() * 0.3f;
        }
        if (m_inputManager.IsActionDown("Backward"))
        {
            DrawText("Backward!", 200, 200, 20, GREEN);
            mainPos -= mainCam->Direction() * 0.1f;
        }
        if (m_inputManager.IsActionDown("Left"))
        {
            DrawText("Left!", 200, 200, 20, GREEN);
            mainPos -= mainCam->Right() * 0.1f;
        }
        if (m_inputManager.IsActionDown("Right"))
        {
            DrawText("Right!", 200, 200, 20, GREEN);
            mainPos += mainCam->Right() * 0.1f;
        }
        mainCam->UpdateFromDirection(mainPos, mainCam->Direction(), mainCam->Up());

        float lookHorizontal = -m_inputManager.GetAxisValue("LookHorizontal") * PI / 180;
        float lookVertical = m_inputManager.GetAxisValue("LookVertical") * PI / 180;
        DrawText(TextFormat("LookHorizontal: %f", lookHorizontal), 200, 300, 20, GREEN);
        DrawText(TextFormat("LookVertical: %f", lookVertical), 200, 350, 20, GREEN);
        mainCam->Rotate(lookHorizontal, lookVertical);

        if (auto *rearCam = m_cameraManager.GetCamera("rear_view"))
        {
            Vector3f mainPos = mainCam->Position();
            Vector3f mainTarget = mainCam->Target();
            Vector3f direction = mainTarget - mainPos;
            direction.Normalize();

            rearCam->UpdateFromDirection(mainPos, -direction, mainCam->Up());
        }
    }
}

void GameplayScreen::Draw()
{
    ClearBackground(RAYWHITE); // 设置一个浅灰色背景

    // m_renderer->RenderScene(*m_world, *m_cameraManager);
    m_world->Render();
    // 在3D内容之上绘制一些2D的调试信息
    DrawText("Welcome to the 3D World!", 10, 40, 20, DARKGRAY);
    DrawText("Press ESC to return.", 10, GetScreenHeight() - 30, 20, DARKGRAY);
    // m_worldRenderer->Draw(*m_world, *m_cameraManager);
    // m_uiManager->Draw(*m_world, *m_cameraManager);
}

// 向 ScreenManager 报告下一个状态
ScreenState GameplayScreen::GetNextScreenState() const
{
    return m_nextScreenState;
}
ScreenState GameplayScreen::GetScreenState() const
{
    return GAMEPLAY;
}