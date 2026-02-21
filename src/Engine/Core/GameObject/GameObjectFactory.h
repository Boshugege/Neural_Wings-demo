#pragma once
#include "Engine/Core/GameObject/GameObject.h"
#include "Engine/Core/GameWorld.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class GameObjectFactory
{
public:
    static GameObject &CreateFromPrefab(const std::string &name, const std::string &tag, const std::string &path, GameWorld &world);

private:
    static void ApplyComponent(GameWorld &gameWorld, GameObject &gameObject, const std::string &compName, const json &prefab);
    static void ParseRigidBodyComponent(GameObject &gameObject, const json &prefab);
    static void ParseTransformComponent(GameObject &gameObject, const json &prefab);
    static void ParseScriptComponent(GameWorld &gameWorld, GameObject &gameObject, const json &prefab);
    static void ParseRenderComponent(GameWorld &gameWorld, GameObject &gameObject, const json &prefab);
    static void ParseParticleEmitterComponent(GameWorld &gameWorld, GameObject &gameObject, const json &prefab);
    static void ParseAudioComponent(GameWorld &gameWorld, GameObject &gameObject, const json &prefab);
    static void ParseLightComponent(GameWorld &gameWorld, GameObject &gameObject, const json &prefab);
};