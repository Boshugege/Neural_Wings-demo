#include "PhysicsSystem.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/TransformComponent.h"
#include "Engine/Core/Components/RigidbodyComponent.h"
#include "raymath.h"

void PhysicsSystem::AddStage(std::unique_ptr<IPhysicsStage> stage) {
    m_stages.push_back(std::move(stage));
}

void PhysicsSystem::ClearStages() {
    m_stages.clear();
}

void PhysicsSystem::Update(GameWorld& world, float fixedDeltaTime) {
    
    for (auto& stage : m_stages) {
        stage->Execute(world, fixedDeltaTime);
    }

    Integrate(world, fixedDeltaTime);
}

void PhysicsSystem::Integrate(GameWorld& world, float fixedDeltaTime) {
    for (auto& object : world.GetGameObjects()) {
        if (object->HasComponent<RigidbodyComponent>() && object->HasComponent<TransformComponent>()) {
            auto& rb = object->GetComponent<RigidbodyComponent>();
            auto& tf = object->GetComponent<TransformComponent>();

            // 1. F = ma  =>  a = F / m
            // 如果质量为0，不移动
            if (std::abs(rb.mass) <= 0.0001f) continue; 

            Vector3 acceleration = Vector3Scale(rb.accumulatedForces, 1.0f / rb.mass);

            // 2. v = v + a * t
            rb.velocity = Vector3Add(rb.velocity, Vector3Scale(acceleration, fixedDeltaTime));

            // v = v * (1 - drag * t)
            float dragFactor = 1.0f - (rb.drag * fixedDeltaTime);
            if (dragFactor < 0) dragFactor = 0;
            rb.velocity = Vector3Scale(rb.velocity, dragFactor);

            // 4. p = p + v * t
            tf.position = Vector3Add(tf.position, Vector3Scale(rb.velocity, fixedDeltaTime));

            // 5. 清理受力
            rb.ClearForces();
        }
    }
}