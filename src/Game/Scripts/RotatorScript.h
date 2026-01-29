#pragma once
#include "Engine/Core/Components/Components.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
class RotatorScript : public IScriptableComponent
{
public:
    RotatorScript() = default;
    void Initialize(const json &data) override;
    void OnFixedUpdate(float fixedDeltaTime) override;
    void OnDestroy() override;

private:
    Vector3f m_angluarVelocity = Vector3f(0, 1, 0);
};