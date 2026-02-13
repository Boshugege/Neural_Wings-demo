#pragma once
#include "Engine/Network/NetTypes.h"
#include "Engine/Network/Transport/ENetTransport.h"
#include "Engine/Network/Protocol/PacketSerializer.h"

#include <unordered_map>
#include <vector>
#include <iostream>

/// Minimal authoritative game server.
/// MVP scope: accept clients, store latest positions, broadcast every tick.
class GameServer
{
public:
    GameServer() = default;
    ~GameServer();

    /// Start listening. Returns false on failure.
    bool Start(uint16_t port = DEFAULT_SERVER_PORT);

    /// Run one server tick: poll network + broadcast positions.
    void Tick();

    /// Shut down gracefully.
    void Stop();

    bool IsRunning() const { return m_running; }

private:
    // ── Handlers ───────────────────────────────────────────────────
    void OnPeerConnected(ENetPeer *peer);
    void OnPeerDisconnected(ENetPeer *peer);
    void OnPacketReceived(ENetPeer *peer, const uint8_t *data,
                          size_t len, uint8_t channel);

    void HandleClientHello(ENetPeer *peer, const uint8_t *data, size_t len);
    void HandlePositionUpdate(ENetPeer *peer, const uint8_t *data, size_t len);
    void HandleClientDisconnect(ENetPeer *peer, const uint8_t *data, size_t len);

    void BroadcastPositions();

    // ── Data ───────────────────────────────────────────────────────
    ENetTransport m_transport;
    bool m_running = false;
    ClientID m_nextClientID = 1; // 0 is INVALID

    /// Per-client state stored on the server.
    struct ClientState
    {
        ClientID id = INVALID_CLIENT_ID;
        ENetPeer *peer = nullptr;
        NetObjectID objectID = INVALID_NET_OBJECT_ID;
        NetTransformState lastTransform{};
        bool hasTransform = false;
    };

    /// peer pointer → client state  (fast lookup on receive).
    std::unordered_map<ENetPeer *, ClientState> m_clients;
};
