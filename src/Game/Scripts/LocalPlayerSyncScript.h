#pragma once
#include "Engine/Core/Components/IScriptableComponent.h"
#include "Engine/Network/NetTypes.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class LocalPlayerSyncScript : public IScriptableComponent
{
public:
    LocalPlayerSyncScript() = default;

    void Initialize(const json &data) override;
    void OnCreate() override;
    void OnUpdate(float deltaTime) override;

private:
    std::string m_cameraName;
    NetObjectID m_netObjectID = 1;
};
