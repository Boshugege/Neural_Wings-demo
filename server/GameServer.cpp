#include "GameServer.h"

// ── Lifecycle ──────────────────────────────────────────────────────

GameServer::~GameServer()
{
    Stop();
}

bool GameServer::Start(uint16_t port)
{
    m_transport.SetOnConnect([this](ENetPeer *p)
                             { OnPeerConnected(p); });
    m_transport.SetOnDisconnect([this](ENetPeer *p)
                                { OnPeerDisconnected(p); });
    m_transport.SetOnReceive([this](ENetPeer *p, const uint8_t *d, size_t l, uint8_t ch)
                             { OnPacketReceived(p, d, l, ch); });

    if (!m_transport.Listen(port))
        return false;

    m_running = true;
    std::cout << "[GameServer] Started on port " << port << "\n";
    return true;
}

void GameServer::Stop()
{
    m_running = false;
    m_transport.Disconnect();
    m_clients.clear();
    std::cout << "[GameServer] Stopped\n";
}

// ── Tick ───────────────────────────────────────────────────────────

void GameServer::Tick()
{
    if (!m_running)
        return;
    m_transport.Poll(0);
    BroadcastPositions();
}

// ── Connection callbacks ───────────────────────────────────────────

void GameServer::OnPeerConnected(ENetPeer *peer)
{
    std::cout << "[GameServer] Peer connected (awaiting Hello)\n";
    // Don't assign a ClientID yet — wait for MsgClientHello.
}

void GameServer::OnPeerDisconnected(ENetPeer *peer)
{
    auto it = m_clients.find(peer);
    if (it != m_clients.end())
    {
        std::cout << "[GameServer] Client " << it->second.id
                  << " disconnected\n";
        m_clients.erase(it);
    }
}

// ── Packet dispatch ────────────────────────────────────────────────

void GameServer::OnPacketReceived(ENetPeer *peer, const uint8_t *data,
                                  size_t len, uint8_t /*channel*/)
{
    if (len < sizeof(NetPacketHeader))
        return;

    NetMessageType type = PacketSerializer::PeekType(data, len);

    switch (type)
    {
    case NetMessageType::ClientHello:
        HandleClientHello(peer, data, len);
        break;
    case NetMessageType::PositionUpdate:
        HandlePositionUpdate(peer, data, len);
        break;
    case NetMessageType::ClientDisconnect:
        HandleClientDisconnect(peer, data, len);
        break;
    default:
        std::cerr << "[GameServer] Unknown message type "
                  << static_cast<int>(type) << "\n";
        break;
    }
}

// ── Handlers ───────────────────────────────────────────────────────

void GameServer::HandleClientHello(ENetPeer *peer,
                                   const uint8_t * /*data*/, size_t /*len*/)
{
    ClientID newID = m_nextClientID++;
    ClientState state;
    state.id = newID;
    state.peer = peer;
    m_clients[peer] = state;

    // Send welcome.
    auto pkt = PacketSerializer::WriteServerWelcome(newID);
    m_transport.SendTo(peer, pkt, 0); // reliable

    std::cout << "[GameServer] Assigned ClientID " << newID << "\n";
}

void GameServer::HandlePositionUpdate(ENetPeer *peer,
                                      const uint8_t *data, size_t len)
{
    auto msg = PacketSerializer::Read<MsgPositionUpdate>(data, len);

    auto it = m_clients.find(peer);
    if (it == m_clients.end())
        return; // unknown peer

    it->second.objectID = msg.objectID;
    it->second.lastTransform = msg.transform;
    it->second.hasTransform = true;
}

void GameServer::HandleClientDisconnect(ENetPeer *peer,
                                        const uint8_t * /*data*/, size_t /*len*/)
{
    auto it = m_clients.find(peer);
    if (it != m_clients.end())
    {
        std::cout << "[GameServer] Client " << it->second.id
                  << " requested disconnect\n";
        m_clients.erase(it);
    }
    enet_peer_disconnect(peer, 0);
}

// ── Broadcast ──────────────────────────────────────────────────────

void GameServer::BroadcastPositions()
{
    // Collect entries from all clients that have reported at least once.
    std::vector<NetBroadcastEntry> entries;
    entries.reserve(m_clients.size());

    for (auto &[_, cs] : m_clients)
    {
        if (!cs.hasTransform)
            continue;
        NetBroadcastEntry e;
        e.clientID = cs.id;
        e.objectID = cs.objectID;
        e.transform = cs.lastTransform;
        entries.push_back(e);
    }

    if (entries.empty())
        return;

    auto pkt = PacketSerializer::WritePositionBroadcast(entries);
    m_transport.Broadcast(pkt, 1); // unreliable channel
}
