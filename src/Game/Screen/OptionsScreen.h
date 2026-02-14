#pragma once
#include "Engine/System/Screen/GameScreen.h"
#include "Engine/Config/EngineConfig.h"
#include "Engine/Network/Client/NetworkClient.h"
#include "MyScreenState.h"
#include <memory>

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

    // ── Server ping check ──────────────────────────────────────────
    enum class PingState
    {
        Idle,
        Connecting,
        Done
    };
    PingState m_pingState = PingState::Idle;
    std::unique_ptr<NetworkClient> m_pingClient;
    float m_pingTimer = 0.0f;
    static constexpr float PING_TIMEOUT = 3.0f;
    void UpdatePingCheck(float deltaTime);
};
