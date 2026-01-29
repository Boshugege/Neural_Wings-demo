#pragma once
#include "Engine/System/Physics/IPhysicsStage.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/Math/Math.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class GameWorld;

class CollisionStage : public IPhysicsStage
{
public:
    CollisionStage() = default;

    void Execute(GameWorld &world, float fixedDeltaTime) override;
    void ResolveCollision(GameObject *a, GameObject *b, const Vector3f &normal, float penetration, const Vector3f &hitPoint);
    void Initialize(const json &config) override;

private:
    float epsilon = 0.0001f;
};