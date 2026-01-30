#pragma once
#include <functional>

class Timer {
public:
    using Callback = std::function<void()>;

    Timer(float duration, Callback callback, bool repeat = false);

    void Tick(float deltaTime);

    bool IsFinished() const;
    bool IsActive() const;

    void Pause();
    void Resume();
    void Reset(bool autoPlay = true);

private:
    void ExecuteCallback();

    float m_duration;
    float m_elapsedTime;
    Callback m_callback;
    bool m_isRepeating;
    bool m_isPaused;
};