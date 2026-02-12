#include "PhysicsSystem.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/TransformComponent.h"
#include "Engine/Core/Components/RigidBodyComponent.h"
#include "Engine/Math/Math.h"
#include <limits>
void PhysicsSystem::AddStage(std::unique_ptr<IPhysicsStage> stage)
{
    m_stages.push_back(std::move(stage));
}

void PhysicsSystem::ClearStages()
{
    m_stages.clear();
}

void PhysicsSystem::Update(GameWorld &world, float fixedDeltaTime)
{

    for (auto &stage : m_stages)
    {
        stage->Execute(world, fixedDeltaTime);
    }

    Integrate(world, fixedDeltaTime);
}

void PhysicsSystem::Integrate(GameWorld &world, float fixedDeltaTime)
{
    for (auto &object : world.GetGameObjects())
    {
        if (object->HasComponent<RigidbodyComponent>() && object->HasComponent<TransformComponent>())
        {
            auto &rb = object->GetComponent<RigidbodyComponent>();
            auto &tf = object->GetComponent<TransformComponent>();

            // 1. F = ma  =>  a = F / m
            // 如果质量为0，不移动
            if (std::abs(rb.mass) <= std::numeric_limits<float>::min())
                continue;

            Vector3f acceleration = rb.accumulatedForces / rb.mass;

            // 2. v = v + a * t
            rb.velocity += acceleration * fixedDeltaTime;

            // v = v * (1 - drag * t)
            float dragFactor = 1.0f - (rb.drag * fixedDeltaTime);
            if (dragFactor < 0)
                dragFactor = 0;
            rb.velocity *= dragFactor;

            // 4. p = p + v * t

            Vector3f pos = tf.GetWorldPosition();
            Quat4f rot = tf.GetWorldRotation();
            Vector3f scale = tf.GetWorldScale();
            pos += rb.velocity * fixedDeltaTime;
            // tf.SetLocalPosition(tf.GetLocalPosition() + rb.velocity * fixedDeltaTime);
            tf.SetWorldMatrix(Matrix4f::CreateTransform(pos, rot, scale));

            // angluar velocity
            Matrix3f rotationMatrix = rot.toMatrix();
            Matrix3f worldInverseInertia = rotationMatrix * rb.inverseInertiaTensor * rotationMatrix.transposed();

            rb.angularMomentum += rb.accumulatedTorques * fixedDeltaTime;

            float angularDragFactor = 1.0f - (rb.angularDrag * fixedDeltaTime);
            if (angularDragFactor < 0)
                angularDragFactor = 0;
            rb.angularMomentum *= angularDragFactor;

            rb.angularVelocity = worldInverseInertia * rb.angularMomentum;

            if (rb.angularVelocity.Length() > std::numeric_limits<float>::min())
            {
                // 角速度四元数 (0, ωx, ωy, ωz)
                Quat4f omegaQuat(0, rb.angularVelocity.x(), rb.angularVelocity.y(), rb.angularVelocity.z());

                // dq/dt = 0.5 * w * q
                // 世界系w左乘
                Quat4f dq = (omegaQuat * rot) * 0.5f;

                // 欧拉方法积分 q(t+dt) = q(t) + dq*dt
                rot = rot + dq * fixedDeltaTime;

                // 归一化
                rot.normalize();

                tf.SetWorldMatrix(Matrix4f::CreateTransform(pos, rot, scale));
            }
            // 5. 清理受力
            rb.ClearForces();
        }
    }
}