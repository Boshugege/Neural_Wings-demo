#pragma once
#include <nlohmann/json.hpp>

class GameObject;
class IScriptableComponent
{
public:
    virtual ~IScriptableComponent() = default;

    virtual void OnCreate() {};
    virtual void OnUpdate(float deltaTime) {};
    virtual void OnFixedUpdate(float fixedDeltaTime) {};
    virtual void OnDestroy() {};

    virtual void Initialize(const nlohmann::json &json) {};

    GameObject *owner = nullptr;
};