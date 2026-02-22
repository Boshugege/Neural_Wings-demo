#pragma once
#include "IGameScreen.h"
#include "ScreenFactory.h"
#include "Engine/Config/EngineConfig.h"
#include "Engine/System/Time/TimeManager.h"
#include "Engine/System/Resource/ResourceManager.h"
#include "Engine/System/Audio/AudioManager.h"
#include "Engine/UI/UI.h"
#include "Engine/Network/Client/NetworkClient.h"
#include "Engine/Network/Client/ClientIdentity.h"
#include "Engine/Network/Protocol/Messages.h"
#include "Engine/Network/Chat/ChatManager.h"

#include <string>
#include <memory>
#include <deque>

class ScreenManager
{
public:
    ScreenManager(const EngineConfig &config, const std::string audioPath, std::unique_ptr<ScreenFactory> factory);
    ~ScreenManager();
    // void Run();

    void ApplySettings(const EngineConfig &config);
    const EngineConfig &GetActiveConfig() const;
    UILayer *GetUILayer();

    /// Global NetworkClient shared across all screens.
    std::shared_ptr<NetworkClient> GetNetworkClient() { return m_networkClient; }
    NetworkClient &GetNetworkClientRef() { return *m_networkClient; }

    bool UpdateFrame();
    void Shutdown();

    ResourceManager &GetResourceManager();
    AudioManager &GetAudioManager();

private:
    void ChangeScreen(int newState);
    void PushChatMessageToUI(ChatMessageType type, ClientID senderID,
                             const std::string &senderName, const std::string &text);
    void FlushPendingChatToUI();
    void PollGlobalChatSendRequest();

    // ── Chat send queue with rate limiting ──────────────────────────
    std::deque<std::string> m_chatSendQueue;
    static constexpr size_t CHAT_SEND_QUEUE_MAX = 64;
    float m_chatSendCooldown = 0.0f;
    static constexpr float CHAT_SEND_INTERVAL = 0.30f; // 300 ms
    float m_heartbeatCooldown = 0.0f;
    static constexpr float HEARTBEAT_INTERVAL = 1.0f; // 1 s, for menu/options keep-alive

    std::unique_ptr<ResourceManager> m_resourceManager;
    std::unique_ptr<AudioManager> m_audioManager;

    std::unique_ptr<IGameScreen> m_currentScreen;
    std::unique_ptr<ScreenFactory> m_factory;

    std::unique_ptr<UILayer> m_uiLayer;

    /// Persistent network client — lives as long as ScreenManager.
    std::shared_ptr<NetworkClient> m_networkClient;
    ClientIdentity m_clientIdentity;

    TimeManager m_timeManager;
    float m_accumulator;
    std::deque<ChatEntry> m_pendingChatToUI;

    // Track the last applied runtime configuration so screens can sync from
    // the actual window state instead of only the config file on disk.
    EngineConfig m_activeConfig;
};
