#include "SceneManager.h"
#include <string>
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/GameObject/GameObjectFactory.h"
#include "Engine/System/Physics/Physics.h"
#include "Engine/Utils/JsonParser.h"
#include "rlgl.h"
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

bool SceneManager::LoadScene(const std::string &scenePath, GameWorld &gameWorld)
{

    std::ifstream file(scenePath);
    if (!file.is_open())
    {
        std::cerr << "[SceneManager]: Failed to open scene file: " << scenePath << std::endl;
        return false;
    }
    PhysicsSystem &physicsSystem = gameWorld.GetPhysicsSystem();
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
            std::string objectName = entityData.value("name", "");
            std::string objectTag = entityData.value("tag", "Untagged");
            GameObject &obj = GameObjectFactory::CreateFromPrefab(objectName, objectTag, prefabPath, gameWorld);

            obj.SetOwnerWorld(&gameWorld);

            if (!obj.HasComponent<TransformComponent>())
                obj.AddComponent<TransformComponent>();
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
                // 逐分量相乘
                tf.scale = tf.scale & JsonParser::ToVector3f(entityData["scale"]);
                if (obj.HasComponent<RigidbodyComponent>())
                {
                    auto &rb = obj.GetComponent<RigidbodyComponent>();
                    rb.SetHitbox(tf.scale);
                }
            }

            if (entityData.contains("physics"))
            {
                AddRigidbody(obj, entityData["physics"]);
            }
            if (entityData.contains("render") && obj.HasComponent<RenderComponent>())
            {
                AddShaders(obj, entityData["render"], gameWorld);
            }
            if (entityData.contains("scripts"))
            {
                AddScripts(gameWorld, obj, entityData["scripts"]);
            }
            if (entityData.contains("particles"))
            {
                AddParticle(gameWorld, obj, entityData["particles"]);
            }
        }
    }
    return true;
}

void SceneManager::AddShaders(GameObject &gameObject, const json &renderData, GameWorld &gameWorld)
{
    auto &rd = gameObject.GetComponent<RenderComponent>();
    auto &rm = gameWorld.GetResourceManager();

    if (renderData.contains("renderScale"))
        rd.scale = rd.scale & JsonParser::ToVector3f(renderData["renderScale"]);

    rd.isVisible = renderData.value("isVisible", true);
    rd.showWires = renderData.value("showWires", false);
    rd.showAxes = renderData.value("showAxes", false);
    rd.showAngVol = renderData.value("showAngVol", false);
    rd.showVol = renderData.value("showVol", false);
    rd.showCenter = renderData.value("showCenter", false);

    if (renderData.contains("defaultMaterial"))
    {
        auto &matData = renderData["defaultMaterial"];

        if (matData.contains("fs"))
            rd.defaultMaterial.shader = rm.GetShader(matData.value("vs", "assets/shaders/default.vs"), matData["fs"]);
        if (matData.contains("color"))
            rd.defaultMaterial.baseColor = JsonParser::ToVector4f(matData["color"]);
    }

    if (renderData.contains("meshPasses"))
    {
        auto &matData = renderData["meshPasses"];
        for (const auto &entry : matData)
        {
            int meshIndex = entry.value("meshIndex", 0);
            std::vector<RenderMaterial> &passes = rd.meshPasses[meshIndex];

            for (const auto &pData : entry["passes"])
            {
                RenderMaterial mat;
                mat.LoadFromConfig(pData, rm);
                passes.push_back(mat);
            }
        }
    }
}

void SceneManager::AddRigidbody(GameObject &gameObject, const json &rigidData)
{
    if (!gameObject.HasComponent<RigidbodyComponent>() || !gameObject.HasComponent<TransformComponent>())
    {
        return;
    }
    auto &rb = gameObject.GetComponent<RigidbodyComponent>();
    rb.mass = rigidData.value("mass", rb.mass);
    rb.drag = rigidData.value("drag", rb.drag);
    rb.angularDrag = rigidData.value("angularDrag", rb.angularDrag);
    rb.elasticity = rigidData.value("elasticity", rb.elasticity);

    if (rigidData.contains("velocity"))
        rb.velocity = JsonParser::ToVector3f(rigidData["velocity"]);
    if (rigidData.contains("angularVelocity"))
        rb.angularVelocity = JsonParser::ToVector3f(rigidData["angularVelocity"]);
}

void SceneManager::AddScripts(GameWorld &gameWorld, GameObject &gameObject, const json &scripts)
{

    if (!gameObject.HasComponent<ScriptComponent>())
        gameObject.AddComponent<ScriptComponent>();
    auto &sc = gameObject.GetComponent<ScriptComponent>();
    auto &factory = gameWorld.GetScriptingFactory();

    for (auto &scriptData : scripts)
    {
        auto it = scriptData.begin();
        if (it != scriptData.end())
        {
            const std::string scriptName = it.key();
            const json &scriptData = it.value();

            auto script = factory.Create(scriptName);
            if (script)
            {
                script->world = &gameWorld;
                script->owner = &gameObject;
                script->Initialize(scriptData);
                script->OnCreate();
                sc.scripts.push_back(std::move(script));
            }
        }
    }
}
void SceneManager::AddParticle(GameWorld &gameWorld, GameObject &gameObject, const json &data)
{
    if (!gameObject.HasComponent<ParticleEmitterComponent>())
        gameObject.AddComponent<ParticleEmitterComponent>();
    auto &ec = gameObject.GetComponent<ParticleEmitterComponent>();
    ec.activate = data.value("activate", true);
    if (data.contains("emitters"))
    {
        for (const auto &emitterConfig : data["emitters"])
        {
            auto emitter = std::make_shared<ParticleEmitter>(emitterConfig, gameWorld.GetParticleFactory(), gameWorld.GetResourceManager());
            ec.AddEmitter(emitter);
        }
    }
}