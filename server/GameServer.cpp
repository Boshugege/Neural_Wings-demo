// ────────────────────────────────────────────────────────────────────
// GameServer  –  nbnet server C++ wrapper
//
// The actual nbnet implementation (NBNET_IMPL) is compiled as C in
// nbnet_server_impl.c.  This file only uses declaration-level access.
// ────────────────────────────────────────────────────────────────────

extern "C"
{
#include <nbnet.h>
#include <net_drivers/udp.h>
    // TODO: #include <net_drivers/webrtc_c.h> when adding browser client support
}

#include "GameServer.h"

// ── Constants ──────────────────────────────────────────────────────
static constexpr const char *NW_PROTOCOL_NAME = "neural_wings";

static uint8_t MapChannel(uint8_t ourChannel)
{
    // our convention: 0 = reliable, 1 = unreliable
    return (ourChannel == 0) ? NBN_CHANNEL_RESERVED_RELIABLE : NBN_CHANNEL_RESERVED_UNRELIABLE;
}

// ── Lifecycle ──────────────────────────────────────────────────────

GameServer::~GameServer()
{
    Stop();
}

bool GameServer::Start(uint16_t port)
{
    // Register the UDP driver BEFORE starting.
    // NBN_Driver_Register asserts the driver isn't already registered,
    // and NBN_GameServer_Stop does NOT unregister drivers, so we must
    // only register once per process lifetime.
    static bool s_driverRegistered = false;
    if (!s_driverRegistered)
    {
        NBN_UDP_Register();
        s_driverRegistered = true;
    }

    if (NBN_GameServer_StartEx(NW_PROTOCOL_NAME, port, false) < 0)
    {
        std::cerr << "[GameServer] NBN_GameServer_StartEx failed on port "
                  << port << "\n";
        return false;
    }

    m_running = true;
    std::cout << "[GameServer] Started on port " << port << "\n";
    return true;
}

void GameServer::Stop()
{
    if (!m_running)
        return;

    m_running = false;

    NBN_GameServer_Stop();

    m_connIndex.clear();
    m_clients.clear();
    std::cout << "[GameServer] Stopped\n";
}

// ── Tick ───────────────────────────────────────────────────────────

void GameServer::Tick()
{
    if (!m_running)
        return;

    // 1. Poll all network events
    int ev;
    while ((ev = NBN_GameServer_Poll()) != NBN_NO_EVENT)
    {
        if (ev < 0)
        {
            std::cerr << "[GameServer] Poll error\n";
            break;
        }

        switch (ev)
        {
        case NBN_NEW_CONNECTION:
            HandleNewConnection();
            break;
        case NBN_CLIENT_DISCONNECTED:
            HandleClientDisconnected();
            break;
        case NBN_CLIENT_MESSAGE_RECEIVED:
            HandleClientMessage();
            break;
        }
    }

    // 2. Broadcast game state
    BroadcastPositions();

    // 3. Flush outgoing packets to all clients
    if (NBN_GameServer_SendPackets() < 0)
    {
        std::cerr << "[GameServer] SendPackets failed\n";
    }
}

// ── Connection events ──────────────────────────────────────────────

void GameServer::HandleNewConnection()
{
    NBN_ConnectionHandle conn = NBN_GameServer_GetIncomingConnection();

    // Always accept (authentication can be added later)
    NBN_GameServer_AcceptIncomingConnection();

    const ClientID newID = m_nextClientID++;

    ClientState state;
    state.id = newID;
    state.connHandle = conn;

    m_clients[newID] = state;
    m_connIndex[conn] = newID;

    std::cout << "[GameServer] Peer connected (awaiting Hello), assigned temp ClientID "
              << newID << "\n";
}

void GameServer::HandleClientDisconnected()
{
    NBN_ConnectionHandle conn = NBN_GameServer_GetDisconnectedClient();

    auto it = m_connIndex.find(conn);
    if (it == m_connIndex.end())
        return;

    RemoveClient(it->second, "disconnected");
}

void GameServer::HandleClientMessage()
{
    NBN_MessageInfo info = NBN_GameServer_GetMessageInfo();

    if (info.type != NBN_BYTE_ARRAY_MESSAGE_TYPE || !info.data)
        return;

    NBN_ByteArrayMessage *msg =
        static_cast<NBN_ByteArrayMessage *>(info.data);

    // Look up who sent it (info.sender is NBN_ConnectionHandle)
    auto it = m_connIndex.find(info.sender);
    if (it == m_connIndex.end())
        return;

    DispatchPacket(it->second, msg->bytes, msg->length);
}

// ── Packet dispatch ────────────────────────────────────────────────

void GameServer::DispatchPacket(ClientID clientID,
                                const uint8_t *data, size_t len)
{
    if (len < sizeof(NetPacketHeader))
        return;

    NetMessageType type = PacketSerializer::PeekType(data, len);
    switch (type)
    {
    case NetMessageType::ClientHello:
        HandleClientHello(clientID);
        break;
    case NetMessageType::PositionUpdate:
        HandlePositionUpdate(clientID, data, len);
        break;
    case NetMessageType::ClientDisconnect:
        HandleClientDisconnect(clientID);
        break;
    default:
        std::cerr << "[GameServer] Unknown message type "
                  << static_cast<int>(type) << "\n";
        break;
    }
}

// ── Handlers ───────────────────────────────────────────────────────

void GameServer::HandleClientHello(ClientID clientID)
{
    auto it = m_clients.find(clientID);
    if (it == m_clients.end() || it->second.welcomed)
        return;

    it->second.welcomed = true;
    SendWelcome(clientID);
    std::cout << "[GameServer] Assigned ClientID " << clientID << "\n";
}

void GameServer::HandlePositionUpdate(ClientID clientID,
                                      const uint8_t *data, size_t len)
{
    auto msg = PacketSerializer::Read<MsgPositionUpdate>(data, len);

    auto it = m_clients.find(clientID);
    if (it == m_clients.end())
        return;

    it->second.objectID = msg.objectID;
    it->second.lastTransform = msg.transform;
    it->second.hasTransform = true;
}

void GameServer::HandleClientDisconnect(ClientID clientID)
{
    RemoveClient(clientID, "requested disconnect");
}

// ── Sending helpers ────────────────────────────────────────────────

void GameServer::SendWelcome(ClientID clientID)
{
    auto pkt = PacketSerializer::WriteServerWelcome(clientID);
    SendTo(clientID, pkt.data(), pkt.size(), 0); // reliable
}

void GameServer::SendTo(ClientID clientID,
                        const uint8_t *data, size_t len, uint8_t channel)
{
    auto it = m_clients.find(clientID);
    if (it == m_clients.end())
        return;

    NBN_GameServer_SendByteArrayTo(
        it->second.connHandle,
        const_cast<uint8_t *>(data),
        static_cast<unsigned int>(len),
        MapChannel(channel));
}

void GameServer::RemoveClient(ClientID clientID, const char *reason)
{
    auto it = m_clients.find(clientID);
    if (it == m_clients.end())
        return;

    uint32_t connHandle = it->second.connHandle;
    m_clients.erase(it);
    m_connIndex.erase(connHandle);

    std::cout << "[GameServer] Client " << clientID << " " << reason << "\n";
}

// ── Broadcast ──────────────────────────────────────────────────────

void GameServer::BroadcastPositions()
{
    // Collect entries from all welcomed clients that have reported.
    std::vector<NetBroadcastEntry> entries;
    entries.reserve(m_clients.size());

    for (const auto &[id, cs] : m_clients)
    {
        (void)id;
        if (!cs.welcomed || !cs.hasTransform)
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

    for (auto &[id, cs] : m_clients)
    {
        (void)id;
        if (!cs.welcomed)
            continue;

        NBN_GameServer_SendByteArrayTo(
            cs.connHandle,
            pkt.data(),
            static_cast<unsigned int>(pkt.size()),
            NBN_CHANNEL_RESERVED_UNRELIABLE); // unreliable for position broadcast
    }
}
