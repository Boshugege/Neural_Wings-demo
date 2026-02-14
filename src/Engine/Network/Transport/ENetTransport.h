#pragma once
#include "Engine/Network/Transport/INetworkTransport.h"

// Prevent Windows.h min/max from breaking std::numeric_limits etc.
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include <enet/enet.h>

// Undo Windows macros that conflict with Raylib function names.
#if defined(_WIN32)
#ifdef CloseWindow
#undef CloseWindow
#endif
#ifdef ShowCursor
#undef ShowCursor
#endif
#endif

#include <iostream>

/// ENet-based transport (Desktop / Server).
/// Implements INetworkTransport for the client side.
/// Also exposes server-only helpers (Listen, Broadcast, SendTo with peer).
class ENetTransport : public INetworkTransport
{
public:
    ENetTransport() = default;
    ~ENetTransport() override;

    ENetTransport(const ENetTransport &) = delete;
    ENetTransport &operator=(const ENetTransport &) = delete;

    // ── INetworkTransport (client-facing) ──────────────────────────
    bool Connect(const std::string &host, uint16_t port) override;
    void Disconnect() override;
    void Poll(uint32_t timeoutMs = 0) override;
    void Send(const uint8_t *data, size_t len, uint8_t channel = 0) override;
    bool IsConnected() const override { return m_state == ConnectionState::Connected; }
    ConnectionState GetState() const override { return m_state; }
    void SetOnConnect(OnConnectFn fn) override { m_onConnect = std::move(fn); }
    void SetOnDisconnect(OnDisconnectFn fn) override { m_onDisconnect = std::move(fn); }
    void SetOnReceive(OnReceiveFn fn) override { m_onReceive = std::move(fn); }

    // ── Server-only extras (not part of interface) ─────────────────
    using ServerOnConnectFn = std::function<void(ENetPeer *)>;
    using ServerOnDisconnectFn = std::function<void(ENetPeer *)>;
    using ServerOnReceiveFn = std::function<void(ENetPeer *, const uint8_t *, size_t, uint8_t)>;

    bool Listen(uint16_t port, size_t maxClients = 32);
    void SendTo(ENetPeer *peer, const uint8_t *data, size_t len, uint8_t channel = 0);
    void SendTo(ENetPeer *peer, const std::vector<uint8_t> &data, uint8_t channel = 0);
    void Broadcast(const uint8_t *data, size_t len, uint8_t channel = 0);
    void Broadcast(const std::vector<uint8_t> &data, uint8_t channel = 0);
    ENetPeer *GetServerPeer() const { return m_serverPeer; }

    /// Server-side callbacks (include ENetPeer*). Setting these overrides
    /// the corresponding INetworkTransport callbacks for that event.
    void SetOnConnect(ServerOnConnectFn fn) { m_serverOnConnect = std::move(fn); }
    void SetOnDisconnect(ServerOnDisconnectFn fn) { m_serverOnDisconnect = std::move(fn); }
    void SetOnReceive(ServerOnReceiveFn fn) { m_serverOnReceive = std::move(fn); }

private:
    ENetHost *m_host = nullptr;
    ENetPeer *m_serverPeer = nullptr;
    ConnectionState m_state = ConnectionState::Disconnected;

    OnConnectFn m_onConnect;
    OnDisconnectFn m_onDisconnect;
    OnReceiveFn m_onReceive;

    ServerOnConnectFn m_serverOnConnect;
    ServerOnDisconnectFn m_serverOnDisconnect;
    ServerOnReceiveFn m_serverOnReceive;
    bool InitENet();
    void Cleanup();
};
