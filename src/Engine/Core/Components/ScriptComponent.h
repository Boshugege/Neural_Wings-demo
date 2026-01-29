#pragma once
#include "IComponent.h"
#include "IScriptableComponent.h"
#include <memory>
#include <vector>

struct ScriptComponent : public IComponent
{
    std::vector<std::unique_ptr<IScriptableComponent>> scripts;
    ~ScriptComponent() override
    {
        for (auto &script : scripts)
            script->OnDestroy();
    }
};