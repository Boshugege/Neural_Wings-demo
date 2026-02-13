#pragma once
#include "Engine/Network/NetTypes.h"
#include "Engine/Network/Protocol/Messages.h"
#include <vector>

class GameWorld;
class NetworkClient;

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

    // Buffer filled by the broadcast callback, consumed in Update().
    struct RemoteEntry
    {
        ClientID clientID;
        NetObjectID objectID;
        NetTransformState transform;
    };
    std::vector<RemoteEntry> m_pendingRemote;
    bool m_callbackBound = false;
};
