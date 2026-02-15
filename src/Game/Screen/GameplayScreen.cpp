#include "GameplayScreen.h"
#include "raylib.h"
#include "Game/Screen/MyScreenState.h"
#include "Game/Systems/Physics/SolarStage.h"
#include "Game/Systems/Physics/NetworkVerifyStage.h"
#include "Game/Systems/Particles/Initializers/RandomLife.h"
#include "Game/Systems/Particles/Initializers/SphereDir.h"
#include "Game/Systems/Particles/Initializers/CollisionInit.h"
#include "Game/Systems/Particles/Initializers/SPHInit.h"
#include "Game/Scripts/Scripts.h"

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
    const std::string &effectLibPath = "assets/Library/particle_effects.json";
    m_world = std::make_unique<GameWorld>([this](ScriptingFactory &scriptingFactory, PhysicsStageFactory &physicsStageFactory, ParticleFactory &particleFactory)
                                          { this->ConfigCallback(scriptingFactory, physicsStageFactory, particleFactory); },
                                          cameraConfigPath,
                                          sceneConfigPath,
                                          inputConfigPath,
                                          renderViewConfigPath,
                                          effectLibPath);
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

    // ── 客户端身份：加载或生成 UUID ──
    ClientIdentity identity;
    identity.LoadOrGenerate();
    TraceLog(LOG_INFO, "CLIENT: UUID = %s", identity.GetUUIDString().c_str());

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
    netClient.SetUUID(identity.GetUUID());
    netClient.Connect(serverHost, serverPort);
    m_world->GetNetworkSyncSystem().Init(m_world->GetNetworkClient());

    // 监听事件
    // m_world->GetEventManager().Subscribe<CollisionEvent>([this](const CollisionEvent &e)
    //                                                      { std::cout << "CollisionEvent, impluse: " <<e.impulse << std::endl;
    //                                                         e.hitpoint.print();
    //                                                         if(std::fabsf(e.relativeVelocity.Length())<1.0f) return;
    //                                                         auto &particleSys = m_world->GetParticleSystem();
    //                                                         particleSys.Spawn("Collision", e.hitpoint,
    //                                                             "relVel",e.relativeVelocity,
    //                                                             "normal",e.normal,
    //                                                             "impulse",e.impulse,
    //                                                             "maxSpeed",e.relativeVelocity.Length()/4); });
    m_world->GetParticleSystem().Spawn("SPH", Vector3f(0.0f, 3.0f, 0.0f));
}

// 当离开游戏场景时调用
void GameplayScreen::OnExit()
{
    m_world->GetNetworkClient().Disconnect();
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
    m_world->FixedUpdate(fixedDeltaTime);
}

void GameplayScreen::Update(float deltaTime)
{
    m_nextScreenState = SCREEN_STATE_NONE;

    auto &m_inputManager = m_world->GetInputManager();
    auto &m_cameraManager = m_world->GetCameraManager();
    m_inputManager.Update();
    if (m_inputManager.IsActionPressed("Fire"))
    {
        std::cout << "Fire" << std::endl;
    }
    if (!m_world->Update(deltaTime))
        m_nextScreenState = MAIN_MENU;

    // TODO:操作部分通过脚本实现
    if (m_inputManager.IsActionPressed("Exit"))
    {
        m_nextScreenState = MAIN_MENU;
    }

    if (auto *mainCam = m_cameraManager.GetMainCamera())
    {
        Vector3f mainPos = mainCam->Position();

        if (m_inputManager.IsActionDown("Forward"))
        {
            mainPos += mainCam->Direction() * 0.3f;
        }
        if (m_inputManager.IsActionDown("Backward"))
        {
            mainPos -= mainCam->Direction() * 0.1f;
        }
        if (m_inputManager.IsActionDown("Left"))
        {
            mainPos -= mainCam->Right() * 0.1f;
        }
        if (m_inputManager.IsActionDown("Right"))
        {
            mainPos += mainCam->Right() * 0.1f;
        }
        mainCam->UpdateFromDirection(mainPos, mainCam->Direction(), mainCam->Up());

        float lookHorizontal = -m_inputManager.GetAxisValue("LookHorizontal") * PI / 180;
        float lookVertical = m_inputManager.GetAxisValue("LookVertical") * PI / 180;
        mainCam->Rotate(lookHorizontal, lookVertical);

        if (auto *rearCam = m_cameraManager.GetCamera("rear_view"))
        {
            Vector3f mainPos = mainCam->Position();
            Vector3f mainTarget = mainCam->Target();
            Vector3f direction = mainTarget - mainPos;
            direction.Normalize();

            rearCam->UpdateFromDirection(mainPos, -direction, mainCam->Up());
        }
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
    DrawText("Press ESC to return.", 10, GetScreenHeight() - 30, 20, DARKGRAY);
    int total = m_world->GetGameObjects().size();
    int active = m_world->GetActivateGameObjects().size();
    DrawText(TextFormat("Total Entities: %d", total), 10, 50, 20, WHITE);
    DrawText(TextFormat("Active Entities: %d", active), 10, 80, 20, GREEN);
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
