#include "GameplayScreen.h"
#include "raylib.h"
#include "ScreenState.h"
#include "raymath.h"

GameplayScreen::GameplayScreen()
    : m_nextScreenState(static_cast<int>(ScreenStateID::NONE))
{
    m_world = std::make_unique<GameWorld>();
    m_renderer = std::make_unique<Renderer>();
    m_cameraManager = std::make_unique<CameraManager>();

    m_cameraManager->LoadConfig("assets/config/cameras.json");
    ConfigureRenderer();
}
GameplayScreen::~GameplayScreen() 
{
    OnExit();
}
void GameplayScreen::ConfigureRenderer() {
    m_renderer->ClearRenderViews();

    // 1. 主视图
    RenderView mainView;
    mainView.cameraName = "main_orbital";
    mainView.viewport = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() };
    m_renderer->AddRenderView(mainView);
    
    // 2.后视图
    RenderView rearView;
    rearView.cameraName = "rear_view";
    int viewWidth = GetScreenWidth() / 5;
    int viewHeight = GetScreenHeight() / 5;
    rearView.viewport = { (float)GetScreenWidth() - viewWidth - 20, (float)GetScreenHeight() - viewHeight - 20, (float)viewWidth, (float)viewHeight };
    rearView.clearBackground = true; // 不透明底
    rearView.backgroundColor = Fade(BLACK, 0.7f);
    m_renderer->AddRenderView(rearView);
}

// 当进入游戏场景时调用

#include "Engine/Core/Component/TransformComponent.h"
#include "Engine/Core/Component/RenderComponent.h"
void GameplayScreen::OnEnter() {
    auto& playerCube = m_world->CreateGameObject();

    // 2. 为它添加 TransformComponent，并设置初始位置
    playerCube.AddComponent<TransformComponent>(Vector3{ 0.0f, 0.5f, 0.0f });

    // 3. 为它添加 RenderComponent
    auto& renderComp = playerCube.AddComponent<RenderComponent>();
    
    // 4. 为 RenderComponent 创建一个模型
    // 使用 Raylib 的程序化生成功能来创建一个 1x1x1 的立方体
    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    renderComp.model = LoadModelFromMesh(cubeMesh);
    renderComp.tint = BLUE; // 设置颜色为蓝色

    // m_sceneManager->LoadScene("assets/scenes/earth_map.json");
}

// 当离开游戏场景时调用
void GameplayScreen::OnExit() {
    // m_sceneManager->UnloadCurrentScene();
}

// 在固定时间步更新（未来的物理和网络逻辑将在这里）
void GameplayScreen::FixedUpdate(float fixedDeltaTime) {
    // m_physicsSystem->Update(*m_world, fixedDeltaTime);
    // m_networkManager->Update(fixedDeltaTime);
}

void GameplayScreen::Update(float deltaTime) {
    m_nextScreenState = static_cast<int>(ScreenStateID::NONE);

    if (IsKeyPressed(KEY_ESCAPE)) {
        m_nextScreenState = static_cast<int>(ScreenStateID::MAIN_MENU);
    }
    // TODO:Camera更新
    if (auto* mainCam = m_cameraManager->GetMainCamera()) {
        UpdateCamera(mainCam, CAMERA_ORBITAL);
        
        if(auto* rearCam = m_cameraManager->GetCamera("rear_view")) {
            Vector3 mainPos = mainCam->position;
            Vector3 mainTarget = mainCam->target;
            // TODO:数学库替换
            Vector3 direction = Vector3Subtract(mainTarget, mainPos);
            direction = Vector3Normalize(direction);
            float rearDistance = 10.0f; 
            Vector3 rearPos = Vector3Add(mainPos, Vector3Scale(direction, -rearDistance));

            rearCam->position = rearPos;
            rearCam->target = mainPos; 
            
            UpdateCamera(rearCam, CAMERA_FREE);
        }
    }
    // m_scriptingSystem->Update(*m_world, deltaTime);
    m_world->DestroyWaitingObjects();
}

void GameplayScreen::Draw() {
    ClearBackground(RAYWHITE); // 设置一个浅灰色背景

    m_renderer->RenderScene(*m_world, *m_cameraManager);

    
    // 在3D内容之上绘制一些2D的调试信息
    DrawText("Welcome to the 3D World!", 10, 40, 20, DARKGRAY);
    DrawText("Press ESC to return.", 10, GetScreenHeight() - 30, 20, DARKGRAY);
    // m_worldRenderer->Draw(*m_world, *m_cameraManager);
    // m_uiManager->Draw(*m_world, *m_cameraManager);
}

// 向 ScreenManager 报告下一个状态
int GameplayScreen::GetNextScreenState() const {
    return m_nextScreenState;
}
int GameplayScreen::GetScreenState() const {
    return static_cast<int>(ScreenStateID::GAMEPlAY);
}