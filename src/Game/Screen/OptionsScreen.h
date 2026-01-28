#pragma once
#include "Engine/System/Screen/GameScreen.h"
#include "Engine/Config/EngineConfig.h"
#include "ScreenState.h"

class OptionsScreen : public GameScreen
{
public:
    OptionsScreen();
    ~OptionsScreen();

    void OnEnter() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Update(float deltaTime) override;
    void Draw() override;
    void OnExit() override;
    int GetNextScreenState() const override;
    int GetScreenState() const override;

private:
    int m_nextScreenState;
    bool m_pendingSync = false;

    EngineConfig m_currentConfig;  // 原始配置
    EngineConfig m_modifiedConfig; // 修改后配置
    void SaveConfig();
    void ApplyVueSettings(); // Apply settings from Vue to m_modifiedConfig
    void ApplyConfigToUI();
};
