#include "SceneManager.h"
#include <string>
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/GameObject/GameObjectFactory.h"
#include "Engine/System/Physics/Physics.h"
#include "Engine/Utils/JsonParser.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
void SceneManager::ParsePhysics(const json &sceneData, GameWorld &gameWorld)
{
    auto &physicsSystem = gameWorld.GetPhysicsSystem();
    auto &factory = gameWorld.GetPhysicsStageFactory();
    auto stageJson = sceneData["physicsStage"];
    physicsSystem.ClearStages();
    for (auto &[stageName, stageConfig] : stageJson.items())
    {
        if (stageConfig.value("enable", false))
        {
            auto stage = factory.Create(stageName);
            if (stage)
            {
                stage->Initialize(stageConfig);
                physicsSystem.AddStage(std::move(stage));
            }
        }
    }
}

bool SceneManager::LoadScene(const std::string &scenePath, GameWorld &gameWorld, PhysicsSystem &physicsSystem)
{
    std::ifstream file(scenePath);
    if (!file.is_open())
    {
        std::cerr << "[SceneManager]: Failed to open scene file: " << scenePath << std::endl;
        return false;
    }
    json sceneData = json::parse(file);
    if (sceneData.contains("physics"))
    {
        ParsePhysics(sceneData["physics"], gameWorld);
    }
    if (sceneData.contains("entities"))
    {
        for (const auto &entityData : sceneData["entities"])
        {
            std::string prefabPath = entityData["prefab"];
            GameObject &obj = GameObjectFactory::CreateFromPrefab(prefabPath, gameWorld);
            auto &tf = obj.GetComponent<TransformComponent>();
            if (entityData.contains("position"))
            {
                tf.position += JsonParser::ToVector3f(entityData["position"]);
            }
            if (entityData.contains("rotation"))
            {
                tf.rotation = tf.rotation * Quat4f(JsonParser::ToVector3f(entityData["rotation"]));
            }
            if (entityData.contains("scale"))
            {
                tf.scale = tf.scale * JsonParser::ToVector3f(entityData["scale"]);
                if (obj.HasComponent<RigidbodyComponent>())
                {
                    auto &rb = obj.GetComponent<RigidbodyComponent>();
                    rb.SetHitbox(tf.scale);
                }
            }
        }
    }
}
