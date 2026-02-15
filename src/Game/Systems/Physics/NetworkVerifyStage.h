#pragma once
#include "Engine/System/Physics/IPhysicsStage.h"

class GameWorld;
class NetworkVerifyStage : public IPhysicsStage
{
public:
    NetworkVerifyStage() = default;

    void Execute(GameWorld &world, float fixedDeltaTime) override;
    void Initialize(const json &config) override;
};
