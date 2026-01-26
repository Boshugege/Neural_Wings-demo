#include "SolarStage.h"
#include "Engine/Engine.h"
#include <iostream>
SolarStage::SolarStage(float G) : m_G(G) {}

void SolarStage::Execute(GameWorld &world, float fixedDeltaTime)
{
    auto &gameObjects = world.GetGameObjects();
    if (gameObjects.empty())
    {
        std::cout << "[SloarStage]:Empty Game World" << std::endl;
        return;
    }
    for (auto &gameObject : gameObjects)
    {
        if (gameObject->HasComponent<RigidbodyComponent>() && gameObject->HasComponent<TransformComponent>())
        {
            auto &rb = gameObject->GetComponent<RigidbodyComponent>();
            auto &transform = gameObject->GetComponent<TransformComponent>();
            for (auto &otherGameObject : gameObjects)
            {
                if (gameObject != otherGameObject)
                {std::cout<<"gameObject: "<<gameObject->GetID()<<" otherGameObject: "<<otherGameObject->GetID()<<std::endl;
              
                    auto &otherRb = otherGameObject->GetComponent<RigidbodyComponent>();
                    auto &otherTransform = otherGameObject->GetComponent<TransformComponent>();

                    auto distance =Vector3f::Distance(transform.position,otherTransform.position);
                    auto f = m_G * rb.mass * otherRb.mass / (distance * distance);
                    auto F = (otherTransform.position - transform.position).Normalized() * f;
                    std::cout<<"F: "<<F.x()<<" "<<F.y()<<" "<<F.z()<<std::endl;
                    rb.AddForce(F);
                }
            }
        }
    }
}