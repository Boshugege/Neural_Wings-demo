#include "GameWorld.h"
#include <algorithm>
#include "Engine/System/System.h"
#include "Engine/Graphics/Graphics.h"
#include <string>

GameWorld::GameWorld(std::function<void(ScriptingFactory &, PhysicsStageFactory &, ParticleFactory &)> configCallback,
                     const std::string &cameraConfigPath,
                     const std::string &sceneConfigPath,
                     const std::string &inputConfigPath,
                     const std::string &renderView,
                     const std::string &effectLibPath)
{
    m_timeManager = std::make_unique<TimeManager>();

    m_nextObjectID = 0;
    m_cameraManager = std::make_unique<CameraManager>();
    m_inputManager = std::make_unique<InputManager>();
    m_physicsSystem = std::make_unique<PhysicsSystem>();
    m_physicsStageFactory = std::make_unique<PhysicsStageFactory>();
    m_resourceManager = std::make_unique<ResourceManager>();
    m_scriptingFactory = std::make_unique<ScriptingFactory>();
    m_scriptingSystem = std::make_unique<ScriptingSystem>();
    m_eventManager = std::make_unique<EventManager>();
    m_renderer = std::make_unique<Renderer>();
    m_particleFactory = std::make_unique<ParticleFactory>();
    m_particleSystem = std::make_unique<ParticleSystem>(this);

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
    this->UpdateTransforms();
    m_physicsSystem->Update(*this, fixedDeltaTime);
    this->UpdateTransforms();

    this->DestroyWaitingObjects();
    return true;
}

bool GameWorld::Update(float DeltaTime)
{
    m_timeManager->Tick();
    m_scriptingSystem->Update(*this, DeltaTime);
    m_particleSystem->Update(*this, DeltaTime);
    this->UpdateTransforms();
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

void GameWorld::DestroyWaitingObjects()
{
    bool anyObjectDestroyed = false;
    for (auto &obj : m_gameObjects)
    {
        if (obj->IsWaitingDestroy())
        {
            // 先释放脚本等组件，防止析构时先析构其他组件导致脚本崩溃
            obj->OnDestroy();
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

GameObject *GameWorld::FindEntityByName(const std::string &name) const
{
    for (auto &obj : m_gameObjects)
    {
        if (obj->GetName() == name)
            return obj.get();
    }
    return nullptr;
}