#pragma once
#include "raylib.h"
#include "Engine/Engine.h"
class GameWorld;
class GravityStage : public IPhysicsStage
{
public:
    GravityStage(Vector3f gravity= Vector3f(0.0f, -9.8f, 0.0f) );

    void Execute(GameWorld &world, float fixedDeltaTime) override;

private:
    Vector3f m_gravity= Vector3f(0.0f, -9.8f, 0.0f);
};