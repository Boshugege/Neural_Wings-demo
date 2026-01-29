#pragma once
#include "Engine/Core/GameWorld.h"

class ScriptingSystem
{
public:
    void Update(GameWorld &gameWorld, float deltaTime);
    void FixedUpdate(GameWorld &gameWorld, float fixedDeltaTime);
};