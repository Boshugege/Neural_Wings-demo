#include "GameplayScreen.h"
#include "raylib.h"
#include "ScreenState.h"
#include "raymath.h"

#include <iostream>
GameplayScreen::GameplayScreen()
    : m_nextScreenState(static_cast<int>(ScreenStateID::NONE))
{
    m_world = std::make_unique<GameWorld>();
    m_renderer = std::make_unique<Renderer>();
    m_cameraManager = std::make_unique<CameraManager>();
    m_inputManager = std::make_unique<InputManager>();
    m_physicsSystem= std::make_unique<PhysicsSystem>();

    m_cameraManager->LoadConfig("assets/config/cameras_config.json");
    ConfigureRenderer();

    if (!m_inputManager->LoadBindings("assets/config/input_config.json"))
    {
        std::cerr << "Error: [GameplayScreen] Could not load input bindings.Use default bindings instead." << std::endl;
        m_inputManager->LoadBindings("assets/config/default/input_config.json");
    }
}
GameplayScreen::~GameplayScreen()
{
    OnExit();
}
void GameplayScreen::ConfigureRenderer()
{
    m_renderer->ClearRenderViews();

    // 1. 主视图
    RenderView mainView;
    mainView.cameraName = "main_orbital";
    mainView.viewport = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
    m_renderer->AddRenderView(mainView);

    // 2.后视图
    RenderView rearView;
    rearView.cameraName = "rear_view";
    int viewWidth = GetScreenWidth() / 5;
    int viewHeight = GetScreenHeight() / 5;
    rearView.viewport = {(float)GetScreenWidth() - viewWidth - 20, (float)GetScreenHeight() - viewHeight - 20, (float)viewWidth, (float)viewHeight};
    rearView.clearBackground = true; // 不透明底
    rearView.backgroundColor = Fade(BLACK, 0.7f);
    m_renderer->AddRenderView(rearView);
}

// 当进入游戏场景时调用
#include "Engine/Core/Components/Components.h"
#include "Game/Systems/Physics/GravityStage.h"
#include "Game/Systems/Physics/SolarStage.h"
void GameplayScreen::OnEnter()
{
    DisableCursor();
    m_physicsSystem->AddStage(std::make_unique<SolarStage>(10.0f));
    
    GameObject *Cube = &m_world->CreateGameObject();

    // 2. 为它添加 TransformComponent，并设置初始位置
    Cube->AddComponent<TransformComponent>(Vector3{5.0f, 0.0f, 0.0f});
    RigidbodyComponent*  rbPtr=&Cube->AddComponent<RigidbodyComponent>();
    rbPtr->mass=100.0f;
    rbPtr->drag=0.1f;
    rbPtr->velocity=Vector3{0.0f,0.0f,10.0f};

    // 3. 为它添加 RenderComponent
    RenderComponent* rdPtr = &Cube->AddComponent<RenderComponent>();

    // 4. 为 RenderComponent 创建一个模型
    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    rdPtr->model = LoadModelFromMesh(cubeMesh);
    rdPtr->tint = BLUE;


    Cube = &m_world->CreateGameObject();

    Cube->AddComponent<TransformComponent>(Vector3{-5.0f, 0.0f, 0.0f});
    rbPtr=&Cube->AddComponent<RigidbodyComponent>();
    rbPtr->mass=100.0f;
    rbPtr->drag=0.1f;
    rbPtr->velocity=Vector3{0.0f,0.0f,-10.0f};

    rdPtr = &Cube->AddComponent<RenderComponent>();

    rdPtr->model = LoadModelFromMesh(cubeMesh);
    rdPtr->tint = RED; 

    
    Cube = &m_world->CreateGameObject();

    Cube->AddComponent<TransformComponent>(Vector3{0.0f, 5.0f, 0.0f});
    rbPtr=&Cube->AddComponent<RigidbodyComponent>();
    rbPtr->mass=10.0f;
    rbPtr->drag=0.1f;
    rbPtr->velocity=Vector3{3.0f,0.0f,-1.0f};

    rdPtr = &Cube->AddComponent<RenderComponent>();

    rdPtr->model = LoadModelFromMesh(cubeMesh);
    rdPtr->tint = BLACK; 


    // m_sceneManager->LoadScene("assets/scenes/earth_map.json");
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
    m_physicsSystem->Update(*m_world, fixedDeltaTime);
}

void GameplayScreen::Update(float deltaTime)
{
    m_nextScreenState = static_cast<int>(ScreenStateID::NONE);
    m_inputManager->Update();
    if (m_inputManager->IsActionPressed("Exit"))
    {
        m_nextScreenState = static_cast<int>(ScreenStateID::MAIN_MENU);
    }

    // TODO:Camera更新
    if (auto *mainCam = m_cameraManager->GetMainCamera())
    {
        Vector3 mainPos = mainCam->position;

        if (m_inputManager->IsActionDown("Forward"))
        {
            DrawText("Forward!", 200, 200, 20, GREEN);
            mainPos = Vector3Add(mainPos, Vector3Scale(mainCam->direction, 0.3f));
        }
        if (m_inputManager->IsActionDown("Backward"))
        {
            DrawText("Backward!", 200, 200, 20, GREEN);
            mainPos = Vector3Subtract(mainPos, Vector3Scale(mainCam->direction, 0.1f));
        }
        if (m_inputManager->IsActionDown("Left"))
        {
            DrawText("Left!", 200, 200, 20, GREEN);
            mainPos = Vector3Subtract(mainPos, Vector3Scale(mainCam->right, 0.1f));
        }
        if (m_inputManager->IsActionDown("Right"))
        {
            DrawText("Right!", 200, 200, 20, GREEN);
            mainPos = Vector3Add(mainPos, Vector3Scale(mainCam->right, 0.1f));
        }
        mainCam->UpdateFromDirection(mainPos, mainCam->direction, mainCam->up);

        float lookHorizontal = -m_inputManager->GetAxisValue("LookHorizontal")*PI/180;
        float lookVertical = m_inputManager->GetAxisValue("LookVertical")*PI/180;
        DrawText(TextFormat("LookHorizontal: %f", lookHorizontal), 200, 300, 20, GREEN);
        DrawText(TextFormat("LookVertical: %f", lookVertical), 200, 350, 20, GREEN);
        mainCam->Rotate(lookHorizontal, lookVertical);

        if (auto *rearCam = m_cameraManager->GetCamera("rear_view"))
        {
            Vector3 mainPos = mainCam->position;
            Vector3 mainTarget = mainCam->target;
            Vector3 direction = Vector3Subtract(mainTarget, mainPos);
            direction = Vector3Normalize(Vector3Zero()-direction);

            rearCam->UpdateFromDirection(mainPos, direction, mainCam->up);
        }
    }
    // m_scriptingSystem->Update(*m_world, deltaTime);
    m_world->DestroyWaitingObjects();
}

void GameplayScreen::Draw()
{
    ClearBackground(RAYWHITE); // 设置一个浅灰色背景

    m_renderer->RenderScene(*m_world, *m_cameraManager);

    // 在3D内容之上绘制一些2D的调试信息
    DrawText("Welcome to the 3D World!", 10, 40, 20, DARKGRAY);
    DrawText("Press ESC to return.", 10, GetScreenHeight() - 30, 20, DARKGRAY);
    // m_worldRenderer->Draw(*m_world, *m_cameraManager);
    // m_uiManager->Draw(*m_world, *m_cameraManager);
}

// 向 ScreenManager 报告下一个状态
int GameplayScreen::GetNextScreenState() const
{
    return m_nextScreenState;
}
int GameplayScreen::GetScreenState() const
{
    return static_cast<int>(ScreenStateID::GAMEPlAY);
}