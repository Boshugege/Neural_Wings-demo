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
    {
        ParseTransformComponent(gameObject, prefab);
    }
    else if (compName == "RenderComponent")
    {
        ParseRenderComponent(gameWorld, gameObject, prefab);
    }
    else if (compName == "RigidBodyComponent")
    {
        ParseRigidBodyComponent(gameObject, prefab);
    }
    else if (compName == "ScriptComponent")
    {
        ParseScriptComponent(gameWorld, gameObject, prefab);
    }
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

                mat.depthWrite = pData.value("depthWrite", true);
                mat.depthTest = pData.value("depthTest", true);

                if (pData.contains("fs"))
                {
                    mat.shader = rm.GetShader(pData.value("vs", "assets/shaders/default.vs"), pData["fs"]);
                }
                if (pData.contains("color"))
                    mat.baseColor = JsonParser::ToVector4f(pData["color"]);
                if (pData.contains("blendMode"))
                {
                    std::string blendMode = pData["blendMode"];
                    if (blendMode == "ADDITIVE")
                        mat.blendMode = BlendMode::BLEND_ADDITIVE;
                    else if (blendMode == "ALPHA")
                        mat.blendMode = BlendMode::BLEND_ALPHA;
                    else if (blendMode == "NONE")
                        mat.blendMode = -1;
                    else
                        std::cerr << "[GameObjectFactory]: Unknown blend mode: " << blendMode << std::endl;
                }

                if (pData.contains("cullFace"))
                {
                    if (pData["cullFace"] == "FRONT")
                        mat.cullFace = RL_CULL_FACE_FRONT;
                    else if (pData["cullFace"] == "BACK")
                        mat.cullFace = RL_CULL_FACE_BACK;
                    else
                        mat.cullFace = -1;
                }

                if (pData.contains("uniforms"))
                {
                    auto &uniformsData = pData["uniforms"];
                    for (auto &[uName, uValue] : uniformsData.items())
                    {
                        if (uValue.is_number())
                            mat.customFloats[uName] = uValue;
                        else if (uValue.is_array() && uValue.size() == 3)
                            mat.customVector3[uName] = JsonParser::ToVector3f(uValue);
                        else if (uValue.is_array() && uValue.size() == 4)
                            mat.customVector4[uName] = JsonParser::ToVector4f(uValue);
                        else
                            std::cerr << "[GameObjectFactory]: Unknown uniform type: " << uName << std::endl;
                    }
                }

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
        tf.position = JsonParser::ToVector3f(prefab["position"]);
    if (prefab.contains("scale"))
        tf.scale = JsonParser::ToVector3f(prefab["scale"]);
    if (prefab.contains("rotation"))
        tf.rotation = Quat4f(JsonParser::ToVector3f(prefab["rotation"]));
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
    rb.SetHitbox(tf.scale);
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