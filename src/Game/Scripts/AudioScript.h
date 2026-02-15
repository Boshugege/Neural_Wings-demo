#pragma once
#include "Engine/Core/Components/Components.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
class AudioScript : public IScriptableComponent
{
public:
    AudioScript() = default;
    void OnCreate() override;
    void OnUpdate(float deltaTime) override;

private:
    Vector3f m_angluarVelocity = Vector3f(0, 1, 0);
};