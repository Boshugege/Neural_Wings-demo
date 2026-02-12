#include "CollisionStage.h"
#include "CollisionEvent.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/Components.h"
#include <limits>

void CollisionStage::Initialize(const json &config) {};

void CollisionStage::Execute(GameWorld &world, float fixedDeltaTime)
{
    auto &gameObjects = world.GetGameObjects();
    // TODO:优化

    for (size_t i = 0; i < gameObjects.size(); i++)
    {
        auto &go1 = gameObjects[i];
        if (!go1->HasComponent<RigidbodyComponent>() || !go1->HasComponent<TransformComponent>())
            continue;

        auto &rb1 = go1->GetComponent<RigidbodyComponent>();
        if (!rb1.Collidable)
            continue;

        for (size_t j = i + 1; j < gameObjects.size(); j++)
        {
            auto &go2 = gameObjects[j];
            if (go1 == go2)
                continue;
            if (!go2->HasComponent<RigidbodyComponent>() || !go2->HasComponent<TransformComponent>())
                continue;

            auto &rb2 = go2->GetComponent<RigidbodyComponent>();
            if (!rb2.Collidable)
                continue;

            auto &tf1 = go1->GetComponent<TransformComponent>();
            auto &tf2 = go2->GetComponent<TransformComponent>();

            // TODO:实现更多BoundBox,优化碰撞检测
            bool isColliding = false;
            Vector3f normal;
            Vector3f hitPoint;
            float penetration = 0.0f;

            // Broad Phase
            AABB aabb1 = go1->GetWorldAABB();
            AABB aabb2 = go2->GetWorldAABB();
            if (!AABB::IsCollide(aabb1, aabb2))
                continue;
            // Narrow Phase
            HitBox box1(tf1, rb1);
            HitBox box2(tf2, rb2);

            isColliding = HitBox::GetCollisionInfo(box1, box2, normal, penetration, hitPoint);

            if (isColliding)
            {
                if (rb1.collisionCallback)
                    rb1.collisionCallback(go2.get());
                if (rb2.collisionCallback)
                    rb2.collisionCallback(go1.get());
                ResolveCollision(world, go1.get(), go2.get(), normal, penetration, hitPoint);
            }
        }
    }
}
float GetInverseMass(const RigidbodyComponent &rb)
{
    if (rb.mass <= std::numeric_limits<float>::min())
        return 0.0f;
    return 1.0f / rb.mass;
}
void CollisionStage::ResolveCollision(GameWorld &world, GameObject *a, GameObject *b, const Vector3f &normal, float penetration, const Vector3f &hitPoint)
{
    // normal:A->B为正

    auto &rbA = a->GetComponent<RigidbodyComponent>();
    auto &rbB = b->GetComponent<RigidbodyComponent>();
    // 冲量时无外力
    rbA.ClearForces();
    rbB.ClearForces();
    auto &tfA = a->GetComponent<TransformComponent>();
    auto &tfB = b->GetComponent<TransformComponent>();
    float invMassA = GetInverseMass(rbA);
    float invMassB = GetInverseMass(rbB);

    auto rA = hitPoint - tfA.GetWorldPosition();
    auto rB = hitPoint - tfB.GetWorldPosition();

    if (invMassA + invMassB <= std::numeric_limits<float>::min())
        return;
    Vector3f rV = rbB.velocity + (rbB.angularVelocity ^ rB) - rbA.velocity - (rbA.angularVelocity ^ rA);
    float nrV = rV * normal;
    if (nrV > 0.0f)
        return;

    // TODO 根据材料采取不同恢复系数表达式
    float e = rbA.elasticity * rbB.elasticity;
    float i = -(1.0f + e) * nrV;
    auto raxn = rA ^ normal;
    auto rbxn = rB ^ normal;

    Matrix3f rotA = tfA.GetWorldRotation().toMatrix();
    Matrix3f rotB = tfB.GetWorldRotation().toMatrix();

    auto worldInverseInertiaTensorA = rotA * rbA.inverseInertiaTensor * rotA.transposed();
    auto worldInverseInertiaTensorB = rotB * rbB.inverseInertiaTensor * rotB.transposed();

    float termA = raxn * (worldInverseInertiaTensorA * raxn);
    float termB = rbxn * (worldInverseInertiaTensorB * rbxn);

    float j = i / (invMassA + invMassB + termA + termB);

    Vector3f impulse = j * normal;
    rbA.AddImpulse(-impulse, rA);
    rbB.AddImpulse(impulse, rB);

    // TODO:精度
    const float percent = 0.6f;
    const float slop = 0.0001f;
    Vector3f correction = std::max(penetration - slop, 0.0f) * percent * normal / (invMassA + invMassB);

    Vector3f posA = tfA.GetWorldPosition() - invMassA * correction;
    Vector3f posB = tfB.GetWorldPosition() + invMassB * correction;

    Vector3f scaleA = tfA.GetWorldScale();
    Vector3f scaleB = tfB.GetWorldScale();

    Quat4f _rotA = tfA.GetWorldRotation();
    Quat4f _rotB = tfB.GetWorldRotation();

    // tfA.SetLocalPosition(tfA.GetLocalPosition());
    // tfB.SetLocalPosition(tfB.GetLocalPosition());
    tfA.SetWorldMatrix(Matrix4f::CreateTransform(posA, _rotA, scaleA));
    tfB.SetWorldMatrix(Matrix4f::CreateTransform(posB, _rotB, scaleB));

    world.GetEventManager().Emit(CollisionEvent(a, b, normal, penetration, hitPoint, rV, j));
}