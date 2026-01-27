#include "PhysicsSystem.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/TransformComponent.h"
#include "Engine/Core/Components/RigidBodyComponent.h"
#include "Engine/Math/Math.h"

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
            if (std::abs(rb.mass) <= 0.0001f)
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
            tf.position += rb.velocity * fixedDeltaTime;

            // angluar velocity
            Matrix3f rotationMatrix = tf.rotation.toMatrix();
            Matrix3f worldInverseInertia = rotationMatrix * rb.inverseInertiaTensor * rotationMatrix.transposed();
            // Matrix3f worldInertia = rotationMatrix * rb.inertiaTensor * rotationMatrix.transposed();

            rb.angularMomentum += rb.accumulatedTorques * fixedDeltaTime;

            float angularDragFactor = 1.0f - (rb.angularDrag * fixedDeltaTime);
            if (angularDragFactor < 0)
                angularDragFactor = 0;
            rb.angularMomentum *= angularDragFactor;

            rb.angularVelocity = worldInverseInertia * rb.angularMomentum;
          
            if (rb.angularVelocity.Length() > 0.0001f)
            {
                // 角速度四元数 (0, ωx, ωy, ωz)
                Quat4f omegaQuat(0, rb.angularVelocity.x(), rb.angularVelocity.y(), rb.angularVelocity.z());

                // dq/dt = 0.5 * w * q
                // 世界系w左乘
                Quat4f dq = (omegaQuat*tf.rotation) * 0.5f;

                // 欧拉方法积分 q(t+dt) = q(t) + dq*dt
                tf.rotation = tf.rotation + dq * fixedDeltaTime;

                // 归一化
                tf.rotation.normalize();
            }
            // 5. 清理受力
            rb.ClearForces();
        }
    }
}