#pragma once
#include "Engine/System/Physics/IPhysicsStage.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/Math/Math.h"
class GameWorld;

class CollisionStage : public IPhysicsStage
{
public:
    CollisionStage(float eps = 0.0001f);

    void Execute(GameWorld &world, float fixedDeltaTime) override;
    void ResolveCollision(GameObject *a, GameObject *b, const Vector3f &normal, float penetration, const Vector3f &hitPoint);

private:
    float epsilon = 0.0001f;
};