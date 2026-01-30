#pragma once
#include <string>
#include <nlohmann/json.hpp>
class GameObject;
class GameWorld;
class PhysicsSystem;
using json = nlohmann::json;

class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager() = default;

    void ParsePhysics(const json &sceneData, GameWorld &gameWorld);

    bool LoadScene(const std::string &scenePath, GameWorld &gameWorld, PhysicsSystem &physicsSystem);

private:
    void AddScripts(GameWorld &world, GameObject &gameObject, const json &scriptData);

    void AddRigidbody(GameObject &gameObject, const json &rigidData);
};
