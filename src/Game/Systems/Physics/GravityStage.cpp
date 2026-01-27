#include "GravityStage.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/RigidBodyComponent.h"
#include "Engine/Core/Components/TransformComponent.h"
#include <iostream>

GravityStage::GravityStage(Vector3f gravity) : m_gravity(gravity) {}

void GravityStage::Execute(GameWorld &world, float fixedDeltaTime)
{
    auto &gameObjects = world.GetGameObjects();
    if (gameObjects.empty())
    {
        std::cout << "[Gravity Stage]:Empty Game World" << std::endl;
        return;
    }
    for (auto &gameObject : gameObjects)
    {
        if (gameObject->HasComponent<RigidbodyComponent>())
        {
            auto &rigidBodyComponent = gameObject->GetComponent<RigidbodyComponent>();
            rigidBodyComponent.AddForce(m_gravity*rigidBodyComponent.mass);
            if (gameObject->HasComponent<TransformComponent>())
            {
                auto &transformComponent = gameObject->GetComponent<TransformComponent>();
                if (transformComponent.position.y() < 0.0f)
                {
                    transformComponent.position.y() = 0.0f;
                    rigidBodyComponent.velocity.y() = -rigidBodyComponent.velocity.y()*0.99;
                }
            }
        }
    }
}