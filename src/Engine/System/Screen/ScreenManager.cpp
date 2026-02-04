#include "ScreenManager.h"
#include "raylib.h"
#include "ScreenState.h"

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

ScreenManager::ScreenManager(const EngineConfig &config, std::unique_ptr<ScreenFactory> factory)
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
}

ScreenManager::~ScreenManager()
{
    Shutdown();
}
void ScreenManager::ApplySettings(const EngineConfig &newConfig)
{
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
}

const EngineConfig &ScreenManager::GetActiveConfig() const
{
    return m_activeConfig;
}

UILayer *ScreenManager::GetUILayer()
{
    return m_uiLayer.get();
}

bool ScreenManager::UpdateFrame()
{
    if (WindowShouldClose() || m_currentScreen->GetNextScreenState() == SCREEN_STATE_EXIT)
    {
        return false;
    }
    m_timeManager.Tick();
    m_accumulator += m_timeManager.GetDeltaTime();

    while (m_accumulator >= m_timeManager.GetFixedDeltaTime())
    {
        m_currentScreen->FixedUpdate(m_timeManager.GetFixedDeltaTime());
        m_accumulator -= m_timeManager.GetFixedDeltaTime();
    }

    m_currentScreen->Update(m_timeManager.GetDeltaTime());
    if (m_uiLayer)
    {

        m_uiLayer->Resize(
            static_cast<uint32_t>(GetScreenWidth()),
            static_cast<uint32_t>(GetScreenHeight()));
        m_uiLayer->HandleInput();
        m_uiLayer->Update();
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
    if (m_uiLayer)
    {
        m_uiLayer->Shutdown();
        m_uiLayer.reset();
    }
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
