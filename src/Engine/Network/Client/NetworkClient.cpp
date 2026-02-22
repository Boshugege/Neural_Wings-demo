#include "NetworkClient.h"
#include "Engine/Network/Transport/NBNetTransport.h"
#include <iostream>
#include <utility>

// ────────────────────────────────────────────────────────────────────
NetworkClient::NetworkClient()
    : m_transport(std::make_unique<NBNetTransport>())
{
}

NetworkClient::~NetworkClient()
{
    Disconnect();
}

// ── Connect ────────────────────────────────────────────────────────
bool NetworkClient::Connect(const std::string &host, uint16_t port)
{
    ConnectionState state = m_transport->GetState();
    if (state == ConnectionState::Connecting || state == ConnectionState::Connected)
        return true;

    m_transport->SetOnConnect([this]()
                              {
        std::cout << "[NetworkClient] Connected – sending Hello with UUID\n";
        auto pkt = PacketSerializer::WriteClientHello(m_uuid);
        m_transport->Send(pkt, 0); });

    m_transport->SetOnDisconnect([this]()
                                 {
        std::cout << "[NetworkClient] Disconnected from server\n";
        m_localClientID = INVALID_CLIENT_ID;
        if (!m_playerMeta.empty())
        {
            m_playerMeta.clear();
            if (m_onPlayerMetaChanged)
                m_onPlayerMetaChanged();
        } });

    m_transport->SetOnReceive([this](const uint8_t *data, size_t len, uint8_t ch)
                              { OnRawReceive(data, len, ch); });

    return m_transport->Connect(host, port);
}

// ── Disconnect ─────────────────────────────────────────────────────
void NetworkClient::Disconnect()
{
    if (m_localClientID != INVALID_CLIENT_ID)
    {
        auto pkt = PacketSerializer::WriteClientDisconnect(m_localClientID);
        m_transport->Send(pkt, 0);
        m_transport->FlushSend(); // ensure the disconnect packet is actually transmitted
    }
    m_transport->Disconnect();
    m_localClientID = INVALID_CLIENT_ID;
    if (!m_playerMeta.empty())
    {
        m_playerMeta.clear();
        if (m_onPlayerMetaChanged)
            m_onPlayerMetaChanged();
    }
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

ConnectionState NetworkClient::GetConnectionState() const
{
    return m_transport->GetState();
}

// ── Sending ────────────────────────────────────────────────────────
void NetworkClient::SendPositionUpdate(NetObjectID objectID,
                                       const NetTransformState &transform)
{
    if (!IsConnected())
        return;
    auto pkt = PacketSerializer::WritePositionUpdate(
        m_localClientID, objectID, transform);
    m_transport->Send(pkt, 1); // unreliable channel
}

void NetworkClient::SendObjectRelease(NetObjectID objectID)
{
    if (!IsConnected())
        return;
    auto pkt = PacketSerializer::WriteObjectRelease(m_localClientID, objectID);
    m_transport->Send(pkt, 0); // reliable channel
    m_transport->FlushSend();  // ensure it goes out immediately
}

void NetworkClient::SendHeartbeat()
{
    if (!IsConnected())
        return;
    auto pkt = PacketSerializer::WriteHeartbeat(m_localClientID);
    m_transport->Send(pkt, 0); // reliable keep-alive
}

bool NetworkClient::SendChatMessage(ChatMessageType chatType,
                                    const std::string &text,
                                    ClientID targetID)
{
    if (!IsConnected())
        return false;
    auto pkt = PacketSerializer::WriteChatRequest(chatType, targetID, text);
    return m_transport->Send(pkt, 0); // reliable channel
}

void NetworkClient::SendNicknameUpdate(const std::string &nickname)
{
    if (!IsConnected())
        return;
    auto pkt = PacketSerializer::WriteNicknameUpdateRequest(nickname);
    m_transport->Send(pkt, 0); // reliable channel
}

void NetworkClient::FlushSend()
{
    m_transport->FlushSend();
}

std::string NetworkClient::GetPlayerNickname(ClientID clientID) const
{
    auto it = m_playerMeta.find(clientID);
    if (it == m_playerMeta.end())
        return "";
    return it->second.nickname;
}

// ── Incoming dispatch ──────────────────────────────────────────────
void NetworkClient::OnRawReceive(const uint8_t *data, size_t len,
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
        if (!m_desiredNickname.empty())
            SendNicknameUpdate(m_desiredNickname);
        break;
    }
    case NetMessageType::PositionBroadcast:
    {
        auto packet = PacketSerializer::ReadPositionBroadcast(data, len);
        if (m_onPositionBroadcast)
            m_onPositionBroadcast(packet.serverTick, packet.entries);
        break;
    }
    case NetMessageType::ObjectDespawn:
    {
        auto msg = PacketSerializer::Read<MsgObjectDespawn>(data, len);
        if (m_onObjectDespawn)
            m_onObjectDespawn(msg.ownerClientID, msg.objectID);
        break;
    }
    case NetMessageType::ChatBroadcast:
    {
        auto chat = PacketSerializer::ReadChatBroadcast(data, len);
        if (m_onChatMessage)
            m_onChatMessage(chat.chatType, chat.senderClientID,
                            chat.senderName, chat.text);
        break;
    }
    case NetMessageType::NicknameUpdateResult:
    {
        auto result = PacketSerializer::ReadNicknameUpdateResult(data, len);
        m_authoritativeNickname = result.nickname;

        bool changed = false;
        if (result.status == NicknameUpdateStatus::Accepted &&
            m_localClientID != INVALID_CLIENT_ID &&
            !result.nickname.empty())
        {
            auto &slot = m_playerMeta[m_localClientID];
            if (slot.clientID != m_localClientID || slot.nickname != result.nickname)
                changed = true;
            slot.clientID = m_localClientID;
            slot.nickname = result.nickname;
        }

        if (m_onNicknameUpdateResult)
            m_onNicknameUpdateResult(result.status, result.nickname);
        if (changed && m_onPlayerMetaChanged)
            m_onPlayerMetaChanged();
        break;
    }
    case NetMessageType::PlayerMetaSnapshot:
    {
        auto snapshot = PacketSerializer::ReadPlayerMetaSnapshot(data, len);
        m_playerMeta.clear();
        for (const auto &entry : snapshot.entries)
        {
            if (entry.clientID == INVALID_CLIENT_ID)
                continue;
            PlayerMeta meta;
            meta.clientID = entry.clientID;
            meta.nickname = entry.nickname;
            m_playerMeta[entry.clientID] = std::move(meta);
        }
        if (m_onPlayerMetaChanged)
            m_onPlayerMetaChanged();
        break;
    }
    case NetMessageType::PlayerMetaUpsert:
    {
        auto entry = PacketSerializer::ReadPlayerMetaUpsert(data, len);
        if (entry.clientID == INVALID_CLIENT_ID)
            break;

        bool changed = false;
        auto &slot = m_playerMeta[entry.clientID];
        if (slot.clientID != entry.clientID || slot.nickname != entry.nickname)
            changed = true;
        slot.clientID = entry.clientID;
        slot.nickname = entry.nickname;

        if (changed && m_onPlayerMetaChanged)
            m_onPlayerMetaChanged();
        break;
    }
    case NetMessageType::PlayerMetaRemove:
    {
        ClientID removedClientID = PacketSerializer::ReadPlayerMetaRemove(data, len);
        if (removedClientID == INVALID_CLIENT_ID)
            break;

        const size_t erased = m_playerMeta.erase(removedClientID);
        if (erased > 0 && m_onPlayerMetaChanged)
            m_onPlayerMetaChanged();
        break;
    }
    default:
        std::cerr << "[NetworkClient] Unknown message type: "
                  << static_cast<int>(type) << "\n";
        break;
    }
}
