#pragma once
#include "Engine/Network/Transport/INetworkTransport.h"

/// Unified client transport powered by nbnet.
/// Desktop builds use the UDP driver; Web/WASM builds use the WebRTC driver.
/// The driver selection is compile-time (in NBNetTransport.cpp) – this header
/// is completely platform-agnostic.
class NBNetTransport : public INetworkTransport
{
public:
    NBNetTransport() = default;
    ~NBNetTransport() override;

    NBNetTransport(const NBNetTransport &) = delete;
    NBNetTransport &operator=(const NBNetTransport &) = delete;

    // ── INetworkTransport ──────────────────────────────────────────
    bool Connect(const std::string &host, uint16_t port) override;
    void Disconnect() override;
    void Poll(uint32_t timeoutMs = 0) override;
    void Send(const uint8_t *data, size_t len, uint8_t channel = 0) override;

    bool IsConnected() const override { return m_state == ConnectionState::Connected; }
    ConnectionState GetState() const override { return m_state; }

    void SetOnConnect(OnConnectFn fn) override { m_onConnect = std::move(fn); }
    void SetOnDisconnect(OnDisconnectFn fn) override { m_onDisconnect = std::move(fn); }
    void SetOnReceive(OnReceiveFn fn) override { m_onReceive = std::move(fn); }

private:
    OnConnectFn m_onConnect;
    OnDisconnectFn m_onDisconnect;
    OnReceiveFn m_onReceive;
    ConnectionState m_state = ConnectionState::Disconnected;
    bool m_started = false;
};
