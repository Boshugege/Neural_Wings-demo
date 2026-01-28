#include "OptionsScreen.h"
#include "raylib.h"
#include "Engine/System/Screen/ScreenManager.h"
#include "Game/Screen/MyScreenState.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
#include <cstdio>
#include <sstream>

OptionsScreen::OptionsScreen()
    : m_nextScreenState(SCREEN_STATE_NONE)
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

    if (screenManager && screenManager->GetUILayer())
    {
        auto *uiLayer = screenManager->GetUILayer();
        uiLayer->SetVisible(true);
        // Reset readiness so we don't sync against the previous page state.
        uiLayer->ExecuteScript(
            "window.vueAppState = window.vueAppState || {};"
            "window.vueAppState.vueAppReady = false;"
            "window.vueAppState.settingsSaveRequested = false;");
        uiLayer->LoadRoute(OPTIONS);
    }
}
void OptionsScreen::OnExit()
{
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

    if (IsKeyPressed(KEY_ESCAPE))
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

        printf("[OptionsScreen] Settings saved and applied: %dx%d, fullscreen=%s, fps=%.0f\n",
               m_modifiedConfig.screenWidth,
               m_modifiedConfig.screenHeight,
               m_modifiedConfig.fullScreen ? "true" : "false",
               m_modifiedConfig.targetFPS);

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

    std::ostringstream script;
    script << "if (window.__applyEngineSettings) { "
           << "window.__applyEngineSettings({fullscreen: " << (m_currentConfig.fullScreen ? "true" : "false")
           << ", resolution: \"" << resolution << "\", targetFPS: "
           << static_cast<int>(m_currentConfig.targetFPS) << "}); }";
    screenManager->GetUILayer()->ExecuteScript(script.str());
}
