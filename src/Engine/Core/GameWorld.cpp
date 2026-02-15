#include "GameWorld.h"
#include <algorithm>
#include "Engine/System/System.h"
#include "Engine/Graphics/Graphics.h"
#include <string>

GameWorld::GameWorld(std::function<void(ScriptingFactory &, PhysicsStageFactory &, ParticleFactory &)> configCallback,
                     ResourceManager *resourceManager,
                     AudioManager *audioManager,
                     const std::string &cameraConfigPath,
                     const std::string &sceneConfigPath,
                     const std::string &inputConfigPath,
                     const std::string &renderView,
                     const std::string &effectLibPath)
    : m_resourceManager(resourceManager),
      m_audioManager(audioManager),
      m_nextObjectID(0)
{
    m_timeManager = std::make_unique<TimeManager>();
    m_cameraManager = std::make_unique<CameraManager>();
    m_inputManager = std::make_unique<InputManager>();
    m_physicsSystem = std::make_unique<PhysicsSystem>();
    m_physicsStageFactory = std::make_unique<PhysicsStageFactory>();
    m_scriptingFactory = std::make_unique<ScriptingFactory>();
    m_scriptingSystem = std::make_unique<ScriptingSystem>();
    m_eventManager = std::make_unique<EventManager>();
    m_renderer = std::make_unique<Renderer>();
    m_particleFactory = std::make_unique<ParticleFactory>();
    m_particleSystem = std::make_unique<ParticleSystem>(this);

    // NetworkClient is injected by ScreenManager via SetNetworkClient().
    m_networkSyncSystem = std::make_unique<NetworkSyncSystem>();

    configCallback(*m_scriptingFactory, *m_physicsStageFactory, *m_particleFactory);

    m_cameraManager->LoadConfig(cameraConfigPath);
    m_sceneManager->LoadScene(sceneConfigPath, *this);
    m_particleSystem->LoadEffectLibrary(effectLibPath);

    m_renderer->Init(renderView, *this);

    if (!m_inputManager->LoadBindings(inputConfigPath))
    {
        std::cerr << "Error: [GameplayScreen] Could not load input bindings.Use default bindings instead." << std::endl;
        m_inputManager->LoadBindings("assets/config/default/input_config.json");
    }
}

GameWorld::~GameWorld()
{
    OnDestroy();
}
void GameWorld::OnDestroy()
{
    for (auto &obj : m_gameObjects)
    {
        obj->SetIsWaitingDestroy(true);
    }
    DestroyWaitingObjects();
    m_gameObjects.clear();

    m_audioManager->ClearOneShots();
    m_resourceManager->GameWorldUnloadAll();
}

GameObject &GameWorld::CreateGameObject()
{
    auto newObject = std::make_unique<GameObject>(m_nextObjectID++);
    GameObject *rawPtr = newObject.get();
    m_gameObjects.push_back(std::move(newObject));
    return *rawPtr;
}

// 返回true表示游戏继续，返回false表示游戏结束
bool GameWorld::FixedUpdate(float fixedDeltaTime)
{
    m_timeManager->TickGame(fixedDeltaTime);

    m_scriptingSystem->FixedUpdate(*this, fixedDeltaTime);
    this->SyncActiveEntities();
    this->UpdateTransforms();

    m_physicsSystem->Update(*this, fixedDeltaTime);
    this->SyncActiveEntities();
    this->UpdateTransforms();

    this->DestroyWaitingObjects();
    return true;
}

bool GameWorld::Update(float DeltaTime)
{
    m_timeManager->Tick();

    m_scriptingSystem->Update(*this, DeltaTime);
    this->SyncActiveEntities();

    m_particleSystem->Update(*this, DeltaTime);
    this->UpdateTransforms();

    mCamera *activeCam = m_cameraManager->GetMainCamera();
    if (activeCam)
    {
        m_audioManager->Update(*this, *activeCam);
    // Network: poll incoming packets and sync transforms.
    if (m_networkClient)
    {
        m_networkClient->Poll();
        if (m_networkSyncSystem)
            m_networkSyncSystem->Update(*this, *m_networkClient);
    }

    return true;
}

void GameWorld::UpdateTransforms()
{
    for (auto &obj : m_gameObjects)
    {
        if (obj->IsWaitingDestroy())
            continue;
        if (!obj->HasComponent<TransformComponent>())
            continue;
        auto &tf = obj->GetComponent<TransformComponent>();
        if (tf.GetParent() == nullptr)
        {
            UpdateHierarchyLogic(obj.get(), Matrix4f::identity());
        }
    }
}

void GameWorld::UpdateHierarchyLogic(GameObject *obj, const Matrix4f &parentWorldMatrix)
{
    auto &tf = obj->GetComponent<TransformComponent>();
    if (tf.isDirty)
    {
        Matrix4f localMat = tf.GetLocalMatrix();
        tf.SetWorldMatrix(parentWorldMatrix * localMat);
        tf.SetClean();
    }
    for (auto *child : tf.GetChildren())
    {
        if (child && !child->IsWaitingDestroy())
            UpdateHierarchyLogic(child, tf.GetWorldMatrix());
    }
}
const std::vector<std::unique_ptr<GameObject>> &GameWorld::GetGameObjects() const
{
    return m_gameObjects;
}
const std::vector<GameObject *> &GameWorld::GetActivateGameObjects() const
{
    return m_activateGameObjects;
}
void GameWorld::DestroyWaitingObjects()
{
    bool anyObjectDestroyed = false;
    for (auto &obj : m_gameObjects)
    {
        if (obj->IsWaitingDestroy())
        {
            // 先释放脚本等组件，防止析构时先析构其他组件导致脚本崩溃
            obj->OnDestroy();
            if (obj->IsActive())
                NotifyActivateStateChanged(obj.get(), false);
            anyObjectDestroyed = true;
        }
    }
    if (anyObjectDestroyed)
    {
        m_gameObjects.erase(
            std::remove_if(
                m_gameObjects.begin(),
                m_gameObjects.end(),
                [](const std::unique_ptr<GameObject> &object)
                {
                    return object->IsWaitingDestroy();
                }),
            m_gameObjects.end());
    }
}
void GameWorld::Render()
{
    m_renderer->RenderScene(*this, *m_cameraManager);
}

void GameWorld::NotifyActivateStateChanged(GameObject *obj, bool active)
{
    m_activeChanges.push({obj, active});
    // if (activate)
    // {
    //     m_activateGameObjects.push_back(obj);
    // }
    // else
    // {
    //     auto it = std::find(m_activateGameObjects.begin(), m_activateGameObjects.end(), obj);
    //     if (it != m_activateGameObjects.end())
    //     {
    //         *it = m_activateGameObjects.back();
    //         m_activateGameObjects.pop_back();
    //     }
    // }
}
void GameWorld::SyncActiveEntities()
{
    while (!m_activeChanges.empty())
    {
        auto change = m_activeChanges.front();
        m_activeChanges.pop();
        auto it = std::find(m_activateGameObjects.begin(), m_activateGameObjects.end(), change.obj);
        bool currentlyInList = (it != m_activateGameObjects.end());
        if (change.newState && !currentlyInList)
        {
            m_activateGameObjects.push_back(change.obj);
        }
        else if (!change.newState && currentlyInList)
        {
            *it = m_activateGameObjects.back();
            m_activateGameObjects.pop_back();
        }
    }
}

GameObject *GameWorld::FindEntityByName(const std::string &name) const
{
    for (auto &obj : m_gameObjects)
    {
        if (obj->GetName() == name)
            return obj.get();
    }
    return nullptr;
}
GameObjectPool &GameWorld::GetOrCreatePool(const std::string &name, const std::string &prefabPath, size_t preloadCount)
{
    if (m_pools.find(name) == m_pools.end())
    {
        auto pool = std::make_unique<GameObjectPool>(prefabPath, *this);
        if (preloadCount > 0)
            pool->Preload(preloadCount);
        m_pools[name] = std::move(pool);
        std::cout << "[GameWorld]: Created pool: " << name << " using prefab: " << prefabPath << std::endl;
    }
    return *m_pools[name];
}
GameObjectPool &GameWorld::GetPool(const std::string &name) const
{
    auto it = m_pools.find(name);
    if (it != m_pools.end())
    {
        return *it->second;
    }
    throw std::runtime_error("[GameWorld]:Pool not found: " + name);
}
