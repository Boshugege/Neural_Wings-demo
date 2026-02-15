#pragma once
#include "Engine/Network/NetTypes.h"
#include "Engine/Network/Protocol/Messages.h"
#include "Engine/Math/Math.h"
#include <vector>
#include <string>
#include <deque>
#include <unordered_map>
#include <cstdint>

class GameWorld;
class NetworkClient;
class GameObject;

/// Reads / writes TransformComponent data through NetworkClient,
/// acting as the bridge between the ECS world and the network.
class NetworkSyncSystem
{
public:
    NetworkSyncSystem() = default;

    /// Call once after NetworkClient::Connect to wire up the broadcast callback.
    void Init(NetworkClient &client);

    /// Called every frame **after** NetworkClient::Poll().
    /// • Uploads local player transforms → server.
    /// • Applies received remote transforms → GameObjects.
    void Update(GameWorld &world, NetworkClient &client);

private:
    /// Internal: apply a remote broadcast to the world.
    void ApplyRemoteBroadcast(GameWorld &world, NetworkClient &client);
    void ApplyRemoteInterpolation(GameWorld &world, NetworkClient &client);
    void ApplyRemoteDespawn(GameWorld &world, NetworkClient &client);
    GameObject *FindOrSpawnRemoteObject(GameWorld &world, ClientID ownerClientID, NetObjectID objectID);
    static uint64_t MakeRemoteKey(ClientID ownerClientID, NetObjectID objectID);

    // Buffer filled by the broadcast callback, consumed in Update().
    struct RemoteEntry
    {
        uint32_t serverTick = 0;
        ClientID clientID;
        NetObjectID objectID;
        NetTransformState transform;
    };
    struct DespawnEntry
    {
        ClientID ownerClientID;
        NetObjectID objectID;
    };
    struct RemoteSnapshot
    {
        uint32_t serverTick = 0;
        double receiveTimeSec = 0.0;
        Vector3f position = Vector3f::ZERO;
        Quat4f rotation = Quat4f::IDENTITY;
    };
    struct RemoteTrack
    {
        std::deque<RemoteSnapshot> snapshots;
    };

    std::vector<RemoteEntry> m_pendingRemote;
    std::vector<DespawnEntry> m_pendingDespawn;
    std::unordered_map<uint64_t, RemoteTrack> m_remoteTracks;
    bool m_callbackBound = false;
    std::string m_remotePlayerPrefabPath = "assets/prefabs/remote_player.json";

    double m_interpolationBackTimeSec = 0.07; // 70ms
    double m_maxExtrapolationSec = 0.10;      // 100ms
    size_t m_maxSnapshotsPerTrack = 32;
};
