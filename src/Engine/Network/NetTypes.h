#pragma once
#include <cstdint>

/// Unique identifier assigned by the server to each connected client.
using ClientID = uint32_t;

/// Unique identifier for a networked game object.
using NetObjectID = uint32_t;

/// Reserved value: "not assigned yet".
constexpr ClientID INVALID_CLIENT_ID = 0;
constexpr NetObjectID INVALID_NET_OBJECT_ID = 0;

/// Default network settings.
constexpr uint16_t DEFAULT_SERVER_PORT = 7777;
constexpr const char *DEFAULT_SERVER_HOST = "127.0.0.1";

enum class ConnectionState : uint8_t
{
    Disconnected,
    Connecting,
    Connected,
    Disconnecting
};
