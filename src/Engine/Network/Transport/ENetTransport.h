#pragma once
#include "Engine/Network/NetTypes.h"

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

#include <vector>
#include <string>
#include <functional>
#include <cstdint>
#include <iostream>

/// Thin RAII wrapper around ENet that works for both client and server.
/// It hides the raw enet API behind Connect / Listen / Send / Poll.
class ENetTransport
{
public:
    /// Callbacks the owner can bind.
    using OnConnectFn = std::function<void(ENetPeer *peer)>;
    using OnDisconnectFn = std::function<void(ENetPeer *peer)>;
    using OnReceiveFn = std::function<void(ENetPeer *peer, const uint8_t *data, size_t len, uint8_t channelID)>;

    ENetTransport() = default;
    ~ENetTransport();

    ENetTransport(const ENetTransport &) = delete;
    ENetTransport &operator=(const ENetTransport &) = delete;

    // ── Initialisation ─────────────────────────────────────────────

    /// Create a **server** host that listens on `port`.
    bool Listen(uint16_t port, size_t maxClients = 32);

    /// Create a **client** host and connect to `host:port`.
    bool Connect(const std::string &host, uint16_t port);

    // ── Runtime ────────────────────────────────────────────────────

    /// Pump ENet events.  Call once per frame (or server tick).
    void Poll(uint32_t timeoutMs = 0);

    /// Send raw bytes to a specific peer.
    /// `channel` 0 = reliable-ordered, 1 = unreliable-unsequenced.
    void SendTo(ENetPeer *peer, const uint8_t *data, size_t len, uint8_t channel = 0);
    void SendTo(ENetPeer *peer, const std::vector<uint8_t> &data, uint8_t channel = 0);

    /// Broadcast to all connected peers.
    void Broadcast(const uint8_t *data, size_t len, uint8_t channel = 0);
    void Broadcast(const std::vector<uint8_t> &data, uint8_t channel = 0);

    // ── Teardown ───────────────────────────────────────────────────

    void Disconnect();
    bool IsConnected() const { return m_state == ConnectionState::Connected; }
    ConnectionState GetState() const { return m_state; }

    /// Get the server-side peer (only valid on a client transport).
    ENetPeer *GetServerPeer() const { return m_serverPeer; }

    // ── Callbacks ──────────────────────────────────────────────────

    void SetOnConnect(OnConnectFn fn) { m_onConnect = std::move(fn); }
    void SetOnDisconnect(OnDisconnectFn fn) { m_onDisconnect = std::move(fn); }
    void SetOnReceive(OnReceiveFn fn) { m_onReceive = std::move(fn); }

private:
    ENetHost *m_host = nullptr;
    ENetPeer *m_serverPeer = nullptr; // client-side only
    ConnectionState m_state = ConnectionState::Disconnected;

    OnConnectFn m_onConnect;
    OnDisconnectFn m_onDisconnect;
    OnReceiveFn m_onReceive;

    bool InitENet();
    void Cleanup();
};
