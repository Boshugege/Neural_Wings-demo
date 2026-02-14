#include "ENetTransport.h"
#include <cstring>

// ── Static init guard ──────────────────────────────────────────────
static bool s_enetInitialised = false;

bool ENetTransport::InitENet()
{
    if (!s_enetInitialised)
    {
        if (enet_initialize() != 0)
        {
            std::cerr << "[ENetTransport] enet_initialize failed!\n";
            return false;
        }
        s_enetInitialised = true;
    }
    return true;
}

// ── Destructor ─────────────────────────────────────────────────────
ENetTransport::~ENetTransport()
{
    Cleanup();
}

void ENetTransport::Cleanup()
{
    if (m_serverPeer)
    {
        enet_peer_disconnect_now(m_serverPeer, 0);
        m_serverPeer = nullptr;
    }
    if (m_host)
    {
        enet_host_destroy(m_host);
        m_host = nullptr;
    }
    m_state = ConnectionState::Disconnected;
}

// ── Server: Listen ─────────────────────────────────────────────────
bool ENetTransport::Listen(uint16_t port, size_t maxClients)
{
    if (!InitENet())
        return false;

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    m_host = enet_host_create(&address, maxClients, NET_CHANNEL_COUNT, 0, 0);
    if (!m_host)
    {
        std::cerr << "[ENetTransport] Failed to create server host on port "
                  << port << "\n";
        return false;
    }
    m_state = ConnectionState::Connected;
    std::cout << "[ENetTransport] Server listening on port " << port << "\n";
    return true;
}

// ── Client: Connect ────────────────────────────────────────────────
bool ENetTransport::Connect(const std::string &host, uint16_t port)
{
    if (!InitENet())
        return false;

    m_host = enet_host_create(nullptr, 1, NET_CHANNEL_COUNT, 0, 0);
    if (!m_host)
    {
        std::cerr << "[ENetTransport] Failed to create client host\n";
        return false;
    }

    ENetAddress address;
    enet_address_set_host(&address, host.c_str());
    address.port = port;

    m_serverPeer = enet_host_connect(m_host, &address, NET_CHANNEL_COUNT, 0);
    if (!m_serverPeer)
    {
        std::cerr << "[ENetTransport] enet_host_connect failed\n";
        Cleanup();
        return false;
    }

    m_state = ConnectionState::Connecting;
    std::cout << "[ENetTransport] Connecting to " << host << ":" << port << " ...\n";
    return true;
}

// ── Poll ───────────────────────────────────────────────────────────
void ENetTransport::Poll(uint32_t timeoutMs)
{
    if (!m_host)
        return;

    ENetEvent event;
    while (enet_host_service(m_host, &event, timeoutMs) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            m_state = ConnectionState::Connected;
            std::cout << "[ENetTransport] Peer connected\n";
            if (m_serverOnConnect)
                m_serverOnConnect(event.peer);
            else if (m_onConnect)
                m_onConnect();
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            if (m_serverOnReceive)
                m_serverOnReceive(event.peer,
                                  event.packet->data,
                                  event.packet->dataLength,
                                  event.channelID);
            else if (m_onReceive)
                m_onReceive(event.packet->data,
                            event.packet->dataLength,
                            event.channelID);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "[ENetTransport] Peer disconnected\n";
            if (m_serverOnDisconnect)
                m_serverOnDisconnect(event.peer);
            else if (m_onDisconnect)
                m_onDisconnect();
            if (event.peer == m_serverPeer)
            {
                m_serverPeer = nullptr;
                m_state = ConnectionState::Disconnected;
            }
            break;

        default:
            break;
        }
        // After first service call use 0 timeout so we drain all pending events.
        timeoutMs = 0;
    }
}

// ── Send (INetworkTransport interface – sends to server peer) ──────
void ENetTransport::Send(const uint8_t *data, size_t len, uint8_t channel)
{
    SendTo(m_serverPeer, data, len, channel);
}

// ── SendTo (server-side: send to a specific peer) ──────────────────
void ENetTransport::SendTo(ENetPeer *peer, const uint8_t *data, size_t len, uint8_t channel)
{
    if (!peer)
        return;
    uint32_t flags = (channel == 0) ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED;
    ENetPacket *packet = enet_packet_create(data, len, flags);
    enet_peer_send(peer, channel, packet);
}

void ENetTransport::SendTo(ENetPeer *peer, const std::vector<uint8_t> &data, uint8_t channel)
{
    SendTo(peer, data.data(), data.size(), channel);
}

void ENetTransport::Broadcast(const uint8_t *data, size_t len, uint8_t channel)
{
    if (!m_host)
        return;
    uint32_t flags = (channel == 0) ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED;
    ENetPacket *packet = enet_packet_create(data, len, flags);
    enet_host_broadcast(m_host, channel, packet);
}

void ENetTransport::Broadcast(const std::vector<uint8_t> &data, uint8_t channel)
{
    Broadcast(data.data(), data.size(), channel);
}

// ── Disconnect ─────────────────────────────────────────────────────
void ENetTransport::Disconnect()
{
    if (m_serverPeer)
    {
        enet_peer_disconnect(m_serverPeer, 0);
        m_state = ConnectionState::Disconnecting;

        // Wait up to 3 seconds for graceful disconnect.
        ENetEvent event;
        while (enet_host_service(m_host, &event, 3000) > 0)
        {
            if (event.type == ENET_EVENT_TYPE_DISCONNECT)
            {
                m_serverPeer = nullptr;
                m_state = ConnectionState::Disconnected;
                break;
            }
            if (event.type == ENET_EVENT_TYPE_RECEIVE)
                enet_packet_destroy(event.packet);
        }
        if (m_serverPeer)
        {
            enet_peer_reset(m_serverPeer);
            m_serverPeer = nullptr;
        }
    }
    Cleanup();
}
