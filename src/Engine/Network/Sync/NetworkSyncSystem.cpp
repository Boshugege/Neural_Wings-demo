#include "NetworkSyncSystem.h"
#include "NetworkSyncComponent.h"
#include "Engine/Network/Client/NetworkClient.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/TransformComponent.h"
#include "Engine/Core/GameObject/GameObjectFactory.h"
#include <iostream>
#include <string>

// ── Init ───────────────────────────────────────────────────────────
void NetworkSyncSystem::Init(NetworkClient &client)
{
    if (m_callbackBound)
        return;

    client.SetOnPositionBroadcast(
        [this](const std::vector<NetBroadcastEntry> &entries)
        {
            static int s_lastCount = -1;
            if (static_cast<int>(entries.size()) != s_lastCount)
            {
                s_lastCount = static_cast<int>(entries.size());
                std::cout << "[NetworkSyncSystem] PositionBroadcast entries=" << entries.size() << std::endl;
            }
            m_pendingRemote.clear();
            m_pendingRemote.reserve(entries.size());
            for (auto &e : entries)
            {
                m_pendingRemote.push_back({e.clientID, e.objectID, e.transform});
            }
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
        return;

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
}

// ── Apply remote ───────────────────────────────────────────────────
void NetworkSyncSystem::ApplyRemoteBroadcast(GameWorld &world,
                                             NetworkClient &client)
{
    if (m_pendingRemote.empty())
        return;

    ClientID localID = client.GetLocalClientID();

    auto syncedEntities = world.GetEntitiesWith<NetworkSyncComponent, TransformComponent>();

    for (auto &remote : m_pendingRemote)
    {
        // Skip our own echo.
        if (remote.clientID == localID)
            continue;

        // Find the matching remote GameObject.
        GameObject *target = nullptr;
        for (auto *obj : syncedEntities)
        {
            if (obj == nullptr || !obj->HasComponent<NetworkSyncComponent>() || !obj->HasComponent<TransformComponent>())
                continue;
            auto &sync = obj->GetComponent<NetworkSyncComponent>();
            if (sync.ownerClientID == remote.clientID &&
                sync.netObjectID == remote.objectID)
            {
                target = obj;
                break;
            }
        }

        if (target == nullptr)
        {
            target = FindOrSpawnRemoteObject(world, remote.clientID, remote.objectID);
            if (target != nullptr)
                syncedEntities.push_back(target);
        }

        if (target == nullptr)
            continue;

        auto &tf = target->GetComponent<TransformComponent>();
        tf.SetLocalPosition(Vector3f(remote.transform.posX,
                                     remote.transform.posY,
                                     remote.transform.posZ));
        tf.SetLocalRotation(Quat4f(remote.transform.rotW,
                                   remote.transform.rotX,
                                   remote.transform.rotY,
                                   remote.transform.rotZ));
    }

    m_pendingRemote.clear();
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
