#pragma once
#include "IPhysicsStage.h"
#include <vector>
#include <memory>

class PhysicsSystem {
public:
    void Update(GameWorld& world, float fixedDeltaTime);
    void AddStage(std::unique_ptr<IPhysicsStage> stage);
    void ClearStages();

private:
    // 不同物理规则
    std::vector<std::unique_ptr<IPhysicsStage>> m_stages;
    
    // 半euler积分
    void Integrate(GameWorld& world, float fixedDeltaTime);
};