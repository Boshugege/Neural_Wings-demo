#include "GameObjectPool.h"
#include "GameObjectFactory.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/Components.h"

GameObjectPool::GameObjectPool(std::string prefab_path, GameWorld &world) : m_prefab_path(prefab_path), m_world(world)
{
}
GameObjectPool::~GameObjectPool()
{
    for (auto *obj : m_pool)
    {
        if (obj)
            obj->OnDestroy();
    }
    m_pool.clear();
    std::cout << "[ObjectPool]: Pool destroyed and objects marked for cleanup." << std::endl;
}
void GameObjectPool::Preload(size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        GameObject &obj = GameObjectFactory::CreateFromPrefab("PoolObj", "Pool", m_prefab_path, m_world);
        obj.SetActive(false);
        m_pool.push_back(&obj);
    }
}
GameObject *GameObjectPool::Spawn(const std::string &name, const Vector3f &position, const Quat4f &rotation)
{
    GameObject *obj = nullptr;
    if (m_pool.empty())
    {
        obj = &GameObjectFactory::CreateFromPrefab(name, "Pool", m_prefab_path, m_world);
    }
    else
    {
        obj = m_pool.back();
        m_pool.pop_back();
        obj->SetName(name);
    }
    if (!obj->HasComponent<TransformComponent>())
        obj->AddComponent<TransformComponent>();
    auto &tf = obj->GetComponent<TransformComponent>();
    tf.SetLocalPosition(position);
    tf.SetLocalRotation(rotation);
    tf.SetDirty();

    obj->SetActive(true);
    if (obj->HasComponent<ScriptComponent>())
    {
        for (auto &script : obj->GetComponent<ScriptComponent>().scripts)
            script->OnWake();
    }

    return obj;
}
void GameObjectPool::Recycle(GameObject *obj)
{
    if (obj == nullptr)
        return;
    if (obj->HasComponent<RigidbodyComponent>())
    {
        auto &rb = obj->GetComponent<RigidbodyComponent>();
        rb.velocity = Vector3f::ZERO;
        rb.angularVelocity = Vector3f::ZERO;
        rb.ClearForces();
    }
    if (obj->HasComponent<ScriptComponent>())
    {
        for (auto &script : obj->GetComponent<ScriptComponent>().scripts)
            script->OnSleep();
    }
    if (obj->HasComponent<AudioComponent>())
    {
        auto &audio = obj->GetComponent<AudioComponent>();
        for (auto &[name, clip] : audio.audioClips)
        {
            audio.Stop(name);
        }
    }

    obj->SetActive(false);
    m_pool.push_back(obj);
}