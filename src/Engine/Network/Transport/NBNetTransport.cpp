// ────────────────────────────────────────────────────────────────────
// NBNetTransport  –  nbnet client C++ wrapper
//
// The actual nbnet implementation (NBNET_IMPL) is compiled as C in
// nbnet_client_impl.c.  This file only uses declaration-level access.
// ────────────────────────────────────────────────────────────────────

extern "C"
{
#include <nbnet.h>

#if defined(PLATFORM_WEB)
#include <net_drivers/webrtc.h>
#else
#include <net_drivers/udp.h>
#endif
}

#include "NBNetTransport.h"
#include <iostream>
#include <string>

#if !defined(PLATFORM_WEB)
#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#endif
#endif

// ── Constants ──────────────────────────────────────────────────────
static constexpr const char *NW_PROTOCOL_NAME = "neural_wings";

static uint8_t MapChannel(uint8_t ourChannel)
{
    // our convention: 0 = reliable, 1 = unreliable
    return (ourChannel == 0) ? NBN_CHANNEL_RESERVED_RELIABLE : NBN_CHANNEL_RESERVED_UNRELIABLE;
}

#if !defined(PLATFORM_WEB)
static std::string ResolveHostToIPv4(const std::string &host)
{
    addrinfo hints{};
    hints.ai_family = AF_INET;      // nbnet UDP driver is IPv4-only
    hints.ai_socktype = SOCK_DGRAM; // align with UDP transport

    addrinfo *result = nullptr;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &result) != 0 || result == nullptr)
    {
        return host;
    }

    char ipBuffer[INET_ADDRSTRLEN] = {};
    const sockaddr_in *addr = reinterpret_cast<const sockaddr_in *>(result->ai_addr);
    const char *ok = inet_ntop(AF_INET, &addr->sin_addr, ipBuffer, sizeof(ipBuffer));

    std::string resolved = host;
    if (ok != nullptr)
        resolved = ipBuffer;

    freeaddrinfo(result);
    return resolved;
}
#endif

// ── Lifecycle ──────────────────────────────────────────────────────

NBNetTransport::~NBNetTransport()
{
    Disconnect();
}

bool NBNetTransport::Connect(const std::string &host, uint16_t port)
{
    if (m_started)
        Disconnect();

    std::string targetHost = host;
#if !defined(PLATFORM_WEB)
    targetHost = ResolveHostToIPv4(host);
    if (targetHost != host)
    {
        std::cout << "[NBNetTransport] Resolved host " << host << " -> " << targetHost << "\n";
    }
#endif

    // Register the platform-appropriate driver BEFORE starting.
    // NBN_Driver_Register asserts the driver isn't already registered,
    // and NBN_GameClient_Stop does NOT unregister drivers, so we must
    // only register once per process lifetime.
    static bool s_driverRegistered = false;
    if (!s_driverRegistered)
    {
#if defined(PLATFORM_WEB)
        NBN_WebRTC_Register((NBN_WebRTC_Config){false, NULL, NULL});
#else
        NBN_UDP_Register();
#endif
        s_driverRegistered = true;
    }

    // Start the client (init + driver activation + built-in message registration)
    if (NBN_GameClient_StartEx(NW_PROTOCOL_NAME,
                               targetHost.c_str(),
                               port,
                               false,   // no encryption
                               nullptr, // no connection data
                               0) < 0)
    {
        std::cerr << "[NBNetTransport] NBN_GameClient_StartEx failed\n";
        return false;
    }

    m_started = true;
    m_state = ConnectionState::Connecting;
    std::cout << "[NBNetTransport] Connecting to " << targetHost << ":" << port << " ...\n";
    return true;
}

void NBNetTransport::Disconnect()
{
    if (!m_started)
        return;

    NBN_GameClient_Stop();

    m_started = false;
    m_state = ConnectionState::Disconnected;
}

// ── Poll ───────────────────────────────────────────────────────────
void NBNetTransport::Poll(uint32_t /*timeoutMs*/)
{
    if (!m_started)
        return;

    int ev;
    while ((ev = NBN_GameClient_Poll()) != NBN_NO_EVENT)
    {
        if (ev < 0)
        {
            std::cerr << "[NBNetTransport] Poll error\n";
            m_state = ConnectionState::Disconnected;
            if (m_onDisconnect)
                m_onDisconnect();
            return;
        }

        switch (ev)
        {
        case NBN_CONNECTED:
            m_state = ConnectionState::Connected;
            std::cout << "[NBNetTransport] Connected\n";
            if (m_onConnect)
                m_onConnect();
            break;

        case NBN_DISCONNECTED:
            m_state = ConnectionState::Disconnected;
            std::cout << "[NBNetTransport] Disconnected\n";
            if (m_onDisconnect)
                m_onDisconnect();
            break;

        case NBN_MESSAGE_RECEIVED:
        {
            NBN_MessageInfo info = NBN_GameClient_GetMessageInfo();

            if (info.type == NBN_BYTE_ARRAY_MESSAGE_TYPE)
            {
                NBN_ByteArrayMessage *msg =
                    static_cast<NBN_ByteArrayMessage *>(info.data);

                if (m_onReceive && msg)
                {
                    // Translate nbnet channel back to our convention
                    uint8_t ourChannel =
                        (info.channel_id == NBN_CHANNEL_RESERVED_RELIABLE) ? 0 : 1;
                    m_onReceive(msg->bytes, msg->length, ourChannel);
                }
            }
            break;
        }
        }
    }

    // Flush outgoing packets
    if (NBN_GameClient_SendPackets() < 0)
    {
        std::cerr << "[NBNetTransport] SendPackets failed\n";
    }
}

// ── Send ───────────────────────────────────────────────────────────
bool NBNetTransport::Send(const uint8_t *data, size_t len, uint8_t channel)
{
    if (!m_started || m_state != ConnectionState::Connected || !data || len == 0)
        return false;

    // Guard against nbnet-internal stale/closed state to prevent
    // assert(!connection->is_stale) crash in NBN_Connection_EnqueueOutgoingMessage.
    if (!nbn_game_client.server_connection ||
        nbn_game_client.server_connection->is_stale ||
        nbn_game_client.server_connection->is_closed)
    {
        // Sync our state so callers see Disconnected immediately.
        m_state = ConnectionState::Disconnected;
        return false;
    }

    if (NBN_GameClient_SendByteArray(
            const_cast<uint8_t *>(data),
            static_cast<unsigned int>(len),
            MapChannel(channel)) < 0)
    {
        std::cerr << "[NBNetTransport] SendByteArray failed\n";
        return false;
    }
    return true;
}

void NBNetTransport::FlushSend()
{
    if (!m_started)
        return;
    if (NBN_GameClient_SendPackets() < 0)
    {
        std::cerr << "[NBNetTransport] FlushSend SendPackets failed\n";
    }
}
