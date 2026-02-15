#include "LocalPlayerSyncScript.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/GameObject/GameObject.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/Network/Sync/NetworkSyncComponent.h"
#include "Engine/Graphics/Camera/CameraManager.h"
#include <iostream>

void LocalPlayerSyncScript::Initialize(const json &data)
{
    m_cameraName = data.value("cameraName", "");
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

    std::cout << "[LocalPlayerSyncScript] Ready on object="
              << owner->GetName() << ", netObjectID=" << m_netObjectID << "\n";
}

void LocalPlayerSyncScript::OnUpdate(float /*deltaTime*/)
{
    if (owner == nullptr || world == nullptr)
        return;
    if (!owner->HasComponent<TransformComponent>())
        return;

    auto &cameraManager = world->GetCameraManager();
    mCamera *cam = m_cameraName.empty() ? cameraManager.GetMainCamera() : cameraManager.GetCamera(m_cameraName);
    if (cam == nullptr)
        return;

    Vector3f forward = cam->Direction().Normalized();
    Vector3f up = cam->Up().Normalized();
    Vector3f right = (forward ^ up).Normalized();
    up = (right ^ forward).Normalized();

    auto &tf = owner->GetComponent<TransformComponent>();
    tf.SetLocalPosition(cam->Position());
    tf.SetLocalRotation(Quat4f::fromRotatedBasis(right, up, forward));
}
