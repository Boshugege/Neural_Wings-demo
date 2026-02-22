#pragma once
#include "Engine/Core/Components/IScriptableComponent.h"
#include "Engine/Network/NetTypes.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

/// Attaches NetworkSyncComponent to the owning entity (the local plane)
/// and marks it as a local-player object so NetworkSyncSystem uploads
/// its flight state (transform + velocity) to the server.
/// No longer copies camera data — the plane's own physics drives the transform.
class LocalPlayerSyncScript : public IScriptableComponent
{
public:
    LocalPlayerSyncScript() = default;

    void Initialize(const json &data) override;
    void OnCreate() override;
    void OnUpdate(float deltaTime) override;

private:
    NetObjectID m_netObjectID = 1;
};
