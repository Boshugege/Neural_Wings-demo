#include "NetworkSyncSystem.h"
#include "NetworkSyncComponent.h"
#include "Engine/Network/Client/NetworkClient.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/TransformComponent.h"
#include "Engine/Core/Components/RigidBodyComponent.h"
#include "Engine/Core/GameObject/GameObjectFactory.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include <cmath>

namespace
{
    double NowSeconds()
    {
        using clock = std::chrono::steady_clock;
        return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
    }

    /// Hermite interpolation between two positions using velocities.
    /// t in [0,1], dt is the time span between the two snapshots.
    Vector3f HermiteLerp(const Vector3f &p0, const Vector3f &v0,
                         const Vector3f &p1, const Vector3f &v1,
                         float t, float dt)
    {
        float t2 = t * t;
        float t3 = t2 * t;
        float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
        float h10 = t3 - 2.0f * t2 + t;
        float h01 = -2.0f * t3 + 3.0f * t2;
        float h11 = t3 - t2;
        return p0 * h00 + v0 * (h10 * dt) + p1 * h01 + v1 * (h11 * dt);
    }

    GameObject *FindLiveRemoteObject(GameWorld &world,
                                     ClientID ownerClientID,
                                     NetObjectID objectID)
    {
        for (const auto &objPtr : world.GetGameObjects())
        {
            GameObject *obj = objPtr.get();
            if (obj == nullptr || obj->IsWaitingDestroy())
                continue;
            if (!obj->HasComponent<NetworkSyncComponent>() ||
                !obj->HasComponent<TransformComponent>())
                continue;

            auto &sync = obj->GetComponent<NetworkSyncComponent>();
            if (sync.ownerClientID == ownerClientID &&
                sync.netObjectID == objectID)
            {
                return obj;
            }
        }
        return nullptr;
    }
} // namespace

// ── Cleanup ────────────────────────────────────────────────────────
void NetworkSyncSystem::Cleanup()
{
    m_pendingRemote.clear();
    m_pendingDespawn.clear();
    m_remoteTracks.clear();
    m_remoteRespawnSuppressions.clear();
    m_callbackBound = false;
    m_sendAccumulator = 0.0f;
}

void NetworkSyncSystem::ReleaseLocalObjects(GameWorld &world, NetworkClient &client)
{
    const bool canSendRelease = client.IsConnected();
    size_t releasedCount = 0;
    for (const auto &objPtr : world.GetGameObjects())
    {
        GameObject *obj = objPtr.get();
        if (obj == nullptr || obj->IsWaitingDestroy() || !obj->HasComponent<NetworkSyncComponent>())
            continue;
        auto &sync = obj->GetComponent<NetworkSyncComponent>();
        if (!sync.isLocalPlayer)
            continue;

        if (canSendRelease)
        {
            client.SendObjectRelease(sync.netObjectID);
            ++releasedCount;
            std::cout << "[NetworkSyncSystem] Released local object " << sync.netObjectID << "\n";
        }
    }

    if (!canSendRelease)
    {
        std::cout << "[NetworkSyncSystem] Skip ObjectRelease: client not connected.\n";
    }
    else if (releasedCount == 0)
    {
        std::cout << "[NetworkSyncSystem] No local object to release.\n";
    }

    Cleanup();
}

// ── Init ───────────────────────────────────────────────────────────
void NetworkSyncSystem::Init(NetworkClient &client)
{
    if (m_callbackBound)
        return;

    client.SetOnPositionBroadcast(
        [this](uint32_t serverTick, const std::vector<NetBroadcastEntry> &entries)
        {
            const double receiveTimeSec = NowSeconds();
            for (const auto &e : entries)
            {
                m_pendingRemote.push_back({serverTick, e.clientID, e.objectID, e.transform, receiveTimeSec});
            }
            if (m_pendingRemote.size() > 1024)
                m_pendingRemote.erase(m_pendingRemote.begin(),
                                      m_pendingRemote.begin() + (m_pendingRemote.size() - 1024));
        });
    client.SetOnObjectDespawn(
        [this](ClientID ownerClientID, NetObjectID objectID)
        {
            m_pendingDespawn.push_back({ownerClientID, objectID});
        });

    m_callbackBound = true;
}

// ── Update ─────────────────────────────────────────────────────────
void NetworkSyncSystem::Update(GameWorld &world, NetworkClient &client, float deltaTime)
{
    const double nowSec = NowSeconds();
    PruneRemoteRespawnSuppressions(nowSec);

    if (!client.IsConnected())
    {
        // Connection lost: purge all remote entities to avoid stale ghosts.
        RemoveRemoteObjects(world, INVALID_CLIENT_ID, true);
        m_pendingRemote.clear();
        m_pendingDespawn.clear();
        m_remoteTracks.clear();
        m_sendAccumulator = 0.0f;
        return;
    }
    const ClientID localID = client.GetLocalClientID();

    // 1. Upload local flight state (rate-limited) ────────────────────
    m_sendAccumulator += deltaTime;
    const float sendInterval = (sendHz > 0.0f) ? (1.0f / sendHz) : 0.033f;
    bool shouldSend = (m_sendAccumulator >= sendInterval);
    if (shouldSend)
        m_sendAccumulator -= sendInterval;

    auto syncedEntities = world.GetEntitiesWith<NetworkSyncComponent, TransformComponent>();
    static bool s_loggedNoLocalSync = false;
    static bool s_loggedLocalSync = false;
    bool hasLocalSync = false;
    for (auto *obj : syncedEntities)
    {
        if (obj == nullptr || !obj->HasComponent<NetworkSyncComponent>() || !obj->HasComponent<TransformComponent>())
            continue;
        auto &sync = obj->GetComponent<NetworkSyncComponent>();
        if (!sync.isLocalPlayer)
            continue;
        hasLocalSync = true;

        if (!shouldSend)
            break; // still need to set hasLocalSync but skip sending

        // Fill owner id lazily.
        if (sync.ownerClientID == INVALID_CLIENT_ID)
            sync.ownerClientID = client.GetLocalClientID();

        auto &tf = obj->GetComponent<TransformComponent>();
        Vector3f pos = tf.GetWorldPosition();
        Quat4f rot = tf.GetWorldRotation();

        NetTransformState ts{};
        ts.posX = pos.x();
        ts.posY = pos.y();
        ts.posZ = pos.z();
        ts.rotW = rot[0];
        ts.rotX = rot[1];
        ts.rotY = rot[2];
        ts.rotZ = rot[3];

        // Read velocity from RigidBodyComponent if available
        if (obj->HasComponent<RigidbodyComponent>())
        {
            auto &rb = obj->GetComponent<RigidbodyComponent>();
            ts.linVelX = rb.velocity.x();
            ts.linVelY = rb.velocity.y();
            ts.linVelZ = rb.velocity.z();
            ts.angVelX = rb.angularVelocity.x();
            ts.angVelY = rb.angularVelocity.y();
            ts.angVelZ = rb.angularVelocity.z();
        }

        client.SendPositionUpdate(sync.netObjectID, ts);
    }
    if (!hasLocalSync && !s_loggedNoLocalSync)
    {
        std::cout << "[NetworkSyncSystem] No local sync object found." << std::endl;
        s_loggedNoLocalSync = true;
    }
    if (hasLocalSync && !s_loggedLocalSync)
    {
        std::cout << "[NetworkSyncSystem] Local flight state upload active (sendHz="
                  << sendHz << ")." << std::endl;
        s_loggedLocalSync = true;
    }

    // Defensive cleanup: never keep non-local entities owned by our own client id.
    RemoveRemoteObjects(world, localID, false);

    // 2. Apply remote flight states ──────────────────────────────────
    ApplyRemoteBroadcast(world, client);
    ApplyRemoteDespawn(world, client);
    ApplyRemoteInterpolation(world, client, deltaTime);
}

// ── Apply remote ───────────────────────────────────────────────────
void NetworkSyncSystem::ApplyRemoteBroadcast(GameWorld &world,
                                             NetworkClient &client)
{
    if (m_pendingRemote.empty())
        return;

    ClientID localID = client.GetLocalClientID();
    const double nowSec = NowSeconds();

    for (auto &remote : m_pendingRemote)
    {
        if (remote.clientID == INVALID_CLIENT_ID || remote.objectID == INVALID_NET_OBJECT_ID)
            continue;

        // Skip our own echo.
        if (remote.clientID == localID)
            continue;

        const uint64_t key = MakeRemoteKey(remote.clientID, remote.objectID);
        if (IsRemoteRespawnSuppressed(key, nowSec))
            continue;

        // Ensure the corresponding remote object exists in world.
        GameObject *target = FindOrSpawnRemoteObject(world, remote.clientID, remote.objectID);

        if (target == nullptr)
            continue;

        auto &track = m_remoteTracks[key];

        RemoteSnapshot snap{};
        snap.serverTick = remote.serverTick;
        snap.receiveTimeSec = remote.receiveTimeSec;
        snap.position = Vector3f(remote.transform.posX,
                                 remote.transform.posY,
                                 remote.transform.posZ);
        snap.rotation = Quat4f(remote.transform.rotW,
                               remote.transform.rotX,
                               remote.transform.rotY,
                               remote.transform.rotZ);
        snap.linearVelocity = Vector3f(remote.transform.linVelX,
                                       remote.transform.linVelY,
                                       remote.transform.linVelZ);
        snap.angularVelocity = Vector3f(remote.transform.angVelX,
                                        remote.transform.angVelY,
                                        remote.transform.angVelZ);

        if (!track.snapshots.empty())
        {
            const auto &last = track.snapshots.back();
            if (snap.serverTick < last.serverTick)
                continue;
            if (snap.serverTick == last.serverTick)
            {
                if (snap.receiveTimeSec < last.receiveTimeSec)
                    snap.receiveTimeSec = last.receiveTimeSec;
                track.snapshots.back() = snap;
                continue;
            }
            if (snap.receiveTimeSec <= last.receiveTimeSec)
                snap.receiveTimeSec = last.receiveTimeSec + 1e-6;
        }

        track.snapshots.push_back(snap);
        while (track.snapshots.size() > m_maxSnapshotsPerTrack)
            track.snapshots.pop_front();
    }

    m_pendingRemote.clear();
}

void NetworkSyncSystem::ApplyRemoteInterpolation(GameWorld &world, NetworkClient &client, float deltaTime)
{
    if (m_remoteTracks.empty())
        return;

    const double nowSec = NowSeconds();
    const double renderTimeSec = nowSec - interpolationBackTimeSec;
    ClientID localID = client.GetLocalClientID();

    auto syncedEntities = world.GetEntitiesWith<NetworkSyncComponent, TransformComponent>();
    std::unordered_map<uint64_t, GameObject *> remoteObjects;
    remoteObjects.reserve(syncedEntities.size());

    for (auto *obj : syncedEntities)
    {
        if (obj == nullptr || !obj->HasComponent<NetworkSyncComponent>() || !obj->HasComponent<TransformComponent>())
            continue;
        auto &sync = obj->GetComponent<NetworkSyncComponent>();
        if (sync.isLocalPlayer || sync.ownerClientID == localID)
            continue;
        remoteObjects[MakeRemoteKey(sync.ownerClientID, sync.netObjectID)] = obj;
    }

    for (auto &[key, track] : m_remoteTracks)
    {
        auto itObj = remoteObjects.find(key);
        if (itObj == remoteObjects.end())
            continue;
        if (track.snapshots.empty())
            continue;

        Vector3f targetPos = track.snapshots.back().position;
        Quat4f targetRot = track.snapshots.back().rotation;

        const auto &snaps = track.snapshots;
        bool usedInterpolation = false;

        // ── Snapshot buffer interpolation (Hermite) ─────────────────
        for (size_t i = 1; i < snaps.size(); ++i)
        {
            const auto &a = snaps[i - 1];
            const auto &b = snaps[i];
            if (b.receiveTimeSec < renderTimeSec)
                continue;

            const double dt = b.receiveTimeSec - a.receiveTimeSec;
            float t = 1.0f;
            if (dt > 1e-6)
                t = static_cast<float>((renderTimeSec - a.receiveTimeSec) / dt);
            t = std::clamp(t, 0.0f, 1.0f);

            // Hermite interpolation using velocity at both endpoints
            targetPos = HermiteLerp(a.position, a.linearVelocity,
                                    b.position, b.linearVelocity,
                                    t, static_cast<float>(dt));
            targetRot = Quat4f::slerp(a.rotation, b.rotation, t);
            usedInterpolation = true;
            break;
        }

        // ── Velocity-based extrapolation ────────────────────────────
        if (!usedInterpolation && !snaps.empty() && renderTimeSec > snaps.back().receiveTimeSec)
        {
            const auto &last = snaps.back();
            double extraSec = renderTimeSec - last.receiveTimeSec;
            if (extraSec > maxExtrapolationSec)
                extraSec = maxExtrapolationSec;

            targetPos = last.position + last.linearVelocity * static_cast<float>(extraSec);

            // Extrapolate rotation using angular velocity
            float angSpeed = last.angularVelocity.Length();
            if (angSpeed > 1e-6f)
            {
                Vector3f axis = last.angularVelocity / angSpeed;
                float angle = angSpeed * static_cast<float>(extraSec);
                Quat4f deltaRot;
                deltaRot.setAxisAngle(angle, axis);
                targetRot = (deltaRot * last.rotation).normalized();
            }
            else
            {
                targetRot = last.rotation;
            }
        }

        // ── Error convergence ───────────────────────────────────────
        auto &tf = itObj->second->GetComponent<TransformComponent>();

        if (!track.hasDisplayState)
        {
            // First time — snap immediately
            track.displayPosition = targetPos;
            track.displayRotation = targetRot;
            track.hasDisplayState = true;
        }
        else
        {
            Vector3f error = targetPos - track.displayPosition;
            float errorLen = error.Length();

            if (errorLen > correctionSnapThreshold)
            {
                // Too far — snap to target
                track.displayPosition = targetPos;
                track.displayRotation = targetRot;
            }
            else
            {
                // Smooth convergence
                float dt_frame = std::clamp(deltaTime, 0.001f, 0.1f);
                float blend = 1.0f - std::exp(-correctionBlendRate * dt_frame);
                blend = std::clamp(blend, 0.0f, 1.0f);
                track.displayPosition = track.displayPosition + (targetPos - track.displayPosition) * blend;
                track.displayRotation = Quat4f::slerp(track.displayRotation, targetRot, blend);
            }
        }

        tf.SetWorldMatrix(Matrix4f::CreateTransform(track.displayPosition,
                                                    track.displayRotation,
                                                    Vector3f::ONE));

        // Prune old snapshots
        while (track.snapshots.size() > 2 &&
               track.snapshots[1].receiveTimeSec < (renderTimeSec - 0.25))
        {
            track.snapshots.pop_front();
        }
    }
}

GameObject *NetworkSyncSystem::FindOrSpawnRemoteObject(GameWorld &world, ClientID ownerClientID, NetObjectID objectID)
{
    if (GameObject *existing = FindLiveRemoteObject(world, ownerClientID, objectID))
        return existing;

    const std::string remoteName = "remote_plane_" + std::to_string(ownerClientID) + "_" + std::to_string(objectID);
    GameObject &newObj = GameObjectFactory::CreateFromPrefab(remoteName, "RemotePlayer", m_remotePlayerPrefabPath, world);
    newObj.SetOwnerWorld(&world);

    if (!newObj.HasComponent<TransformComponent>())
        newObj.AddComponent<TransformComponent>();
    auto &tf = newObj.GetComponent<TransformComponent>();
    tf.SetOwner(&newObj);

    auto &sync = newObj.AddComponent<NetworkSyncComponent>(objectID, false);
    sync.ownerClientID = ownerClientID;
    sync.netObjectID = objectID;
    sync.isLocalPlayer = false;

    newObj.SetActive(true);

    std::cout << "[NetworkSyncSystem] Spawned remote plane client="
              << ownerClientID << " obj=" << objectID << "\n";
    return &newObj;
}

void NetworkSyncSystem::ApplyRemoteDespawn(GameWorld &world, NetworkClient &client)
{
    if (m_pendingDespawn.empty())
        return;

    ClientID localID = client.GetLocalClientID();
    auto syncedEntities = world.GetEntitiesWith<NetworkSyncComponent, TransformComponent>();

    for (const auto &despawn : m_pendingDespawn)
    {
        if (despawn.ownerClientID == INVALID_CLIENT_ID || despawn.objectID == INVALID_NET_OBJECT_ID)
            continue;

        if (despawn.ownerClientID == localID)
            continue;
        const uint64_t key = MakeRemoteKey(despawn.ownerClientID, despawn.objectID);
        MarkRemoteDespawned(despawn.ownerClientID, despawn.objectID, NowSeconds());
        m_remoteTracks.erase(key);

        for (auto *obj : syncedEntities)
        {
            if (obj == nullptr || !obj->HasComponent<NetworkSyncComponent>() || !obj->HasComponent<TransformComponent>())
                continue;
            auto &sync = obj->GetComponent<NetworkSyncComponent>();
            if (sync.isLocalPlayer)
                continue;
            if (sync.ownerClientID != despawn.ownerClientID || sync.netObjectID != despawn.objectID)
                continue;

            obj->SetActive(false);
            obj->SetIsWaitingDestroy(true);
            std::cout << "[NetworkSyncSystem] Despawn remote plane client="
                      << despawn.ownerClientID << " obj=" << despawn.objectID << "\n";
            break;
        }
    }

    m_pendingDespawn.clear();
}

void NetworkSyncSystem::RemoveRemoteObjects(GameWorld &world, ClientID localClientID, bool removeAllRemotes)
{
    const double nowSec = NowSeconds();
    auto syncedEntities = world.GetEntitiesWith<NetworkSyncComponent, TransformComponent>();
    for (auto *obj : syncedEntities)
    {
        if (obj == nullptr || !obj->HasComponent<NetworkSyncComponent>() || !obj->HasComponent<TransformComponent>())
            continue;

        auto &sync = obj->GetComponent<NetworkSyncComponent>();
        if (sync.isLocalPlayer)
            continue;

        const bool shouldRemove = removeAllRemotes ||
                                  (localClientID != INVALID_CLIENT_ID && sync.ownerClientID == localClientID);
        if (!shouldRemove)
            continue;

        MarkRemoteDespawned(sync.ownerClientID, sync.netObjectID, nowSec);
        m_remoteTracks.erase(MakeRemoteKey(sync.ownerClientID, sync.netObjectID));
        obj->SetActive(false);
        obj->SetIsWaitingDestroy(true);
    }
}

bool NetworkSyncSystem::IsRemoteRespawnSuppressed(uint64_t key, double nowSec) const
{
    auto it = m_remoteRespawnSuppressions.find(key);
    if (it == m_remoteRespawnSuppressions.end())
        return false;
    return it->second.expireTimeSec > nowSec;
}

void NetworkSyncSystem::MarkRemoteDespawned(ClientID ownerClientID, NetObjectID objectID, double nowSec)
{
    const uint64_t key = MakeRemoteKey(ownerClientID, objectID);
    m_remoteRespawnSuppressions[key] = RemoteRespawnSuppression{nowSec + m_remoteRespawnSuppressionSec};
}

void NetworkSyncSystem::PruneRemoteRespawnSuppressions(double nowSec)
{
    for (auto it = m_remoteRespawnSuppressions.begin(); it != m_remoteRespawnSuppressions.end();)
    {
        if (it->second.expireTimeSec <= nowSec)
        {
            it = m_remoteRespawnSuppressions.erase(it);
            continue;
        }
        ++it;
    }
}

uint64_t NetworkSyncSystem::MakeRemoteKey(ClientID ownerClientID, NetObjectID objectID)
{
    return (static_cast<uint64_t>(ownerClientID) << 32) |
           static_cast<uint64_t>(objectID);
}
