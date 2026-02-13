#pragma once
#include "Engine/Network/NetTypes.h"
#include "Engine/Network/Protocol/PacketSerializer.h"
#include <vector>
#include <functional>
#include <string>
#include <memory>

// Forward-declare to avoid leaking enet/windows headers.
class ENetTransport;
struct _ENetPeer; // ENetPeer is a typedef for struct _ENetPeer

/// High-level client-side network manager.
/// Owns an ENetTransport, handles the handshake, and exposes
/// typed send helpers + an incoming-message callback table.
class NetworkClient
{
public:
    /// Callback: server sent us a position broadcast.
    using OnPositionBroadcastFn =
        std::function<void(const std::vector<NetBroadcastEntry> &entries)>;

    NetworkClient();
    ~NetworkClient();

    // Non-copyable (owns transport).
    NetworkClient(const NetworkClient &) = delete;
    NetworkClient &operator=(const NetworkClient &) = delete;

    // ── Lifecycle ──────────────────────────────────────────────────

    /// Attempt to connect to a server.  Non-blocking.
    bool Connect(const std::string &host = DEFAULT_SERVER_HOST,
                 uint16_t port = DEFAULT_SERVER_PORT);

    /// Gracefully disconnect.
    void Disconnect();

    /// Pump the network.  Call once per game frame.
    void Poll();

    // ── State ──────────────────────────────────────────────────────

    bool IsConnected() const;
    ClientID GetLocalClientID() const { return m_localClientID; }

    // ── Sending ────────────────────────────────────────────────────

    /// Send our local object's transform to the server (unreliable channel).
    void SendPositionUpdate(NetObjectID objectID,
                            const NetTransformState &transform);

    // ── Callbacks ──────────────────────────────────────────────────

    void SetOnPositionBroadcast(OnPositionBroadcastFn fn)
    {
        m_onPositionBroadcast = std::move(fn);
    }

private:
    void OnRawReceive(_ENetPeer *peer, const uint8_t *data,
                      size_t len, uint8_t channelID);

    std::unique_ptr<ENetTransport> m_transport;
    ClientID m_localClientID = INVALID_CLIENT_ID;

    OnPositionBroadcastFn m_onPositionBroadcast;
};
