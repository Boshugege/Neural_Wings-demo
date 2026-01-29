#pragma once
#include "Engine/System/Physics/IPhysicsStage.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/Math/Math.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class GameWorld;
class GravityStage : public IPhysicsStage
{
public:
    GravityStage(Vector3f gravity = Vector3f(0.0f, -9.8f, 0.0f));

    void Execute(GameWorld &world, float fixedDeltaTime) override;

    void Initialize(const json &config) override;

private:
    float ground = 0.0f;
    float e_ground = 1.0f;
    float mu = 0.1f;
    // 强制阻尼
    float friction = 0.5f;
    Vector3f m_gravity = Vector3f(0.0f, -9.8f, 0.0f);
    float baumgarte = 0.9f;
    float slop = 0.0f;
};