#pragma once
#include "GameObjectFactory.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/Utils/JsonParser.h"
#include "raylib.h"
#include "rlgl.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
using json = nlohmann::json;

GameObject &GameObjectFactory::CreateFromPrefab(const std::string &name, const std::string &tag, const std::string &path, GameWorld &world)
{
    std::ifstream file(path);
    json data = json::parse(file);
    file.close();

    GameObject &gameObject = world.CreateGameObject();
    gameObject.SetName(name);

    gameObject.SetTag(tag);

    if (data.contains("components"))
    {
        // 严格顺序，确保transform在rigid前面
        for (auto &comp : data["components"])
        {
            auto it = comp.begin();
            if (it != comp.end())
            {
                const std::string compName = it.key();
                const json &compData = it.value();
                ApplyComponent(world, gameObject, compName, compData);
            }
        }
    }
    return gameObject;
}

void GameObjectFactory::ApplyComponent(GameWorld &gameWorld, GameObject &gameObject, const std::string &compName, const json &prefab)
{
    if (compName == "TransformComponent")
        ParseTransformComponent(gameObject, prefab);
    else if (compName == "RenderComponent")
        ParseRenderComponent(gameWorld, gameObject, prefab);
    else if (compName == "RigidBodyComponent")
        ParseRigidBodyComponent(gameObject, prefab);
    else if (compName == "ScriptComponent")
        ParseScriptComponent(gameWorld, gameObject, prefab);
    else if (compName == "ParticleEmitterComponent")
        ParseParticleEmitterComponent(gameWorld, gameObject, prefab);
    else
        std::cerr << "Component " << compName << " not implemented" << std::endl;
}

void GameObjectFactory::ParseRenderComponent(GameWorld &gameWorld, GameObject &gameObject, const json &prefab)
{
    auto &rd = gameObject.AddComponent<RenderComponent>();
    auto &rm = gameWorld.GetResourceManager();
    rd.model = rm.GetModel(prefab.value("model", "primitive://cube"));

    rd.isVisible = prefab.value("isVisible", true);
    rd.showWires = prefab.value("showWires", false);
    rd.showAxes = prefab.value("showAxes", false);
    rd.showAngVol = prefab.value("showAngVol", false);
    rd.showVol = prefab.value("showVol", false);
    rd.showCenter = prefab.value("showCenter", false);

    if (prefab.contains("defaultMaterial"))
    {
        auto &matData = prefab["defaultMaterial"];

        if (matData.contains("fs"))
            rd.defaultMaterial.shader = rm.GetShader(matData.value("vs", "assets/shaders/default.vs"), matData["fs"]);
        if (matData.contains("color"))
            rd.defaultMaterial.baseColor = JsonParser::ToVector4f(matData["color"]);
    }

    if (prefab.contains("meshPasses"))
    {
        auto &matData = prefab["meshPasses"];
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
    if (prefab.contains("scale"))
        rd.scale = JsonParser::ToVector3f(prefab["scale"]);
}

void GameObjectFactory::ParseTransformComponent(GameObject &gameObject, const json &prefab)
{
    auto &tf = gameObject.AddComponent<TransformComponent>();
    if (prefab.contains("position"))
        tf.SetLocalPosition(JsonParser::ToVector3f(prefab["position"]));
    if (prefab.contains("scale"))
        tf.SetLocalScale(JsonParser::ToVector3f(prefab["scale"]));
    if (prefab.contains("rotation"))
        tf.SetLocalRotation(Quat4f(JsonParser::ToVector3f(prefab["rotation"])));
}
void GameObjectFactory::ParseRigidBodyComponent(GameObject &gameObject, const json &prefab)
{
    if (!gameObject.HasComponent<TransformComponent>())
    {
        std::cerr << "[GameObjectFactory]: RigidbodyComponent requires TransformComponent!!!" << std::endl;
        return;
    }
    auto &rb = gameObject.AddComponent<RigidbodyComponent>();
    auto &tf = gameObject.GetComponent<TransformComponent>();
    rb.mass = prefab.value("mass", 1.0f);
    rb.drag = prefab.value("drag", 0.0f);
    rb.angularDrag = prefab.value("angularDrag", 0.0f);
    rb.elasticity = prefab.value("elasticity", 0.0f);

    if (prefab.contains("velocity"))
        rb.velocity = JsonParser::ToVector3f(prefab["velocity"]);
    if (prefab.contains("angularVelocity"))
        rb.angularVelocity = JsonParser::ToVector3f(prefab["angularVelocity"]);

    std::string colliderType = prefab.value("colliderType", "BOX");
    if (colliderType == "BOX")
        rb.colliderType = ColliderType::BOX;
    else if (colliderType == "SPHERE")
        rb.colliderType = ColliderType::SPHERE;
    else
        std::cerr << "Unknown collider type: " << colliderType << std::endl;
    rb.SetHitbox(tf.GetLocalScale());
}
void GameObjectFactory::ParseScriptComponent(GameWorld &gameWorld, GameObject &gameObject, const json &prefab)
{

    auto &sc = gameObject.AddComponent<ScriptComponent>();
    auto &factory = gameWorld.GetScriptingFactory();
    for (auto &[scriptName, scriptData] : prefab.items())
    {
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

void GameObjectFactory::ParseParticleEmitterComponent(GameWorld &gameWorld, GameObject &gameObject, const json &data)
{
    auto &ec = gameObject.AddComponent<ParticleEmitterComponent>();
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