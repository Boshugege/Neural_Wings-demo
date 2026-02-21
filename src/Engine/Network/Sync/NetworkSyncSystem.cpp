#include "NetworkSyncSystem.h"
#include "NetworkSyncComponent.h"
#include "Engine/Network/Client/NetworkClient.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/TransformComponent.h"
#include "Engine/Core/GameObject/GameObjectFactory.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>

namespace
{
    double NowSeconds()
    {
        using clock = std::chrono::steady_clock;
        return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
    }
} // namespace

// ── Init ───────────────────────────────────────────────────────────
void NetworkSyncSystem::Init(NetworkClient &client)
{
    if (m_callbackBound)
        return;

    client.SetOnPositionBroadcast(
        [this](uint32_t serverTick, const std::vector<NetBroadcastEntry> &entries)
        {
            for (const auto &e : entries)
            {
                m_pendingRemote.push_back({serverTick, e.clientID, e.objectID, e.transform});
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
void NetworkSyncSystem::Update(GameWorld &world, NetworkClient &client)
{
    if (!client.IsConnected())
    {
        m_pendingRemote.clear();
        m_pendingDespawn.clear();
        m_remoteTracks.clear();
        return;
    }

    // 1. Upload local transforms ─────────────────────────────────────
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

        client.SendPositionUpdate(sync.netObjectID, ts);
    }
    if (!hasLocalSync && !s_loggedNoLocalSync)
    {
        std::cout << "[NetworkSyncSystem] No local sync object found." << std::endl;
        s_loggedNoLocalSync = true;
    }
    if (hasLocalSync && !s_loggedLocalSync)
    {
        std::cout << "[NetworkSyncSystem] Local sync upload active." << std::endl;
        s_loggedLocalSync = true;
    }

    // 2. Apply remote transforms ─────────────────────────────────────
    ApplyRemoteBroadcast(world, client);
    ApplyRemoteDespawn(world, client);
    ApplyRemoteInterpolation(world, client);
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
        // Skip our own echo.
        if (remote.clientID == localID)
            continue;

        // Ensure the corresponding remote object exists in world.
        GameObject *target = FindOrSpawnRemoteObject(world, remote.clientID, remote.objectID);

        if (target == nullptr)
            continue;

        const uint64_t key = MakeRemoteKey(remote.clientID, remote.objectID);
        auto &track = m_remoteTracks[key];

        RemoteSnapshot snap{};
        snap.serverTick = remote.serverTick;
        snap.receiveTimeSec = nowSec;
        snap.position = Vector3f(remote.transform.posX,
                                 remote.transform.posY,
                                 remote.transform.posZ);
        snap.rotation = Quat4f(remote.transform.rotW,
                               remote.transform.rotX,
                               remote.transform.rotY,
                               remote.transform.rotZ);

        if (!track.snapshots.empty())
        {
            const auto &last = track.snapshots.back();
            if (snap.serverTick < last.serverTick)
                continue;
            if (snap.serverTick == last.serverTick)
            {
                track.snapshots.back() = snap;
                continue;
            }
        }

        track.snapshots.push_back(snap);
        while (track.snapshots.size() > m_maxSnapshotsPerTrack)
            track.snapshots.pop_front();
    }

    m_pendingRemote.clear();
}

void NetworkSyncSystem::ApplyRemoteInterpolation(GameWorld &world, NetworkClient &client)
{
    if (m_remoteTracks.empty())
        return;

    const double nowSec = NowSeconds();
    const double renderTimeSec = nowSec - m_interpolationBackTimeSec;
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

        Vector3f outputPos = track.snapshots.back().position;
        Quat4f outputRot = track.snapshots.back().rotation;

        const auto &snaps = track.snapshots;
        bool usedInterpolation = false;

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

            outputPos = a.position + (b.position - a.position) * t;
            outputRot = Quat4f::slerp(a.rotation, b.rotation, t);
            usedInterpolation = true;
            break;
        }

        if (!usedInterpolation && snaps.size() >= 2 && renderTimeSec > snaps.back().receiveTimeSec)
        {
            const auto &last = snaps.back();
            const auto &prev = snaps[snaps.size() - 2];

            const double dt = last.receiveTimeSec - prev.receiveTimeSec;
            Vector3f vel = Vector3f::ZERO;
            if (dt > 1e-6)
                vel = (last.position - prev.position) * static_cast<float>(1.0 / dt);

            double extrapolationSec = renderTimeSec - last.receiveTimeSec;
            if (extrapolationSec < 0.0)
                extrapolationSec = 0.0;
            if (extrapolationSec > m_maxExtrapolationSec)
                extrapolationSec = m_maxExtrapolationSec;

            outputPos = last.position + vel * static_cast<float>(extrapolationSec);
            outputRot = last.rotation;
        }

        auto &tf = itObj->second->GetComponent<TransformComponent>();

        tf.SetWorldMatrix(Matrix4f::CreateTransform(outputPos, outputRot, Vector3f::ONE));

        while (track.snapshots.size() > 2 &&
               track.snapshots[1].receiveTimeSec < (renderTimeSec - 0.25))
        {
            track.snapshots.pop_front();
        }
    }
}

GameObject *NetworkSyncSystem::FindOrSpawnRemoteObject(GameWorld &world, ClientID ownerClientID, NetObjectID objectID)
{
    auto syncedEntities = world.GetEntitiesWith<NetworkSyncComponent, TransformComponent>();
    for (auto *obj : syncedEntities)
    {
        if (obj == nullptr || !obj->HasComponent<NetworkSyncComponent>() || !obj->HasComponent<TransformComponent>())
            continue;
        auto &sync = obj->GetComponent<NetworkSyncComponent>();
        if (sync.ownerClientID == ownerClientID && sync.netObjectID == objectID)
            return obj;
    }

    const std::string remoteName = "remote_player_" + std::to_string(ownerClientID) + "_" + std::to_string(objectID);
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

    std::cout << "[NetworkSyncSystem] Spawned remote player client="
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
        if (despawn.ownerClientID == localID)
            continue;
        const uint64_t key = MakeRemoteKey(despawn.ownerClientID, despawn.objectID);
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
            std::cout << "[NetworkSyncSystem] Despawn remote player client="
                      << despawn.ownerClientID << " obj=" << despawn.objectID << "\n";
            break;
        }
    }

    m_pendingDespawn.clear();
}

uint64_t NetworkSyncSystem::MakeRemoteKey(ClientID ownerClientID, NetObjectID objectID)
{
    return (static_cast<uint64_t>(ownerClientID) << 32) |
           static_cast<uint64_t>(objectID);
}
