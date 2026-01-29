#include "ScriptingSystem.h"
#include "Engine/Core/Components/ScriptComponent.h"

void ScriptingSystem::Update(GameWorld &gameWorld, float deltaTime)
{
    for (auto &obj : gameWorld.GetGameObjects())
    {
        if (obj->HasComponent<ScriptComponent>())
        {
            auto &sc = obj->GetComponent<ScriptComponent>();
            for (auto &script : sc.scripts)
                script->OnUpdate(deltaTime);
        }
    }
}
void ScriptingSystem::FixedUpdate(GameWorld &gameWorld, float FixedDeltaTime)
{
    for (auto &obj : gameWorld.GetGameObjects())
    {
        if (obj->HasComponent<ScriptComponent>())
        {
            auto &sc = obj->GetComponent<ScriptComponent>();
            for (auto &script : sc.scripts)
                script->OnFixedUpdate(FixedDeltaTime);
        }
    }
}