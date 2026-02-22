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

/// Reads / writes flight state data (transform + velocity) through
/// NetworkClient, acting as the bridge between the ECS world and the network.
/// Upload source is the local plane entity (not camera anchor).
class NetworkSyncSystem
{
public:
    NetworkSyncSystem() = default;

    /// Call once after NetworkClient::Connect to wire up the broadcast callback.
    void Init(NetworkClient &client);

    /// Reset all internal state (tracks, pending entries, callback flag).
    /// Call this when leaving gameplay.
    void Cleanup();

    /// Send ObjectRelease for all local objects, then clean up sync state.
    /// Connection stays alive for other systems (chat, lobby, etc.).
    void ReleaseLocalObjects(GameWorld &world, NetworkClient &client);

    /// Called every frame **after** NetworkClient::Poll().
    /// • Uploads local plane flight state → server (rate-limited to m_sendHz).
    /// • Applies received remote flight states → GameObjects with interpolation.
    void Update(GameWorld &world, NetworkClient &client, float deltaTime);

    // ── Tunable parameters (public for editor / config) ────────────
    float sendHz = 30.0f;                   // upload rate (Hz)
    double interpolationBackTimeSec = 0.10; // snapshot buffer delay (sec)
    double maxExtrapolationSec = 0.15;      // max extrapolation beyond last snapshot
    float correctionSnapThreshold = 5.0f;   // beyond this distance → snap
    float correctionBlendRate = 10.0f;      // convergence speed (units/sec lerp factor)

private:
    // ── Receive / apply pipeline ───────────────────────────────────
    /// Internal: apply a remote broadcast to the world.
    void ApplyRemoteBroadcast(GameWorld &world, NetworkClient &client);
    void ApplyRemoteInterpolation(GameWorld &world, NetworkClient &client, float deltaTime);
    void ApplyRemoteDespawn(GameWorld &world, NetworkClient &client);

    // ── Remote entity lifecycle guards ─────────────────────────────
    void RemoveRemoteObjects(GameWorld &world, ClientID localClientID, bool removeAllRemotes);
    bool IsRemoteRespawnSuppressed(uint64_t key, double nowSec) const;
    void MarkRemoteDespawned(ClientID ownerClientID, NetObjectID objectID, double nowSec);
    void PruneRemoteRespawnSuppressions(double nowSec);

    GameObject *FindOrSpawnRemoteObject(GameWorld &world, ClientID ownerClientID, NetObjectID objectID);
    static uint64_t MakeRemoteKey(ClientID ownerClientID, NetObjectID objectID);

    // Buffer filled by the broadcast callback, consumed in Update().
    struct RemoteEntry
    {
        uint32_t serverTick = 0;
        ClientID clientID;
        NetObjectID objectID;
        NetTransformState transform;
        double receiveTimeSec = 0.0; // receive timestamp captured in network callback
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
        Vector3f linearVelocity = Vector3f::ZERO;
        Vector3f angularVelocity = Vector3f::ZERO;
    };
    struct RemoteTrack
    {
        std::deque<RemoteSnapshot> snapshots;
        // Error-correction state
        Vector3f displayPosition = Vector3f::ZERO;
        Quat4f displayRotation = Quat4f::IDENTITY;
        bool hasDisplayState = false;
    };
    struct RemoteRespawnSuppression
    {
        double expireTimeSec = 0.0;
    };

    std::vector<RemoteEntry> m_pendingRemote;
    std::vector<DespawnEntry> m_pendingDespawn;
    std::unordered_map<uint64_t, RemoteTrack> m_remoteTracks;
    std::unordered_map<uint64_t, RemoteRespawnSuppression> m_remoteRespawnSuppressions;
    bool m_callbackBound = false;
    std::string m_remotePlayerPrefabPath = "assets/prefabs/remote_player.json";

    size_t m_maxSnapshotsPerTrack = 32;

    // Send rate limiter
    float m_sendAccumulator = 0.0f;

    // Ignore stale unreliable broadcasts briefly after a despawn.
    double m_remoteRespawnSuppressionSec = 1.2;
};
