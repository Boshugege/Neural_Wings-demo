#include "NetworkSyncSystem.h"
#include "NetworkSyncComponent.h"
#include "Engine/Network/Client/NetworkClient.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/TransformComponent.h"
#include <iostream>

// ── Init ───────────────────────────────────────────────────────────
void NetworkSyncSystem::Init(NetworkClient &client)
{
    if (m_callbackBound)
        return;

    client.SetOnPositionBroadcast(
        [this](const std::vector<NetBroadcastEntry> &entries)
        {
            m_pendingRemote.clear();
            m_pendingRemote.reserve(entries.size());
            for (auto &e : entries)
            {
                m_pendingRemote.push_back({e.clientID, e.objectID, e.transform});
            }
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
    for (auto *obj : syncedEntities)
    {
        auto &sync = obj->GetComponent<NetworkSyncComponent>();
        if (!sync.isLocalPlayer)
            continue;

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

    // 2. Apply remote transforms ─────────────────────────────────────
    ApplyRemoteBroadcast(world, client);
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
        bool found = false;
        for (auto *obj : syncedEntities)
        {
            auto &sync = obj->GetComponent<NetworkSyncComponent>();
            if (sync.ownerClientID == remote.clientID &&
                sync.netObjectID == remote.objectID)
            {
                auto &tf = obj->GetComponent<TransformComponent>();

                Vector3f pos = (Vector3f(remote.transform.posX,
                                         remote.transform.posY,
                                         remote.transform.posZ));
                Quat4f rot = (Quat4f(remote.transform.rotW,
                                     remote.transform.rotX,
                                     remote.transform.rotY,
                                     remote.transform.rotZ));
                tf.SetWorldMatrix(Matrix4f::CreateTransform(pos, rot, Vector3f::ONE));
                found = true;
                break;
            }
        }

        if (!found)
        {
            // TODO Phase 2: auto-spawn remote player GameObject here.
            // For now just log it.
            std::cout << "[NetworkSyncSystem] No GO for remote client="
                      << remote.clientID << " obj=" << remote.objectID << "\n";
        }
    }

    m_pendingRemote.clear();
}
