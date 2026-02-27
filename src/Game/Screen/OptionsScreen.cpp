#include "OptionsScreen.h"
#include "raylib.h"
#include "Engine/System/Screen/ScreenManager.h"
#include "Game/Screen/MyScreenState.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
#include <cstdio>

namespace
{
    void PushNicknameStateToUI(UILayer *ui, const std::string &status,
                               const std::string &nickname = "")
    {
        if (!ui)
            return;

        json payload = {
            {"status", status},
            {"nickname", nickname},
        };

        const std::string payloadJson = payload.dump();
        std::string script =
            "window.vueAppState = window.vueAppState || {};"
            "(function(){ const __nwNick = " +
            payloadJson +
            ";"
            "if (typeof __nwNick.status === 'string') "
            "window.vueAppState.nicknameStatus = __nwNick.status;"
            "if (typeof __nwNick.nickname === 'string' && __nwNick.nickname.length > 0) {"
            "window.vueAppState.nicknameServerName = __nwNick.nickname;"
            "window.vueAppState.nickname = __nwNick.nickname;"
            "}"
            "if (window.__NW_APPLY_NICKNAME_STATE__) "
            "window.__NW_APPLY_NICKNAME_STATE__(__nwNick);"
            "})();";

        ui->ExecuteScript(script);
    }
}

OptionsScreen::OptionsScreen(ScreenManager *sm)
    : m_nextScreenState(SCREEN_STATE_NONE), IGameScreen(sm)
{
}
OptionsScreen::~OptionsScreen()
{
    OnExit();
}
void OptionsScreen::OnEnter()
{
    // Prefer the runtime-applied settings so the UI reflects the actual window.
    if (screenManager)
    {
        m_currentConfig = screenManager->GetActiveConfig();
    }
    else
    {
        m_currentConfig.load("assets/config/engine_config.json");
    }

    // As an extra guard, sync window size/fullscreen from raylib.
    m_currentConfig.fullScreen = IsWindowFullscreen();
    if (!m_currentConfig.fullScreen)
    {
        m_currentConfig.screenWidth = GetScreenWidth();
        m_currentConfig.screenHeight = GetScreenHeight();
    }

    m_modifiedConfig = m_currentConfig;
    m_pendingSync = true;
    m_waitingServerCheck = false;
    m_serverCheckTimer = 0.0f;
    m_waitingNicknameFetch = false;
    m_nicknameFetchTimer = 0.0f;

    if (screenManager && screenManager->GetUILayer())
    {
        auto *uiLayer = screenManager->GetUILayer();
        uiLayer->SetVisible(true);
        // Reset readiness so we don't sync against the previous page state.
        uiLayer->ExecuteScript(
            "window.vueAppState = window.vueAppState || {};"
            "window.vueAppState.vueAppReady = false;"
            "window.vueAppState.settingsSaveRequested = false;"
            "window.vueAppState.nicknameApplyRequested = false;");
        uiLayer->LoadRoute(OPTIONS);
    }

    if (screenManager)
    {
        auto &netClient = screenManager->GetNetworkClientRef();
        netClient.SetOnNicknameUpdateResult(
            [this](NicknameUpdateStatus status, const std::string &authoritativeNickname)
            {
                if (!screenManager || !screenManager->GetUILayer())
                    return;
                auto *ui = screenManager->GetUILayer();
                std::string statusStr = "invalid";
                if (m_waitingNicknameFetch)
                {
                    m_waitingNicknameFetch = false;
                    m_nicknameFetchTimer = 0.0f;
                    statusStr = authoritativeNickname.empty() ? "failed" : "accepted";
                }
                else
                {
                    switch (status)
                    {
                    case NicknameUpdateStatus::Accepted:
                        statusStr = "accepted";
                        break;
                    case NicknameUpdateStatus::Conflict:
                        statusStr = "conflict";
                        break;
                    case NicknameUpdateStatus::Invalid:
                        statusStr = "invalid";
                        break;
                    }
                }

                if (status == NicknameUpdateStatus::Accepted && !authoritativeNickname.empty())
                {
                    screenManager->GetNetworkClientRef().SetDesiredNickname(authoritativeNickname);
                }
                PushNicknameStateToUI(ui, statusStr, authoritativeNickname);
            });
        StartNicknameFetch();
    }
}
void OptionsScreen::OnExit()
{
    if (screenManager)
    {
        auto &netClient = screenManager->GetNetworkClientRef();
        netClient.SetOnNicknameUpdateResult({});
    }
    m_waitingServerCheck = false;
    m_serverCheckTimer = 0.0f;
    m_waitingNicknameFetch = false;
    m_nicknameFetchTimer = 0.0f;

    // 不再自动应用设置，只有点击保存按钮时才应用
    if (screenManager && screenManager->GetUILayer())
    {
        screenManager->GetUILayer()->SetVisible(false);
    }
}
void OptionsScreen::FixedUpdate(float fixedDeltaTime) {}

void OptionsScreen::Update(float deltaTime)
{
    m_nextScreenState = SCREEN_STATE_NONE;
    HandleNicknameApplyRequest();
    UpdateNicknameFetch(deltaTime);

    // 检查 Vue 路由是否已变化
    if (screenManager && screenManager->GetUILayer())
    {
        std::string currentRoute = screenManager->GetUILayer()->GetCurrentRoute();

        if (currentRoute == "#/" + MAIN_MENU.getName())
        {
            m_nextScreenState = MAIN_MENU;
            return;
        }
    }

    bool chatActive = false;
    if (screenManager && screenManager->GetUILayer())
    {
        chatActive = (screenManager->GetUILayer()->GetAppState("chatActive") == "true");
    }

    if (!chatActive && IsKeyPressed(KEY_ESCAPE))
    {
        m_nextScreenState = MAIN_MENU;
    }

    // 检查 Vue 中的设置是否被保存
    if (screenManager && screenManager->GetUILayer())
    {
        if (m_pendingSync)
        {
            std::string readyStr = screenManager->GetUILayer()->GetAppState("vueAppReady");
            if (readyStr == "true")
            {
                ApplyConfigToUI();
                m_pendingSync = false;
            }
        }
        ApplyVueSettings();
        UpdatePingCheck(deltaTime);
    }
}

void OptionsScreen::Draw()
{
    if (screenManager && screenManager->GetUILayer())
    {
        screenManager->GetUILayer()->Draw();
    }
}

ScreenState OptionsScreen::GetNextScreenState() const
{
    return m_nextScreenState;
}

ScreenState OptionsScreen::GetScreenState() const
{
    return OPTIONS;
}

#include <fstream>
void OptionsScreen::SaveConfig()
{
    json configJson;
    m_modifiedConfig.toJson(configJson);

    std::ofstream o("assets/config/engine_config.json");
    if (o.is_open())
    {
        o << configJson.dump(4);
        o.close();
        printf("[OptionsScreen] Config saved to engine_config.json\n");
    }
}

void OptionsScreen::ApplyVueSettings()
{
    // Check if save is requested
    std::string saveRequestedStr = screenManager->GetUILayer()->GetAppState("settingsSaveRequested");

    if (saveRequestedStr == "true")
    {
        // Read settings values
        std::string fullscreenStr = screenManager->GetUILayer()->GetAppState("fullscreen");
        std::string resolutionStr = screenManager->GetUILayer()->GetAppState("resolution");
        std::string fpsStr = screenManager->GetUILayer()->GetAppState("targetFPS");

        // Parse resolution string (e.g. "1920x1080")
        size_t xPos = resolutionStr.find('x');
        if (xPos != std::string::npos)
        {
            try
            {
                int width = std::stoi(resolutionStr.substr(0, xPos));
                int height = std::stoi(resolutionStr.substr(xPos + 1));

                // Update configuration
                m_modifiedConfig.screenWidth = width;
                m_modifiedConfig.screenHeight = height;
            }
            catch (...)
            {
                printf("[OptionsScreen] Failed to parse resolution: %s\n", resolutionStr.c_str());
            }
        }

        // Parse fullscreen setting
        m_modifiedConfig.fullScreen = (fullscreenStr == "true");

        // Parse FPS
        try
        {
            m_modifiedConfig.targetFPS = static_cast<float>(std::stoi(fpsStr));
        }
        catch (...)
        {
            printf("[OptionsScreen] Failed to parse FPS: %s\n", fpsStr.c_str());
        }

        // Parse server IP
        std::string serverIPStr = screenManager->GetUILayer()->GetAppState("serverIP");
        if (!serverIPStr.empty())
        {
            m_modifiedConfig.serverIP = serverIPStr;
        }

        printf("[OptionsScreen] Settings saved and applied: %dx%d, fullscreen=%s, fps=%.0f, serverIP=%s\n",
               m_modifiedConfig.screenWidth,
               m_modifiedConfig.screenHeight,
               m_modifiedConfig.fullScreen ? "true" : "false",
               m_modifiedConfig.targetFPS,
               m_modifiedConfig.serverIP.c_str());

        // Save to file immediately
        SaveConfig();

        // Apply settings to game engine immediately
        if (screenManager)
        {
            screenManager->ApplySettings(m_modifiedConfig);
        }

        // Keep the current snapshot in sync after applying.
        m_currentConfig = m_modifiedConfig;

        // Reset save request flag
        screenManager->GetUILayer()->ExecuteScript("window.vueAppState.settingsSaveRequested = false;");

        printf("[OptionsScreen] Settings applied to engine\n");
    }
}

void OptionsScreen::ApplyConfigToUI()
{
    if (!screenManager || !screenManager->GetUILayer())
        return;

    const std::string resolution = std::to_string(m_currentConfig.screenWidth) + "x" +
                                   std::to_string(m_currentConfig.screenHeight);

    std::string nicknameFromNetwork;
    if (screenManager)
    {
        auto &netClient = screenManager->GetNetworkClientRef();
        nicknameFromNetwork = netClient.GetAuthoritativeNickname();
    }

    json payload = {
        {"fullscreen", m_currentConfig.fullScreen},
        {"resolution", resolution},
        {"targetFPS", static_cast<int>(m_currentConfig.targetFPS)},
        {"serverIP", m_currentConfig.serverIP},
        {"nickname", nicknameFromNetwork},
    };
    std::string script =
        "if (window.__applyEngineSettings) window.__applyEngineSettings(" +
        payload.dump() + ");";
    screenManager->GetUILayer()->ExecuteScript(script);
}

void OptionsScreen::UpdatePingCheck(float deltaTime)
{
    if (!screenManager)
        return;

    auto *uiLayer = screenManager->GetUILayer();
    if (!uiLayer)
        return;

    auto &netClient = screenManager->GetNetworkClientRef();
    const auto &activeConfig = screenManager->GetActiveConfig();

    if (uiLayer->GetAppState("serverCheckRequested") == "true")
    {
        uiLayer->ExecuteScript(
            "window.vueAppState.serverCheckRequested = false;"
            "window.vueAppState.serverStatus = 'checking';");

        std::string targetIP = uiLayer->GetAppState("serverIP");
        if (targetIP.empty())
            targetIP = activeConfig.serverIP;
        const uint16_t targetPort = activeConfig.serverPort;

        m_serverCheckTimer = 0.0f;

        // Single global client: perform a fresh connect attempt for this probe.
        if (netClient.GetConnectionState() != ConnectionState::Disconnected)
            netClient.Disconnect();

        if (!netClient.Connect(targetIP, targetPort))
        {
            m_waitingServerCheck = false;
            uiLayer->ExecuteScript("window.vueAppState.serverStatus = 'offline';");
            return;
        }

        m_waitingServerCheck = true;
        return;
    }

    if (!m_waitingServerCheck)
        return;

    if (netClient.IsConnected())
    {
        m_waitingServerCheck = false;
        m_serverCheckTimer = 0.0f;
        uiLayer->ExecuteScript("window.vueAppState.serverStatus = 'online';");
        return;
    }

    m_serverCheckTimer += deltaTime;
    if (netClient.GetConnectionState() == ConnectionState::Disconnected ||
        m_serverCheckTimer >= SERVER_CHECK_TIMEOUT)
    {
        m_waitingServerCheck = false;
        m_serverCheckTimer = 0.0f;
        if (netClient.GetConnectionState() != ConnectionState::Disconnected)
            netClient.Disconnect();
        uiLayer->ExecuteScript("window.vueAppState.serverStatus = 'offline';");
        return;
    }
}

void OptionsScreen::HandleNicknameApplyRequest()
{
    if (!screenManager || !screenManager->GetUILayer())
        return;
    auto *ui = screenManager->GetUILayer();

    if (ui->GetAppState("nicknameApplyRequested") != "true")
        return;

    ui->ExecuteScript("window.vueAppState.nicknameApplyRequested = false;");
    std::string nickname = ui->GetAppState("nickname");

    if (nickname.empty())
    {
        PushNicknameStateToUI(ui, "invalid");
        return;
    }

    auto &netClient = screenManager->GetNetworkClientRef();
    if (!netClient.IsConnected())
    {
        PushNicknameStateToUI(ui, "offline");
        return;
    }

    m_waitingNicknameFetch = false;
    m_nicknameFetchTimer = 0.0f;
    netClient.SetDesiredNickname(nickname);
    netClient.SendNicknameUpdate(nickname);
}

void OptionsScreen::StartNicknameFetch()
{
    if (!screenManager || !screenManager->GetUILayer())
        return;

    auto *ui = screenManager->GetUILayer();
    auto &netClient = screenManager->GetNetworkClientRef();

    m_waitingNicknameFetch = false;
    m_nicknameFetchTimer = 0.0f;

    if (!netClient.IsConnected())
    {
        PushNicknameStateToUI(ui, "failed");
        return;
    }

    const std::string authoritative = netClient.GetAuthoritativeNickname();
    if (!authoritative.empty())
    {
        PushNicknameStateToUI(ui, "accepted", authoritative);
        return;
    }

    PushNicknameStateToUI(ui, "fetching");

    m_waitingNicknameFetch = true;
    m_nicknameFetchTimer = 0.0f;
    netClient.SendNicknameUpdate(netClient.GetDesiredNickname());
}

void OptionsScreen::UpdateNicknameFetch(float deltaTime)
{
    if (!m_waitingNicknameFetch || !screenManager || !screenManager->GetUILayer())
        return;

    auto *ui = screenManager->GetUILayer();
    auto &netClient = screenManager->GetNetworkClientRef();
    if (!netClient.IsConnected())
    {
        m_waitingNicknameFetch = false;
        m_nicknameFetchTimer = 0.0f;
        PushNicknameStateToUI(ui, "failed");
        return;
    }

    m_nicknameFetchTimer += deltaTime;
    if (m_nicknameFetchTimer >= NICKNAME_FETCH_TIMEOUT)
    {
        m_waitingNicknameFetch = false;
        m_nicknameFetchTimer = 0.0f;
        PushNicknameStateToUI(ui, "failed");
    }
}
