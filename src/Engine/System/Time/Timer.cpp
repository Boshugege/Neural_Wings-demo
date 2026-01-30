#include "Timer.h"

Timer::Timer(float duration, Callback callback, bool repeat)
    : m_duration(duration),
      m_callback(callback),
      m_isRepeating(repeat),
      m_elapsedTime(0.0f),
      m_isPaused(false) {
}

void Timer::Tick(float deltaTime) {
    if (m_isPaused || IsFinished()) {
        return;
    }

    m_elapsedTime += deltaTime;

    if (m_elapsedTime >= m_duration) {
        ExecuteCallback();

        if (m_isRepeating) {
            Reset(true); 
        }
    }
}

bool Timer::IsFinished() const {
    return !m_isRepeating && m_elapsedTime >= m_duration;
}

bool Timer::IsActive() const {
    return !m_isPaused && !IsFinished();
}

void Timer::Pause() {
    m_isPaused = true;
}

void Timer::Resume() {
    m_isPaused = false;
}

void Timer::Reset(bool autoPlay) {
    m_elapsedTime = 0.0f;
    m_isPaused = !autoPlay;
}

void Timer::ExecuteCallback() {
    if (m_callback) {
        m_callback();
    }
}