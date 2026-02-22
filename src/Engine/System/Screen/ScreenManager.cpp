#include "ScreenManager.h"
#include "raylib.h"
#include "ScreenState.h"
#include "Game/Screen/MyScreenState.h"
#include "Engine/Network/Chat/ChatManager.h"
#include "Engine/System/HUD/HudBridgeScript.h"
#include <algorithm>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#else
#include "Engine/UI/UltralightLayer.h"
#endif

namespace
{
    std::string GetCurrentDirectoryPath()
    {
        // 使用 raylib 的 GetApplicationDirectory() 来获取可执行文件所在目录
        // 这样可以正确找到相对于 exe 的资源文件
        return GetApplicationDirectory();
    }
} // namespace

ScreenManager::ScreenManager(const EngineConfig &config, const std::string audioPath, std::unique_ptr<ScreenFactory> factory)
    : m_factory(std::move(factory)), m_activeConfig(config)
{
    InitWindow(config.screenWidth, config.screenHeight, config.windowTitle.c_str());
    SetTargetFPS((int)config.targetFPS);
    m_timeManager = TimeManager(static_cast<float>(config.targetFPS));
    m_accumulator = 0.0f;
    SetExitKey(KEY_NULL);
    m_activeConfig.screenWidth = GetScreenWidth();
    m_activeConfig.screenHeight = GetScreenHeight();
    m_activeConfig.fullScreen = IsWindowFullscreen();

    // ── Global Network Client ──────────────────────────────────────
    m_networkClient = std::make_shared<NetworkClient>();
    m_clientIdentity.LoadOrGenerate();
    m_networkClient->SetUUID(m_clientIdentity.GetUUID());
    m_networkClient->SetOnChatMessage(
        [this](ChatMessageType type, ClientID senderID,
               const std::string &senderName, const std::string &text)
        {
            PushChatMessageToUI(type, senderID, senderName, text);
        });
    m_networkClient->Connect(config.serverIP, config.serverPort);
    TraceLog(LOG_INFO, "CLIENT: UUID = %s", m_clientIdentity.GetUUIDString().c_str());

    m_resourceManager = std::make_unique<ResourceManager>();
    m_audioManager = std::make_unique<AudioManager>(*m_resourceManager);

    m_audioManager->LoadLibrary(audioPath);
#if defined(PLATFORM_WEB)
    m_uiLayer = std::make_unique<WebLayer>();
#else
    m_uiLayer = std::make_unique<UltralightLayer>();
#endif

    m_uiLayer->Initialize(
        static_cast<uint32_t>(GetScreenWidth()),
        static_cast<uint32_t>(GetScreenHeight()),
        GetCurrentDirectoryPath());

    m_currentScreen = m_factory->Create(config.initialScreen, this);
    m_currentScreen->OnEnter();

    InitAudioDevice();
    SetMasterVolume(1.0f);
}

ScreenManager::~ScreenManager()
{
    Shutdown();
}
void ScreenManager::ApplySettings(const EngineConfig &newConfig)
{
    const std::string oldServerIP = m_activeConfig.serverIP;
    const uint16_t oldServerPort = m_activeConfig.serverPort;

#if defined(PLATFORM_WEB)
    if (newConfig.fullScreen)
    {
        EmscriptenFullscreenStrategy strategy = {
            EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT,
            EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE,
            EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT};
        emscripten_request_fullscreen_strategy("#canvas", EM_TRUE, &strategy);
    }
    else
    {
        emscripten_exit_fullscreen();
    }
#else
    if (IsWindowFullscreen() != newConfig.fullScreen)
    {
        ToggleFullscreen();
    }
    if (!IsWindowFullscreen())
    {
        if (GetScreenWidth() != newConfig.screenWidth || GetScreenHeight() != newConfig.screenHeight)
        {
            SetWindowSize(newConfig.screenWidth, newConfig.screenHeight);
            int monitor = GetCurrentMonitor();
            SetWindowPosition(
                (GetMonitorWidth(monitor) - newConfig.screenWidth) / 2,
                (GetMonitorHeight(monitor) - newConfig.screenHeight) / 2);
        }
    }
#endif
    SetTargetFPS((int)newConfig.targetFPS);

    // Persist the last applied settings and sync to the actual runtime window.
    m_activeConfig = newConfig;
    m_activeConfig.fullScreen = IsWindowFullscreen();
    if (!m_activeConfig.fullScreen)
    {
        m_activeConfig.screenWidth = GetScreenWidth();
        m_activeConfig.screenHeight = GetScreenHeight();
    }
    m_activeConfig.targetFPS = newConfig.targetFPS;
    m_activeConfig.serverIP = newConfig.serverIP;
    m_activeConfig.serverPort = newConfig.serverPort;

    if (m_networkClient)
    {
        const bool endpointChanged =
            (oldServerIP != newConfig.serverIP) ||
            (oldServerPort != newConfig.serverPort);

        if (endpointChanged)
        {
            if (m_networkClient->GetConnectionState() != ConnectionState::Disconnected)
                m_networkClient->Disconnect();
            m_networkClient->Connect(newConfig.serverIP, newConfig.serverPort);
        }
    }
}

const EngineConfig &ScreenManager::GetActiveConfig() const
{
    return m_activeConfig;
}

UILayer *ScreenManager::GetUILayer()
{
    return m_uiLayer.get();
}

ResourceManager &ScreenManager::GetResourceManager()
{
    return *m_resourceManager;
}

AudioManager &ScreenManager::GetAudioManager()
{
    return *m_audioManager;
}

bool ScreenManager::UpdateFrame()
{
    if (WindowShouldClose() || m_currentScreen->GetNextScreenState() == SCREEN_STATE_EXIT)
    {
        return false;
    }

    m_resourceManager->UpdateMusic();

    m_timeManager.Tick();
    m_accumulator += m_timeManager.GetDeltaTime();

    while (m_accumulator >= m_timeManager.GetFixedDeltaTime())
    {
        m_currentScreen->FixedUpdate(m_timeManager.GetFixedDeltaTime());
        m_accumulator -= m_timeManager.GetFixedDeltaTime();
    }

    m_currentScreen->Update(m_timeManager.GetDeltaTime());

    // Poll global network while outside gameplay, and send keep-alive heartbeats
    // so idle menu/options sessions are not timed out by the server.
    if (m_networkClient && m_currentScreen)
    {
        if (m_currentScreen->GetScreenState() != GAMEPLAY)
        {
            m_networkClient->Poll();

            if (m_networkClient->IsConnected())
            {
                m_heartbeatCooldown -= m_timeManager.GetDeltaTime();
                if (m_heartbeatCooldown <= 0.0f)
                {
                    m_networkClient->SendHeartbeat();
                    m_heartbeatCooldown = HEARTBEAT_INTERVAL;
                }
            }
            else
            {
                m_heartbeatCooldown = 0.0f;
            }
        }
        else
        {
            // Gameplay has high-frequency position sync already.
            m_heartbeatCooldown = 0.0f;
        }
    }
    if (m_uiLayer)
    {

        m_uiLayer->Resize(
            static_cast<uint32_t>(GetScreenWidth()),
            static_cast<uint32_t>(GetScreenHeight()));
        m_uiLayer->HandleInput();
        m_uiLayer->Update();
        FlushPendingChatToUI();
        PollGlobalChatSendRequest();
    }

    BeginDrawing();
    ClearBackground(BLACK);
    m_currentScreen->Draw();
    EndDrawing();

    int nextState = m_currentScreen->GetNextScreenState();
    if (nextState != SCREEN_STATE_NONE)
    {
        ChangeScreen(nextState);
    }

    return true;
}
void ScreenManager::Shutdown()
{
    if (m_currentScreen)
    {
        m_currentScreen->OnExit();
    }
    // Disconnect the global network client before tearing down the window.
    if (m_networkClient && m_networkClient->IsConnected())
    {
        m_networkClient->Disconnect();
    }
    if (m_uiLayer)
    {
        m_uiLayer->Shutdown();
        m_uiLayer.reset();
    }
    CloseAudioDevice();
    CloseWindow();
}
// 屏幕切换
void ScreenManager::ChangeScreen(int newState)
{
    if (newState == SCREEN_STATE_EXIT || newState == SCREEN_STATE_NONE)
    {
        return;
    }
    // 1. 删除当前屏幕，释放资源
    if (m_currentScreen)
    {
        m_currentScreen->OnExit();
        m_currentScreen.reset();
    }
    // 2. 使用工厂创建一个新的屏幕实例
    m_currentScreen = m_factory->Create(newState, this);

    // 3. 如果新屏幕创建成功，则通知它进入
    if (m_currentScreen)
    {

        m_currentScreen->OnEnter();
    }
    else
    {
        m_currentScreen = m_factory->Create(SCREEN_STATE_ERROR, this);
        if (m_currentScreen)
            m_currentScreen->OnEnter();
    }
}

void ScreenManager::PushChatMessageToUI(ChatMessageType type, ClientID senderID,
                                        const std::string &senderName,
                                        const std::string &text)
{
    ChatEntry entry;
    entry.type = type;
    entry.senderID = senderID;
    entry.senderName = senderName;
    entry.text = text;
    m_pendingChatToUI.push_back(std::move(entry));

    constexpr size_t kMaxQueuedIncomingChat = 4096;
    while (m_pendingChatToUI.size() > kMaxQueuedIncomingChat)
        m_pendingChatToUI.pop_front();
}

void ScreenManager::FlushPendingChatToUI()
{
    if (!m_uiLayer || m_pendingChatToUI.empty())
        return;

    constexpr size_t kMaxPushPerFrame = 64;
    size_t count = std::min(kMaxPushPerFrame, m_pendingChatToUI.size());

    std::string batch = "[";
    for (size_t i = 0; i < count; ++i)
    {
        if (i > 0)
            batch += ",";
        batch += ChatManager::EntryToJSON(m_pendingChatToUI.front());
        m_pendingChatToUI.pop_front();
    }
    batch += "]";

    std::string script =
        "if (window.__NW_CHAT_PUSH_BATCH__) window.__NW_CHAT_PUSH_BATCH__(" + batch + ");"
                                                                                      "else if (window.__NW_CHAT_PUSH__) {"
                                                                                      "var __nwBatch=" +
        batch + ";"
                "for (var i=0;i<__nwBatch.length;i++) window.__NW_CHAT_PUSH__(__nwBatch[i]);"
                "}";
    m_uiLayer->ExecuteScript(script);
}

void ScreenManager::PollGlobalChatSendRequest()
{
    if (!m_uiLayer || !m_networkClient)
        return;

    const float dt = m_timeManager.GetDeltaTime();

    // ── 1. Drain the JS-side send queue into C++ queue ─────────────
    std::string raw = m_uiLayer->GetAppState("chatSendQueue");
    if (!raw.empty() && raw != "null" && raw != "undefined" && raw != "[]")
    {
        m_uiLayer->ExecuteScript(HudBridgeScript::ClearChatQueue());

        if (raw.size() >= 2 && raw.front() == '[' && raw.back() == ']')
        {
            std::string inner = raw.substr(1, raw.size() - 2);
            size_t pos = 0;
            while (pos < inner.size())
            {
                if (inner[pos] != '"')
                {
                    ++pos;
                    continue;
                }
                size_t start = pos + 1;
                size_t end = start;
                while (end < inner.size())
                {
                    if (inner[end] == '\\')
                    {
                        end += 2;
                        continue;
                    }
                    if (inner[end] == '"')
                        break;
                    ++end;
                }
                std::string elem = inner.substr(start, end - start);
                // Unescape basic JSON
                std::string text;
                text.reserve(elem.size());
                for (size_t i = 0; i < elem.size(); ++i)
                {
                    if (elem[i] == '\\' && i + 1 < elem.size())
                    {
                        char next = elem[i + 1];
                        if (next == '"' || next == '\\' || next == '/')
                        {
                            text += next;
                            ++i;
                        }
                        else if (next == 'n')
                        {
                            text += '\n';
                            ++i;
                        }
                        else
                        {
                            text += elem[i];
                        }
                    }
                    else
                    {
                        text += elem[i];
                    }
                }
                if (!text.empty() && m_chatSendQueue.size() < CHAT_SEND_QUEUE_MAX)
                    m_chatSendQueue.push_back(std::move(text));

                pos = (end < inner.size()) ? end + 1 : inner.size();
                while (pos < inner.size() && (inner[pos] == ',' || inner[pos] == ' '))
                    ++pos;
            }
        }
    }

    // Legacy single-slot path (GameplayScreen)
    if (m_uiLayer->GetAppState("chatSendRequested") == "true")
    {
        std::string text = m_uiLayer->GetAppState("chatSendText");
        m_uiLayer->ExecuteScript(HudBridgeScript::ClearLegacyChatSend());
        if (!text.empty() && m_chatSendQueue.size() < CHAT_SEND_QUEUE_MAX)
            m_chatSendQueue.push_back(std::move(text));
    }

    // ── 2. Rate-limited send: at most 1 message per CHAT_SEND_INTERVAL ──
    if (m_chatSendCooldown > 0.0f)
        m_chatSendCooldown -= dt;

    if (m_chatSendQueue.empty() || !m_networkClient->IsConnected())
        return;

    if (m_chatSendCooldown > 0.0f)
        return; // still cooling down, messages stay queued

    // Send exactly one message
    const std::string &msg = m_chatSendQueue.front();
    if (m_networkClient->SendChatMessage(
            ChatMessageType::Public, msg, INVALID_CLIENT_ID))
    {
        m_chatSendCooldown = CHAT_SEND_INTERVAL;
        m_chatSendQueue.pop_front();
    }
}
