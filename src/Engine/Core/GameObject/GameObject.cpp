#include "GameObject.h"
#include <iostream>

unsigned int GameObject::s_nextID = 0;

GameObject::GameObject()
    : m_id(s_nextID++)
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
// 用于删除GameObject
void GameObject::Destroy()
{
    m_isWaitingDestroy = true;
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
        Vector3f center = tf.position;
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
    Vector3f axis = tf.rotation.getAxisAngle(&angle);
    Vector3f worldPos = tf.position;
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