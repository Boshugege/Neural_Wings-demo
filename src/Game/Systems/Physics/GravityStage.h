#pragma once
#include "raylib.h"
#include "Engine/Engine.h"
class GameWorld;
class GravityStage : public IPhysicsStage
{
public:
    GravityStage(Vector3f gravity = Vector3f(0.0f, -9.8f, 0.0f));

    void Execute(GameWorld &world, float fixedDeltaTime) override;

private:
    float ground = 0.0f;
    float e_ground = 0.6f;
    float mu = 0.1f;
    //强制阻尼
    float friction = 0.5f;
    Vector3f m_gravity = Vector3f(0.0f, -9.8f, 0.0f);
    float baumgarte=0.9f;
    float slop =0.0f;

};