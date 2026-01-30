#pragma once

#include "Timer.h"
#include <vector>
#include <memory>
#include <functional>

class TimerManager
{
public:
    void Update(float deltaTime);
    Timer *AddTimer(float duration, std::function<void()> callback, bool repeat = false);

private:
    std::vector<std::unique_ptr<Timer>> m_timers;
};