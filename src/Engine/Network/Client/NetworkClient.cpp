#include "NetworkClient.h"
#include "Engine/Network/Transport/ENetTransport.h"
#include <iostream>

// ────────────────────────────────────────────────────────────────────
NetworkClient::NetworkClient()
    : m_transport(std::make_unique<ENetTransport>())
{
}

NetworkClient::~NetworkClient()
{
    Disconnect();
}

// ── Connect ────────────────────────────────────────────────────────
bool NetworkClient::Connect(const std::string &host, uint16_t port)
{
    // Wire up transport callbacks.
    m_transport->SetOnConnect([this](ENetPeer * /*peer*/)
                              {
        std::cout << "[NetworkClient] Connected – sending Hello\n";
        auto pkt = PacketSerializer::WriteClientHello();
        m_transport->SendTo(m_transport->GetServerPeer(), pkt, 0); });

    m_transport->SetOnDisconnect([this](ENetPeer * /*peer*/)
                                 {
        std::cout << "[NetworkClient] Disconnected from server\n";
        m_localClientID = INVALID_CLIENT_ID; });

    m_transport->SetOnReceive([this](ENetPeer *peer, const uint8_t *data,
                                     size_t len, uint8_t ch)
                              { OnRawReceive(peer, data, len, ch); });

    return m_transport->Connect(host, port);
}

// ── Disconnect ─────────────────────────────────────────────────────
void NetworkClient::Disconnect()
{
    if (m_localClientID != INVALID_CLIENT_ID)
    {
        auto pkt = PacketSerializer::WriteClientDisconnect(m_localClientID);
        m_transport->SendTo(m_transport->GetServerPeer(), pkt, 0);
    }
    m_transport->Disconnect();
    m_localClientID = INVALID_CLIENT_ID;
}

// ── Poll ───────────────────────────────────────────────────────────
void NetworkClient::Poll()
{
    m_transport->Poll(0);
}

bool NetworkClient::IsConnected() const
{
    return m_transport->IsConnected() && m_localClientID != INVALID_CLIENT_ID;
}

// ── Sending ────────────────────────────────────────────────────────
void NetworkClient::SendPositionUpdate(NetObjectID objectID,
                                       const NetTransformState &transform)
{
    if (!IsConnected())
        return;
    auto pkt = PacketSerializer::WritePositionUpdate(
        m_localClientID, objectID, transform);
    // Position updates use the unreliable channel (1) for low latency.
    m_transport->SendTo(m_transport->GetServerPeer(), pkt, 1);
}

// ── Incoming dispatch ──────────────────────────────────────────────
void NetworkClient::OnRawReceive(_ENetPeer * /*peer*/,
                                 const uint8_t *data, size_t len,
                                 uint8_t /*channelID*/)
{
    if (len < sizeof(NetPacketHeader))
        return;

    NetMessageType type = PacketSerializer::PeekType(data, len);

    switch (type)
    {
    case NetMessageType::ServerWelcome:
    {
        auto msg = PacketSerializer::Read<MsgServerWelcome>(data, len);
        m_localClientID = msg.assignedClientID;
        std::cout << "[NetworkClient] Received Welcome – my ClientID = "
                  << m_localClientID << "\n";
        break;
    }
    case NetMessageType::PositionBroadcast:
    {
        auto entries = PacketSerializer::ReadBroadcastEntries(data, len);
        if (m_onPositionBroadcast)
            m_onPositionBroadcast(entries);
        break;
    }
    default:
        std::cerr << "[NetworkClient] Unknown message type: "
                  << static_cast<int>(type) << "\n";
        break;
    }
}
