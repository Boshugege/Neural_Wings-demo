#pragma once
#include "Engine/Network/NetTypes.h"
#include <vector>
#include <string>
#include <functional>
#include <cstdint>

/// Platform-agnostic transport interface.
/// Concrete implementation: NBNetTransport (nbnet + UDP on desktop, nbnet + WebRTC on web).
/// NetworkClient depends ONLY on this interface — never on a concrete transport.
class INetworkTransport
{
public:
    // ── Callback signatures (no platform types leak out) ───────────
    using OnConnectFn = std::function<void()>;
    using OnDisconnectFn = std::function<void()>;
    using OnReceiveFn = std::function<void(const uint8_t *data, size_t len, uint8_t channelID)>;

    virtual ~INetworkTransport() = default;

    // ── Client-side ────────────────────────────────────────────────
    virtual bool Connect(const std::string &host, uint16_t port) = 0;
    virtual void Disconnect() = 0;

    // ── Runtime ────────────────────────────────────────────────────
    virtual void Poll(uint32_t timeoutMs = 0) = 0;

    /// Send to the server (client-side) or to a specific internal peer.
    /// `channel` 0 = reliable, 1 = unreliable.
    virtual void Send(const uint8_t *data, size_t len, uint8_t channel = 0) = 0;
    void Send(const std::vector<uint8_t> &data, uint8_t channel = 0)
    {
        Send(data.data(), data.size(), channel);
    }

    // ── State ──────────────────────────────────────────────────────
    virtual bool IsConnected() const = 0;
    virtual ConnectionState GetState() const = 0;

    // ── Callbacks ──────────────────────────────────────────────────
    virtual void SetOnConnect(OnConnectFn fn) = 0;
    virtual void SetOnDisconnect(OnDisconnectFn fn) = 0;
    virtual void SetOnReceive(OnReceiveFn fn) = 0;
};
