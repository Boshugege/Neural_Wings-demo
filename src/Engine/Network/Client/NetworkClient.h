#pragma once
#include "Engine/Network/NetTypes.h"
#include "Engine/Network/Protocol/PacketSerializer.h"
#include <vector>
#include <functional>
#include <string>
#include <memory>

class INetworkTransport;

/// High-level client-side network manager.
/// Depends ONLY on INetworkTransport — no platform #if anywhere.
class NetworkClient
{
public:
    using OnPositionBroadcastFn =
        std::function<void(const std::vector<NetBroadcastEntry> &entries)>;

    NetworkClient();
    ~NetworkClient();

    NetworkClient(const NetworkClient &) = delete;
    NetworkClient &operator=(const NetworkClient &) = delete;

    // ── Lifecycle ──────────────────────────────────────────────────
    bool Connect(const std::string &host = DEFAULT_SERVER_HOST,
                 uint16_t port = DEFAULT_SERVER_PORT);
    void Disconnect();
    void Poll();

    // ── State ──────────────────────────────────────────────────────
    bool IsConnected() const;
    ClientID GetLocalClientID() const { return m_localClientID; }

    // ── Sending ────────────────────────────────────────────────────
    void SendPositionUpdate(NetObjectID objectID,
                            const NetTransformState &transform);

    // ── Callbacks ──────────────────────────────────────────────────
    void SetOnPositionBroadcast(OnPositionBroadcastFn fn)
    {
        m_onPositionBroadcast = std::move(fn);
    }

private:
    void OnRawReceive(const uint8_t *data, size_t len, uint8_t channelID);

    std::unique_ptr<INetworkTransport> m_transport;
    ClientID m_localClientID = INVALID_CLIENT_ID;
    OnPositionBroadcastFn m_onPositionBroadcast;
};
