#pragma once
#include "Engine/Core/Components/IScriptableComponent.h"
#include <functional>
#include <unordered_map>
#include <string>
#include <memory>

class ScriptingFactory
{
public:
    using ScriptCreator = std::function<std::unique_ptr<IScriptableComponent>()>;
    void Register(const std::string &name, ScriptCreator creator)
    {
        m_creators[name] = creator;
    }
    std::unique_ptr<IScriptableComponent> Create(const std::string &name) const
    {
        auto it = m_creators.find(name);
        if (it != m_creators.end())
            return it->second();
        return nullptr;
    }

private:
    std::unordered_map<std::string, ScriptCreator> m_creators;
};