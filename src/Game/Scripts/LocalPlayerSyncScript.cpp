#include "LocalPlayerSyncScript.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/GameObject/GameObject.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/Network/Sync/NetworkSyncComponent.h"
#include <iostream>

void LocalPlayerSyncScript::Initialize(const json &data)
{
    m_netObjectID = data.value("netObjectID", 1u);
}

void LocalPlayerSyncScript::OnCreate()
{
    if (owner == nullptr)
        return;
    if (!owner->HasComponent<TransformComponent>())
        owner->AddComponent<TransformComponent>();
    owner->GetComponent<TransformComponent>().SetOwner(owner);

    if (!owner->HasComponent<NetworkSyncComponent>())
    {
        auto &sync = owner->AddComponent<NetworkSyncComponent>(m_netObjectID, true);
        sync.ownerClientID = INVALID_CLIENT_ID;
    }
    else
    {
        auto &sync = owner->GetComponent<NetworkSyncComponent>();
        sync.netObjectID = m_netObjectID;
        sync.isLocalPlayer = true;
        sync.ownerClientID = INVALID_CLIENT_ID;
    }

    std::cout << "[LocalPlayerSyncScript] Ready on plane object="
              << owner->GetName() << ", netObjectID=" << m_netObjectID << "\n";
}

void LocalPlayerSyncScript::OnUpdate(float /*deltaTime*/)
{
    // No-op: the plane entity's TransformComponent and RigidBodyComponent
    // are updated by the physics system and PlayerControlScript.
    // NetworkSyncSystem reads them directly for upload.
}
