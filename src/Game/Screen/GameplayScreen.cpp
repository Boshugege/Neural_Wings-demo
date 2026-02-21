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
    m_skipExitThisFrame = false;
    m_framesInScreen = 0;

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

    // Initialise Vue-side chat state
    if (screenManager)
    {
        if (auto *ui = screenManager->GetUILayer())
        {
            // Navigate to gameplay route so only ChatHUD renders (no menu overlay)
            ui->ExecuteScript("window.location.hash = '#/gameplay';");
            ui->ExecuteScript(
                "window.vueAppState = window.vueAppState || {};"
                "window.vueAppState.chatMessages = [];"
                "window.vueAppState.chatSendRequested = false;"
                "window.vueAppState.chatSendText = '';"
                "window.vueAppState.chatActive = false;"
                "window.vueAppState.chatInputText = '';");
        }
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
    if (m_chatActive)
        DeactivateChat();
    auto &netClient = m_world->GetNetworkClient();
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
    m_world->FixedUpdate(fixedDeltaTime);
}

void GameplayScreen::Update(float deltaTime)
{
    m_nextScreenState = SCREEN_STATE_NONE;

    auto &m_inputManager = m_world->GetInputManager();
    auto &m_cameraManager = m_world->GetCameraManager();

    // ── Chat toggle (raw raylib key, not via InputManager) ─────────
    // Enter opens chat; when chat active, Enter sends and Esc closes.
    // Guard: skip the first few frames to avoid accidental activation
    // from a lingering Enter keypress during screen transition.
    ++m_framesInScreen;
    if (m_chatActive)
    {
        PollChatUI(); // check for Enter (send) / Esc (dismiss) via raw raylib
    }
    else if (m_framesInScreen > 2 && IsKeyPressed(KEY_ENTER))
    {
        ActivateChat();
    }

    m_inputManager.Update();
    if (m_inputManager.IsActionPressed("Fire"))
    {
        std::cout << "Fire" << std::endl;
    }
    if (!m_world->Update(deltaTime))
        m_nextScreenState = MAIN_MENU;

    // Only allow Exit when chat is NOT active
    if (!m_chatActive && !m_skipExitThisFrame &&
        m_inputManager.IsActionPressed("Exit"))
    {
        m_nextScreenState = MAIN_MENU;
    }
    m_skipExitThisFrame = false;

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
    if (m_chatActive)
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

    // Draw UILayer overlay when chat is active
    if (m_chatActive && screenManager && screenManager->GetUILayer())
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

// ── Chat helpers ───────────────────────────────────────────────────

void GameplayScreen::ActivateChat()
{
    if (m_chatActive)
        return;
    m_chatActive = true;
    m_world->GetInputManager().SetEnabled(false);
    EnableCursor();

    if (screenManager)
    {
        if (auto *ui = screenManager->GetUILayer())
        {
            // Make UILayer visible so Ultralight forwards keyboard to Vue
            ui->SetVisible(true);
            ui->ExecuteScript(
                "window.vueAppState.chatActive = true;"
                "if (window.__NW_CHAT_ACTIVATE__) window.__NW_CHAT_ACTIVATE__();");
        }
    }
}

void GameplayScreen::DeactivateChat()
{
    if (!m_chatActive)
        return;
    m_chatActive = false;
    m_world->GetInputManager().SetEnabled(true);
    DisableCursor();

    if (screenManager)
    {
        if (auto *ui = screenManager->GetUILayer())
        {
            ui->ExecuteScript(
                "window.vueAppState.chatActive = false;"
                "if (window.__NW_CHAT_DEACTIVATE__) window.__NW_CHAT_DEACTIVATE__();");
            // Hide UILayer again so it doesn't overlay gameplay
            ui->SetVisible(false);
        }
    }
}

void GameplayScreen::PollChatUI()
{
    // ── Esc → dismiss chat (C++ side, no dependency on Vue keyboard) ──
    if (IsKeyPressed(KEY_ESCAPE))
    {
        // Consume this ESC so it won't also trigger gameplay "Exit" in this frame.
        m_skipExitThisFrame = true;
        DeactivateChat();
        return;
    }

    // ── Enter → send message ──
    if (IsKeyPressed(KEY_ENTER))
    {
        if (!screenManager)
            return;
        auto *ui = screenManager->GetUILayer();
        if (!ui)
            return;

        // Read text from Vue input field
        std::string text = ui->GetAppState("chatInputText");
        ui->ExecuteScript("if (window.__NW_CHAT_CLEAR_INPUT__) window.__NW_CHAT_CLEAR_INPUT__();");

        if (!text.empty())
        {
            // Parse whisper: "/w <id> message" or "/whisper <id> message"
            ChatMessageType chatType = ChatMessageType::Public;
            ClientID targetID = INVALID_CLIENT_ID;
            std::string msgText = text;

            if (text.size() > 3 && (text.substr(0, 3) == "/w " || text.substr(0, 9) == "/whisper "))
            {
                size_t start = (text[1] == 'w' && text[2] == ' ') ? 3 : 9;
                size_t spacePos = text.find(' ', start);
                if (spacePos != std::string::npos)
                {
                    try
                    {
                        targetID = static_cast<ClientID>(std::stoul(text.substr(start, spacePos - start)));
                        msgText = text.substr(spacePos + 1);
                        chatType = ChatMessageType::Whisper;
                    }
                    catch (...)
                    {
                        // Invalid ID, send as public
                    }
                }
            }

            m_world->GetNetworkClient().SendChatMessage(chatType, msgText, targetID);
        }
        // Keep chat open after sending
    }
}

