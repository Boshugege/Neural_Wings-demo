#pragma once
#include "GameScreen.h"
#include "ScreenFactory.h"
#include "Engine/Config/EngineConfig.h"
#include "Engine/System/Time/TimeManager.h"
#include "Engine/UI/UI.h"

#include <memory>

class ScreenManager
{
public:
    ScreenManager(const EngineConfig &config, std::unique_ptr<ScreenFactory> factory);
    ~ScreenManager();
    // void Run();

    void ApplySettings(const EngineConfig &config);
    const EngineConfig &GetActiveConfig() const;
    UILayer *GetUILayer();

    bool UpdateFrame();
    void Shutdown();

private:
    void ChangeScreen(int newState);

    std::unique_ptr<GameScreen> m_currentScreen;
    std::unique_ptr<ScreenFactory> m_factory;

    std::unique_ptr<UILayer> m_uiLayer;

    TimeManager m_timeManager;
    float m_accumulator;

    // Track the last applied runtime configuration so screens can sync from
    // the actual window state instead of only the config file on disk.
    EngineConfig m_activeConfig;
};
