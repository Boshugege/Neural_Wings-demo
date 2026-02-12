#ifdef _MSC_VER
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#pragma warning(disable : 4305)
#endif

#include "SolarStage.h"
#include "Engine/Engine.h"
#include <iostream>
void SolarStage::Initialize(const json &config)
{
    m_G = config.value("G", 0.1);
}
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
                {
                    auto &otherRb = otherGameObject->GetComponent<RigidbodyComponent>();
                    auto &otherTransform = otherGameObject->GetComponent<TransformComponent>();

                    auto distance = Vector3f::Distance(transform.GetWorldPosition(), otherTransform.GetWorldPosition());

                    auto f = m_G * rb.mass * otherRb.mass / (distance * distance);
                    auto F = (otherTransform.GetWorldPosition() - transform.GetWorldPosition()).Normalized() * f;
                    rb.AddForce(F);
                }
            }
        }
    }
}