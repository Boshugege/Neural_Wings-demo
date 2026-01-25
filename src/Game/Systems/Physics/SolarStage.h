#pragma once
#include "Engine/System/Physics/IPhysicsStage.h"
#include "Engine/System/Input/InputManager.h"
#include "raylib.h"
class GameWorld;
class SolarStage : public IPhysicsStage
{
public:
    SolarStage(float G =0.1f);

    void Execute(GameWorld &world, float fixedDeltaTime) override;

private:
    float m_G = 0.1f;
};