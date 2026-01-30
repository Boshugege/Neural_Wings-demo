// 事件系统通过脚本挂载到GameObject
#pragma once
#include "Engine/Core/Components/Components.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/System/Physics/Physics.h"
#include <iostream>
class CollisionListener : public IScriptableComponent
{
public:
    void OnCreate() override
    {
        auto &em = owner->GetOwnerWorld()->GetEventManager();
        m_subID = em.Subscribe<CollisionEvent>([this](const CollisionEvent &e)
                                               { this->OnCollision(e); });
    }
    void OnCollision(const CollisionEvent &e)
    {
        std::cout << "[CollisionListener]: I HEARD A COLLISION!!!" << std::endl;
    }
    void OnDestroy() override
    {
        std::cout << "[CollisionListener]: I'm being destroyed!" << std::endl;
        if (m_subID != 0)
        {
            // 绑定对象销毁后，取消订阅
            owner->GetOwnerWorld()->GetEventManager().Unsubscribe(m_subID);
            m_subID = 0;
        }
    }

private:
    Subscription_ID m_subID = 0;
};