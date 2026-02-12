#include "GameObject.h"
#include <iostream>
#include <string>

GameObject::GameObject(unsigned int s_nextID, std::string name, std::string tag)
    : m_id(s_nextID), m_name(name), m_tag(tag), m_isWaitingDestroy(false), m_isDestroyed(false)
{
}
GameObject::~GameObject()
{
    std::cout << "Destroying GameObject " << m_id << std::endl;
}

unsigned int GameObject::GetID() const
{
    return m_id;
}

void GameObject::SetName(const std::string &name)
{
    m_name = name;
}
void GameObject::SetTag(const std::string &tag)
{
    m_tag = tag;
}
std::string GameObject::GetName() const
{
    return m_name;
}
std::string GameObject::GetTag() const
{
    return m_tag;
}

GameWorld *GameObject::GetOwnerWorld() const
{
    return owner_world;
}
void GameObject::SetOwnerWorld(GameWorld *world)
{
    owner_world = world;
}

// 用于删除GameObject
// 先销毁脚本
#include "Engine/Core/GameWorld.h"
#include "Engine/Graphics/Particle/ParticleEmitter.h"
#include "Engine/Graphics/Particle/ParticleSystem.h"
void GameObject::OnDestroy()
{
    if (m_isDestroyed)
        return;
    m_isDestroyed = true;
    // 删除脚本
    if (this->HasComponent<ScriptComponent>())
    {
        auto &sc = this->GetComponent<ScriptComponent>();
        for (auto &script : sc.scripts)
        {
            if (script)
                script->OnDestroy();
        }
        sc.scripts.clear();
    }
    // 挂载遗留粒子
    if (this->HasComponent<ParticleEmitterComponent>() && this->HasComponent<TransformComponent>())
    {
        auto &ec = this->GetComponent<ParticleEmitterComponent>();
        const auto &tf = this->GetComponent<TransformComponent>();

        auto &particleSys = owner_world->GetParticleSystem();
        for (auto &emitter : ec.emitters)
        {
            if (emitter->simSpace == SimulationSpace::WORLD)
                particleSys.RegisterOrphan(emitter, tf);
        }
    }
    m_isWaitingDestroy = true;
}
void GameObject::SetIsWaitingDestroy(bool isWaitingDestroy)
{
    m_isWaitingDestroy = isWaitingDestroy;
}
bool GameObject::IsWaitingDestroy() const
{
    return m_isWaitingDestroy;
}

#include "Engine/Core/Components/Components.h"
#include "Engine/Math/Math.h"
AABB GameObject::GetWorldAABB(Vector3f (*outCorners)[8]) const
{
    if (!this->HasComponent<RigidbodyComponent>() || !this->HasComponent<TransformComponent>())
        return AABB();
    TransformComponent &tf = this->GetComponent<TransformComponent>();
    RigidbodyComponent &rb = this->GetComponent<RigidbodyComponent>();
    if (rb.colliderType == ColliderType::SPHERE)
    {
        // Vector3f center = tf.GetLocalPosition();
        Vector3f center = tf.GetWorldPosition();
        float radius = rb.boudingRadius;
        if (outCorners != nullptr)
        {
            (*outCorners)[0] = center + Vector3f(radius, radius, radius);
            (*outCorners)[1] = center + Vector3f(radius, radius, -radius);
            (*outCorners)[2] = center + Vector3f(radius, -radius, radius);
            (*outCorners)[3] = center + Vector3f(radius, -radius, -radius);
            (*outCorners)[4] = center + Vector3f(-radius, radius, radius);
            (*outCorners)[5] = center + Vector3f(-radius, radius, -radius);
            (*outCorners)[6] = center + Vector3f(-radius, -radius, radius);
            (*outCorners)[7] = center + Vector3f(-radius, -radius, -radius);
        }
        return AABB(center - Vector3f(radius, radius, radius), center + Vector3f(radius, radius, radius));
    }
    Vector3f min = rb.localAABB.min;
    Vector3f max = rb.localAABB.max;
    Vector3f corners[8] = {
        Vector3f(min.x(), min.y(), min.z()),
        Vector3f(min.x(), min.y(), max.z()),
        Vector3f(min.x(), max.y(), min.z()),
        Vector3f(min.x(), max.y(), max.z()),
        Vector3f(max.x(), min.y(), min.z()),
        Vector3f(max.x(), min.y(), max.z()),
        Vector3f(max.x(), max.y(), min.z()),
        Vector3f(max.x(), max.y(), max.z()),
    };
    float angle = 0.0f;

    // Vector3f axis = tf.GetLocalRotation().getAxisAngle(&angle);
    // Vector3f worldPos = tf.GetLocalPosition();
    Vector3f axis = tf.GetWorldRotation().getAxisAngle(&angle);
    Vector3f worldPos = tf.GetWorldPosition();

    Vector3f newMin = Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vector3f newMax = Vector3f(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
    for (int i = 0; i < 8; i++)
    {
        corners[i] = corners[i];
        corners[i].RotateByAxixAngle(axis, angle);
        corners[i] += worldPos;
        if (outCorners != nullptr)
            (*outCorners)[i] = corners[i];
        newMin = Vector3f::Min(newMin, corners[i]);
        newMax = Vector3f::Max(newMax, corners[i]);
    }
    return AABB{newMin, newMax};
}
