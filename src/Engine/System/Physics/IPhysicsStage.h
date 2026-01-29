#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;
class GameWorld;

class IPhysicsStage
{
public:
    virtual ~IPhysicsStage() = default;

    virtual void Execute(GameWorld &world, float fixedDeltaTime) = 0;
    virtual void Initialize(const json &data) {};
};