#pragma once
#include "Engine/Core/GameObject/GameObject.h"
#include "Engine/Core/GameWorld.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class GameObjectFactory
{
public:
    static GameObject &CreateFromPrefab(const std::string &path, GameWorld &world);

private:
    static void ApplyComponent(GameWorld &world, GameObject &gameObject, const std::string &compName, const json &prefab);
};