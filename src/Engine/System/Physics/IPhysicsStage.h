#pragma once

class GameWorld;

class IPhysicsStage {
public:
    virtual ~IPhysicsStage() = default;

    // 每一帧 FixedUpdate 调用。
    // GameWorld，计算力，并调用 Rigidbody::AddForce()。
    virtual void Execute(GameWorld& world, float fixedDeltaTime) = 0;
};