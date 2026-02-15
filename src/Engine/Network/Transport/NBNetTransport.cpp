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

// ── Constants ──────────────────────────────────────────────────────
static constexpr const char *NW_PROTOCOL_NAME = "neural_wings";

static uint8_t MapChannel(uint8_t ourChannel)
{
    // our convention: 0 = reliable, 1 = unreliable
    return (ourChannel == 0) ? NBN_CHANNEL_RESERVED_RELIABLE : NBN_CHANNEL_RESERVED_UNRELIABLE;
}

// ── Lifecycle ──────────────────────────────────────────────────────

NBNetTransport::~NBNetTransport()
{
    Disconnect();
}

bool NBNetTransport::Connect(const std::string &host, uint16_t port)
{
    if (m_started)
        Disconnect();

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
                               host.c_str(),
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
    std::cout << "[NBNetTransport] Connecting to " << host << ":" << port << " ...\n";
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
void NBNetTransport::Send(const uint8_t *data, size_t len, uint8_t channel)
{
    if (!m_started || m_state != ConnectionState::Connected || !data || len == 0)
        return;

    if (NBN_GameClient_SendByteArray(
            const_cast<uint8_t *>(data),
            static_cast<unsigned int>(len),
            MapChannel(channel)) < 0)
    {
        std::cerr << "[NBNetTransport] SendByteArray failed\n";
    }
}
