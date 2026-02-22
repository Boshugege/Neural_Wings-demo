#pragma once
#include "IGameHud.h"
#include <functional>
#include <memory>
#include <unordered_map>

class HudFactory
{
public:
    using HudCreator = std::function<std::unique_ptr<IGameHud>()>;

    void Register(int state, HudCreator creator);
    std::unique_ptr<IGameHud> Create(int state) const;

private:
    std::unordered_map<int, HudCreator> m_creators;
};
