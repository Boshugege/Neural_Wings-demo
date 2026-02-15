#pragma once
#include "Engine/Network/NetTypes.h"
#include "Engine/Network/Protocol/Messages.h"
#include <vector>
#include <string>

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
    void ApplyRemoteDespawn(GameWorld &world, NetworkClient &client);
    GameObject *FindOrSpawnRemoteObject(GameWorld &world, ClientID ownerClientID, NetObjectID objectID);

    // Buffer filled by the broadcast callback, consumed in Update().
    struct RemoteEntry
    {
        ClientID clientID;
        NetObjectID objectID;
        NetTransformState transform;
    };
    struct DespawnEntry
    {
        ClientID ownerClientID;
        NetObjectID objectID;
    };
    std::vector<RemoteEntry> m_pendingRemote;
    std::vector<DespawnEntry> m_pendingDespawn;
    bool m_callbackBound = false;
    std::string m_remotePlayerPrefabPath = "assets/prefabs/remote_player.json";
};
