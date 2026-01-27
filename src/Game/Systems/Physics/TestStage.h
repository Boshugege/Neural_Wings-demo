#pragma once
#include "Engine/System/Physics/IPhysicsStage.h"
#include "raylib.h"
class GameWorld;
class TestStage : public IPhysicsStage
{
public:
    TestStage();

    void Execute(GameWorld &world, float fixedDeltaTime) override;

private:
};