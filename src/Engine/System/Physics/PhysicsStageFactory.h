#pragma once
#include "IPhysicsStage.h"
#include <functional>
#include <map>
#include <memory>

class PhysicsStageFactory
{
public:
    using PhysicsStageCreator = std::function<std::unique_ptr<IPhysicsStage>()>;

    void Register(const std::string &name, PhysicsStageCreator creator)
    {
        m_creator[name] = creator;
    }
    std::unique_ptr<IPhysicsStage> Create(const std::string &name)
    {
        auto it = m_creator.find(name);
        if (it != m_creator.end())
            return it->second();
        return nullptr;
    }

private:
    std::unordered_map<std::string, PhysicsStageCreator> m_creator;
};