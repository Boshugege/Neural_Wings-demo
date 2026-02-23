#include "EntityPlateHud.h"

#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/TransformComponent.h"
#include "Engine/Network/Client/NetworkClient.h"
#include "Engine/Network/Sync/NetworkSyncComponent.h"
#include "Engine/Graphics/Camera/CameraManager.h"
#include "Engine/Graphics/Camera/mCamera.h"

#include <cmath>

EntityPlateHud::EntityPlateHud(GameWorld *world)
    : m_world(world)
{
}

void EntityPlateHud::OnEnter()
{
}

void EntityPlateHud::FixedUpdate(float fixedDeltaTime)
{
    (void)fixedDeltaTime;
}

void EntityPlateHud::Update(float deltaTime)
{
    (void)deltaTime;
}

void EntityPlateHud::Draw()
{
    if (!m_world)
        return;

    auto *mainCamera = m_world->GetCameraManager().GetMainCamera();
    if (!mainCamera)
        return;

    NetworkClient &netClient = m_world->GetNetworkClient();
    const ClientID localClientID = netClient.GetLocalClientID();

    auto remoteEntities = m_world->GetEntitiesWith<NetworkSyncComponent, TransformComponent>();
    for (const auto *obj : remoteEntities)
    {
        if (!obj || !obj->HasComponent<NetworkSyncComponent>() || !obj->HasComponent<TransformComponent>())
            continue;

        const auto &sync = obj->GetComponent<NetworkSyncComponent>();
        if (sync.isLocalPlayer || sync.ownerClientID == INVALID_CLIENT_ID || sync.ownerClientID == localClientID)
            continue;

        std::string displayName = netClient.GetPlayerNickname(sync.ownerClientID);
        if (displayName.empty())
            displayName = "Player " + std::to_string(sync.ownerClientID);

        const auto &tf = obj->GetComponent<TransformComponent>();
        Vector3f anchorWorldPos = tf.GetWorldPosition();
        anchorWorldPos.y() += m_anchorHeight;

        Vector2 screenPos{};
        if (!TryProjectToScreen(anchorWorldPos, *mainCamera, screenPos))
            continue;

        DrawNameplate(displayName, screenPos);
    }
}

void EntityPlateHud::OnExit()
{
}

bool EntityPlateHud::TryProjectToScreen(const Vector3f &worldPos,
                                        const mCamera &camera,
                                        Vector2 &outScreenPos) const
{
    const Vector3f toTarget = worldPos - camera.Position();
    if ((toTarget * camera.Direction()) <= 0.01f)
        return false;

    outScreenPos = GetWorldToScreen(worldPos, camera.GetConstRawCamera());

    constexpr float kViewportMargin = 12.0f;
    const float minX = -kViewportMargin;
    const float maxX = static_cast<float>(GetScreenWidth()) + kViewportMargin;
    const float minY = -kViewportMargin;
    const float maxY = static_cast<float>(GetScreenHeight()) + kViewportMargin;
    if (outScreenPos.x < minX || outScreenPos.x > maxX ||
        outScreenPos.y < minY || outScreenPos.y > maxY)
    {
        return false;
    }
    return true;
}

void EntityPlateHud::DrawNameplate(const std::string &displayName,
                                   const Vector2 &screenPos) const
{
    if (displayName.empty())
        return;

    constexpr int kFontSize = 16;
    constexpr int kPadX = 7;
    constexpr int kPadY = 4;
    constexpr float kVerticalOffset = 24.0f;

    const int textWidth = MeasureText(displayName.c_str(), kFontSize);
    const int x = static_cast<int>(std::lround(screenPos.x - (textWidth * 0.5f)));
    const int y = static_cast<int>(std::lround(screenPos.y - kVerticalOffset));

    DrawRectangle(x - kPadX,
                  y - kPadY,
                  textWidth + kPadX * 2,
                  kFontSize + kPadY * 2,
                  Fade(BLACK, 0.42f));
    DrawText(displayName.c_str(), x + 1, y + 1, kFontSize, Fade(BLACK, 0.95f));
    DrawText(displayName.c_str(), x, y, kFontSize, Color{255, 50, 50, 255});
}
