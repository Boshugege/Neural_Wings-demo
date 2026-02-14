#pragma once
#include "Engine/Core/GameObject/GameObject.h"
#include "Engine/Core/GameObject/GameObjectPool.h"
#include "Engine/Core/Events/Events.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/System/System.h"
#include "Engine/System/Time/Time.h"
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <queue>

class ScriptingFactory;
class ScriptingSystem;

class GameWorld
{
public:
    GameWorld(std::function<void(ScriptingFactory &, PhysicsStageFactory &, ParticleFactory &)> configCallback,
              ResourceManager *resourceManager,
              AudioManager *audioManager,
              const std::string &cameraConfigPath = "assets/config/cameras_config.json",
              const std::string &sceneConfigPath = "assets/scenes/test_scene.json",
              const std::string &inputConfigPath = "assets/config/input_config.json",
              const std::string &renderView = "assets/view/test_view.json",
              const std::string &effectLibPath = "assets/Library/particle_effects.json");
    ~GameWorld();
    void OnDestroy();

    GameObject &CreateGameObject();
    bool FixedUpdate(float fexedDeltaTime);
    bool Update(float deltaTime);
    void Render();
    void UpdateTransforms();

    const std::vector<std::unique_ptr<GameObject>> &GetGameObjects() const;
    const std::vector<GameObject *> &GetActivateGameObjects() const;

    PhysicsStageFactory &GetPhysicsStageFactory() { return *m_physicsStageFactory; };
    PhysicsSystem &GetPhysicsSystem() { return *m_physicsSystem; };

    ScriptingFactory &GetScriptingFactory() { return *m_scriptingFactory; };
    ScriptingSystem &GetScriptingSystem() { return *m_scriptingSystem; };

    ResourceManager &GetResourceManager() { return *m_resourceManager; };

    Renderer &GetRenderer() { return *m_renderer; };
    CameraManager &GetCameraManager() { return *m_cameraManager; };
    InputManager &GetInputManager() { return *m_inputManager; };
    EventManager &GetEventManager() { return *m_eventManager; };

    TimeManager &GetTimeManager() { return *m_timeManager; };

    ParticleFactory &GetParticleFactory() { return *m_particleFactory; };
    ParticleSystem &GetParticleSystem() { return *m_particleSystem; };
    AudioManager &GetAudioManager() { return *m_audioManager; }

    template <typename... Components>
    std::vector<GameObject *> GetEntitiesWith()
    {
        std::vector<GameObject *> results;
        for (auto *obj : m_activateGameObjects)
        {
            if (!obj->IsWaitingDestroy() && (obj->HasComponent<Components>() && ...))
            {
                results.push_back(obj);
            }
        }
        return results;
    }

    void SyncActiveEntities();
    void NotifyActivateStateChanged(GameObject *obj, bool activate);

    GameObject *FindEntityByName(const std::string &name) const;

    GameObjectPool &GetOrCreatePool(const std::string &name, const std::string &prefab, size_t preloadCount = 0);
    GameObjectPool &GetPool(const std::string &name) const;

private:
    void UpdateHierarchyLogic(GameObject *obj, const Matrix4f &parentWorldMatrix);
    void DestroyWaitingObjects();

    std::unique_ptr<TimeManager> m_timeManager;

    unsigned m_nextObjectID = 0;
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    std::vector<GameObject *> m_activateGameObjects;

    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<CameraManager> m_cameraManager;
    std::unique_ptr<InputManager> m_inputManager;

    std::unique_ptr<PhysicsStageFactory> m_physicsStageFactory;
    std::unique_ptr<PhysicsSystem> m_physicsSystem;

    std::unique_ptr<ScriptingFactory> m_scriptingFactory;
    std::unique_ptr<ScriptingSystem> m_scriptingSystem;

    std::unique_ptr<SceneManager> m_sceneManager;

    ResourceManager *m_resourceManager;

    std::unique_ptr<EventManager> m_eventManager;

    std::unique_ptr<ParticleFactory> m_particleFactory;
    std::unique_ptr<ParticleSystem> m_particleSystem;

    struct ActiveChange
    {
        GameObject *obj;
        bool newState;
    };
    std::queue<ActiveChange> m_activeChanges;
    std::unordered_map<std::string, std::unique_ptr<GameObjectPool>> m_pools;

    AudioManager *m_audioManager;
};