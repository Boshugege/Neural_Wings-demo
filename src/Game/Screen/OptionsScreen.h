#pragma once
#include "Engine/System/Screen/IGameScreen.h"
#include "Engine/Config/EngineConfig.h"
#include "MyScreenState.h"

class OptionsScreen : public IGameScreen
{
public:
    OptionsScreen(ScreenManager *sm);
    ~OptionsScreen();

    void OnEnter() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Update(float deltaTime) override;
    void Draw() override;
    void OnExit() override;
    ScreenState GetNextScreenState() const override;
    ScreenState GetScreenState() const override;

private:
    ScreenState m_nextScreenState;
    bool m_pendingSync = false;

    EngineConfig m_currentConfig;  // 原始配置
    EngineConfig m_modifiedConfig; // 修改后配置
    void SaveConfig();
    void ApplyVueSettings(); // Apply settings from Vue to m_modifiedConfig
    void ApplyConfigToUI();
};
