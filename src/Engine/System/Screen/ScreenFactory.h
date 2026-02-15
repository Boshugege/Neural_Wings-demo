#pragma once
#include "IGameScreen.h"
#include <memory>
#include <functional>
#include <unordered_map>

class ScreenFactory
{
public:
    using ScreenCreator = std::function<std::unique_ptr<IGameScreen>(ScreenManager *)>;

    void Register(int state, ScreenCreator creator);

    std::unique_ptr<IGameScreen> Create(int state, ScreenManager *manager);

private:
    std::unordered_map<int, ScreenCreator> m_creators;
};