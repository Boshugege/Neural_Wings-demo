#include "TestStage.h"
#include "Engine/Engine.h"
#include <iostream>
TestStage::TestStage() {}
void TestStage::Execute(GameWorld &world, float fixedDeltaTime)
{
    auto &gameObjects = world.GetGameObjects();
    if (gameObjects.empty())
    {
        std::cout << "[TestStage]:Empty Game World" << std::endl;
        return;
    }
    for (auto &gameObject : gameObjects)
    {
        if (gameObject->HasComponent<RigidbodyComponent>() && gameObject->HasComponent<TransformComponent>())
        {
            auto &rb = gameObject->GetComponent<RigidbodyComponent>();
            auto &transform = gameObject->GetComponent<TransformComponent>();
            auto torque = Vector3f(0.0, 0.0, 0.0);
            rb.AddTorque(torque);
        }
    }
}