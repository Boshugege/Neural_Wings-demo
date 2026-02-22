#include "GameplayScreen.h"
#include "raylib.h"
#include "Game/Screen/MyScreenState.h"
#include "Game/HUD/ChatHud.h"
#include "Game/HUD/EntityPlateHud.h"
#include "Game/HUD/MyHudState.h"
#include "Game/Systems/Physics/SolarStage.h"
#include "Game/Systems/Physics/NetworkVerifyStage.h"
#include "Game/Systems/Particles/Initializers/RandomLife.h"
#include "Game/Systems/Particles/Initializers/SphereDir.h"
#include "Game/Systems/Particles/Initializers/CollisionInit.h"
#include "Game/Systems/Particles/Initializers/SPHInit.h"
#include "Game/Scripts/Scripts.h"
#include "Engine/System/HUD/HudFactory.h"
#include "Engine/System/HUD/HudManager.h"

#include "raymath.h"
#include "Engine/Engine.h"
#include <iostream>

GameplayScreen::GameplayScreen(ScreenManager *sm)
    : m_nextScreenState(SCREEN_STATE_NONE), IGameScreen(sm)
{

    const std::string &cameraConfigPath = "assets/config/cameras_config.json";
    const std::string &sceneConfigPath = "assets/scenes/test_scene.json";
    const std::string &inputConfigPath = "assets/config/input_config.json";
    const std::string &renderViewConfigPath = "assets/view/test_view.json";
    const std::string &effectLibPath = "assets/Library/particle_effects.json";
    m_world = std::make_unique<GameWorld>([this](ScriptingFactory &scriptingFactory, PhysicsStageFactory &physicsStageFactory, ParticleFactory &particleFactory)
                                          { this->ConfigCallback(scriptingFactory, physicsStageFactory, particleFactory); },
                                          resourceManager,
                                          audioManager,
                                          cameraConfigPath,
                                          sceneConfigPath,
                                          inputConfigPath,
                                          renderViewConfigPath,
                                          effectLibPath);

    // Register gameplay HUDs in one place; future HUDs can be added here.
    auto hudFactory = std::make_unique<HudFactory>();
    hudFactory->Register(ENTITY_PLATE_HUD, [this]()
                         { return std::make_unique<EntityPlateHud>(m_world.get()); });
    hudFactory->Register(CHAT_HUD, [this]()
                         { return std::make_unique<ChatHud>(screenManager, &m_world->GetInputManager()); });
    m_hudManager = std::make_unique<HudManager>(std::move(hudFactory));
}
GameplayScreen::~GameplayScreen()
{
    OnExit();
}

void GameplayScreen::ConfigCallback(ScriptingFactory &scriptingFactory, PhysicsStageFactory &physicsStageFactory, ParticleFactory &particleFactory)
{
    // 注册后才可使用json配置
    physicsStageFactory.Register("SolarStage", []()
                                 { return std::make_unique<SolarStage>(); });
    physicsStageFactory.Register("NetworkVerifyStage", []()
                                 { return std::make_unique<NetworkVerifyStage>(); });
    physicsStageFactory.Register("CollisionStage", []()
                                 { return std::make_unique<CollisionStage>(); });

    // 注册脚本
    scriptingFactory.Register("RotatorScript", []()
                              { return std::make_unique<RotatorScript>(); });
    scriptingFactory.Register("CollisionListener", []()
                              { return std::make_unique<CollisionListener>(); });
    scriptingFactory.Register("WeaponScript", []()
                              { return std::make_unique<WeaponScript>(); });
    scriptingFactory.Register("BulletScript", []()
                              { return std::make_unique<BulletScript>(); });
    scriptingFactory.Register("RayScript", []()
                              { return std::make_unique<RayScript>(); });
    scriptingFactory.Register("LocalPlayerSyncScript", []()
                              { return std::make_unique<LocalPlayerSyncScript>(); });
    scriptingFactory.Register("AudioScript", []()
                              { return std::make_unique<AudioScript>(); });
    scriptingFactory.Register("PlayerControlScript", []()
                              { return std::make_unique<PlayerControlScript>(); });

    // 注
    // 注册粒子初始化器
    particleFactory.Register("SphereDir", []()
                             { return std::make_unique<SphereDir>(); });
    // particleFactory.Register("RadialVelocity", []()
    //                          { return std::make_unique<RadialVelocity>(); });
    particleFactory.Register("RandomLife", []()
                             { return std::make_unique<RandomLife>(); });
    particleFactory.Register("CollisionInit", []()
                             { return std::make_unique<CollisionInit>(); });
    particleFactory.Register("SPHInit", []()
                             { return std::make_unique<SPHInit>(); });
}

// 当进入游戏场景时调用
void GameplayScreen::OnEnter()
{
    DisableCursor();

    // ── 注入 ScreenManager 的全局 NetworkClient ──
    if (screenManager)
    {
        m_world->SetNetworkClient(screenManager->GetNetworkClient());
    }

    // ── 网络：连接服务器（使用配置的IP和端口） ──
    std::string serverHost = DEFAULT_SERVER_HOST;
    uint16_t serverPort = DEFAULT_SERVER_PORT;
    if (screenManager)
    {
        const auto &config = screenManager->GetActiveConfig();
        serverHost = config.serverIP;
        serverPort = config.serverPort;
    }
    auto &netClient = m_world->GetNetworkClient();
    if (netClient.GetConnectionState() == ConnectionState::Disconnected)
    {
        netClient.Connect(serverHost, serverPort);
    }
    m_world->GetNetworkSyncSystem().Init(netClient);
    if (m_hudManager)
    {
        m_hudManager->Clear();
        // Keep gameplay overlays mounted in stable order.
        m_hudManager->AddHud(ENTITY_PLATE_HUD);
        m_hudManager->AddHud(CHAT_HUD);
    }

    // 监听事件
    m_world->GetEventManager().Subscribe<CollisionEvent>([this](const CollisionEvent &e)
                                                         {
                                                             std::cout << "CollisionEvent, impluse: " << e.impulse << std::endl;
                                                             //  e.hitpoint.print();
                                                             std::cout << "relative velocity: " << e.relativeVelocity.Length() << std::endl;
                                                             if (std::fabsf(e.relativeVelocity.Length()) < 2.0f || std::fabsf(e.impulse) < 10.0f)
                                                                 return;
                                                             auto &particleSys = m_world->GetParticleSystem();
                                                             particleSys.Spawn("Collision",
                                                                               e.hitpoint,
                                                                               "relVel", e.relativeVelocity,
                                                                               "normal", e.normal,
                                                                               "impulse", e.impulse,
                                                                               "maxSpeed", e.relativeVelocity.Length() / 4);
                                                             float randomPitch = 0.5f + (float)GetRandomValue(0, 100) / 100.0f;
                                                             //  m_world->GetAudioManager().PlaySpatial("explosion", e.hitpoint, 5.0f, 50.0f, e.relativeVelocity.Length() / 4, randomPitch);
                                                         });
    m_world->GetParticleSystem().Spawn("SPH", Vector3f(0.0f, 3.0f, 0.0f));
}

// 当离开游戏场景时调用
void GameplayScreen::OnExit()
{
    if (m_hudManager)
        m_hudManager->Clear();

    auto &netClient = m_world->GetNetworkClient();

    // Send ObjectRelease for all local objects → server broadcasts
    // ObjectDespawn to other clients so they destroy our remote plane.
    // Connection stays alive for lobby / chat / other screens.
    m_world->GetNetworkSyncSystem().ReleaseLocalObjects(*m_world, netClient);

    // Clear network callbacks (sync system is cleaned up above)
    netClient.SetOnPositionBroadcast({});
    netClient.SetOnObjectDespawn({});

    EnableCursor();
}

// 在固定时间步更新（未来的物理和网络逻辑将在这里）
void GameplayScreen::FixedUpdate(float fixedDeltaTime)
{
    // m_networkManager->Update(fixedDeltaTime);
    // auto *mainCam = m_cameraManager->GetMainCamera();
    // Vector3 mainPos = mainCam->position;
    // mainPos = Vector3Add(mainPos, Vector3Scale(mainCam->direction, 0.2f));

    // mainCam->UpdateFromDirection(mainPos, mainCam->direction, mainCam->up);

    // auto &m_inputManager = m_world->GetInputManager();
    // m_inputManager.Update();
    if (m_hudManager)
    {
        m_hudManager->FixedUpdate(fixedDeltaTime);
    }
    m_world->FixedUpdate(fixedDeltaTime);
}

void GameplayScreen::Update(float deltaTime)
{
    m_nextScreenState = SCREEN_STATE_NONE;

    auto &m_inputManager = m_world->GetInputManager();
    auto &m_cameraManager = m_world->GetCameraManager();

    if (m_hudManager)
    {
        m_hudManager->Update(deltaTime);
    }

    m_inputManager.Update();
    if (m_inputManager.IsActionPressed("Fire"))
    {
        std::cout << "Fire" << std::endl;
    }
    if (!m_world->Update(deltaTime))
        m_nextScreenState = MAIN_MENU;

    const bool chatBlocksInput = (m_hudManager && m_hudManager->BlocksGameplayInput());
    const bool suppressExit = (m_hudManager && m_hudManager->ConsumeExitSuppressRequest());

    if (!chatBlocksInput && !suppressExit &&
        m_inputManager.IsActionPressed("Exit"))
    {
        m_nextScreenState = MAIN_MENU;
    }

    if (auto *observerCam = m_cameraManager.GetCamera("observer"))
    {
        Vector3f observerPos = observerCam->Position();

        if (m_inputManager.IsActionDown("Forward"))
        {
            observerPos += observerCam->Direction() * 0.3f;
        }
        if (m_inputManager.IsActionDown("Backward"))
        {
            observerPos -= observerCam->Direction() * 0.1f;
        }
        if (m_inputManager.IsActionDown("Left"))
        {
            observerPos -= observerCam->Right() * 0.1f;
        }
        if (m_inputManager.IsActionDown("Right"))
        {
            observerPos += observerCam->Right() * 0.1f;
        }
        observerCam->UpdateFromDirection(observerPos, observerCam->Direction(), observerCam->Up());

        float lookHorizontal = -m_inputManager.GetAxisValue("LookHorizontal") * PI / 180;
        float lookVertical = m_inputManager.GetAxisValue("LookVertical") * PI / 180;
        observerCam->Rotate(lookHorizontal, lookVertical);

        // if (auto *rearCam = m_cameraManager.GetCamera("rear_view"))
        // {
        //     Vector3f mainPos = mainCam->Position();
        //     Vector3f mainTarget = mainCam->Target();
        //     Vector3f direction = mainTarget - mainPos;
        //     direction.Normalize();

        //     rearCam->UpdateFromDirection(mainPos, -direction, mainCam->Up());
        // }
        if (auto *follow = m_cameraManager.GetCamera("follow"))
        {
            Vector3f dir = follow->getLocalLookAtOffset();
            Vector3f up = Vector3f::UP;
            float lookHorizontal = -m_inputManager.GetAxisValue("LookHorizontal") * PI / 180;
            float lookVertical = m_inputManager.GetAxisValue("LookVertical") * PI / 180;
            follow->Rotate(lookHorizontal, lookVertical);
            // follow->UpdateFixed(dir, up);
        }
    }
}

void GameplayScreen::Draw()
{
    ClearBackground(RAYWHITE); // 设置一个浅灰色背景

    m_world->Render();
    // 在3D内容之上绘制一些2D的调试信息
    if (m_hudManager && m_hudManager->BlocksGameplayInput())
    {
        DrawText("Chat active: ENTER send, ESC close chat", 10, GetScreenHeight() - 30, 20, DARKGRAY);
    }
    else
    {
        DrawText("Press ENTER to chat, ESC to return.", 10, GetScreenHeight() - 30, 20, DARKGRAY);
    }
    int total = m_world->GetGameObjects().size();
    int active = m_world->GetActivateGameObjects().size();
    DrawText(TextFormat("Total Entities: %d", total), 10, 50, 20, WHITE);
    DrawText(TextFormat("Active Entities: %d", active), 10, 80, 20, GREEN);

    if (m_hudManager)
    {
        m_hudManager->Draw();
    }

    // Always draw UILayer in gameplay so new chat messages are visible
    // even when chat input is not active.
    if (screenManager && screenManager->GetUILayer())
    {
        screenManager->GetUILayer()->Draw();
    }
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
