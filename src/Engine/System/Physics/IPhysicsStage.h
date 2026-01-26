#pragma once

class GameWorld;

class IPhysicsStage {
public:
    virtual ~IPhysicsStage() = default;

    virtual void Execute(GameWorld& world, float fixedDeltaTime) = 0;
};