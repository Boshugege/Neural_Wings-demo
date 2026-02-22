#pragma once
#include "Engine/Network/NetTypes.h"
#include "Engine/Network/Protocol/PacketSerializer.h"
#include <vector>
#include <functional>
#include <string>
#include <memory>
#include <unordered_map>

class INetworkTransport;

/// High-level client-side network manager.
/// Depends ONLY on INetworkTransport — no platform #if anywhere.
class NetworkClient
{
public:
    struct PlayerMeta
    {
        ClientID clientID = INVALID_CLIENT_ID;
        std::string nickname;
    };

    using OnPositionBroadcastFn =
        std::function<void(uint32_t serverTick,
                           const std::vector<NetBroadcastEntry> &entries)>;
    using OnObjectDespawnFn =
        std::function<void(ClientID ownerClientID, NetObjectID objectID)>;
    using OnChatMessageFn =
        std::function<void(ChatMessageType chatType, ClientID senderID,
                           const std::string &senderName, const std::string &text)>;
    using OnNicknameUpdateResultFn =
        std::function<void(NicknameUpdateStatus status,
                           const std::string &authoritativeNickname)>;
    using OnPlayerMetaChangedFn = std::function<void()>;

    NetworkClient();
    ~NetworkClient();

    NetworkClient(const NetworkClient &) = delete;
    NetworkClient &operator=(const NetworkClient &) = delete;

    // ── Lifecycle ──────────────────────────────────────────────────
    bool Connect(const std::string &host = DEFAULT_SERVER_HOST,
                 uint16_t port = DEFAULT_SERVER_PORT);
    void Disconnect();
    void Poll();

    // ── State ──────────────────────────────────────────────────────
    bool IsConnected() const;
    ConnectionState GetConnectionState() const;
    ClientID GetLocalClientID() const { return m_localClientID; }

    // ── Identity ───────────────────────────────────────────────────
    void SetUUID(const NetUUID &uuid) { m_uuid = uuid; }
    const NetUUID &GetUUID() const { return m_uuid; }

    // ── Sending ────────────────────────────────────────────────────
    void SendPositionUpdate(NetObjectID objectID,
                            const NetTransformState &transform);
    void SendObjectRelease(NetObjectID objectID);
    void SendHeartbeat();
    bool SendChatMessage(ChatMessageType chatType, const std::string &text,
                         ClientID targetID = INVALID_CLIENT_ID);
    void SendNicknameUpdate(const std::string &nickname);

    /// Flush outgoing packet queue immediately (call after a batch of Sends).
    void FlushSend();

    // ── Nickname state ─────────────────────────────────────────────
    void SetDesiredNickname(const std::string &nickname) { m_desiredNickname = nickname; }
    const std::string &GetDesiredNickname() const { return m_desiredNickname; }
    const std::string &GetAuthoritativeNickname() const { return m_authoritativeNickname; }
    const std::unordered_map<ClientID, PlayerMeta> &GetPlayerMetaMap() const { return m_playerMeta; }
    std::string GetPlayerNickname(ClientID clientID) const;

    // ── Callbacks ──────────────────────────────────────────────────
    void SetOnPositionBroadcast(OnPositionBroadcastFn fn)
    {
        m_onPositionBroadcast = std::move(fn);
    }
    void SetOnObjectDespawn(OnObjectDespawnFn fn)
    {
        m_onObjectDespawn = std::move(fn);
    }
    void SetOnChatMessage(OnChatMessageFn fn)
    {
        m_onChatMessage = std::move(fn);
    }
    void SetOnNicknameUpdateResult(OnNicknameUpdateResultFn fn)
    {
        m_onNicknameUpdateResult = std::move(fn);
    }
    void SetOnPlayerMetaChanged(OnPlayerMetaChangedFn fn)
    {
        m_onPlayerMetaChanged = std::move(fn);
    }

private:
    void OnRawReceive(const uint8_t *data, size_t len, uint8_t channelID);

    std::unique_ptr<INetworkTransport> m_transport;
    ClientID m_localClientID = INVALID_CLIENT_ID;
    NetUUID m_uuid{};
    OnPositionBroadcastFn m_onPositionBroadcast;
    OnObjectDespawnFn m_onObjectDespawn;
    OnChatMessageFn m_onChatMessage;
    OnNicknameUpdateResultFn m_onNicknameUpdateResult;
    OnPlayerMetaChangedFn m_onPlayerMetaChanged;
    std::string m_desiredNickname;
    std::string m_authoritativeNickname;
    std::unordered_map<ClientID, PlayerMeta> m_playerMeta;
};
