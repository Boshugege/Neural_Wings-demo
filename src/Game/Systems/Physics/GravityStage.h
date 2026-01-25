#pragma once
#include "Engine/System/Physics/IPhysicsStage.h"
#include "Engine/System/Input/InputManager.h"
#include "raylib.h"
class GameWorld;
class GravityStage : public IPhysicsStage
{
public:
    GravityStage(Vector3 gravity= {0.0f, -9.8f, 0.0f} );

    void Execute(GameWorld &world, float fixedDeltaTime) override;

private:
    Vector3 m_gravity= {0.0f, -9.8f, 0.0f};
};