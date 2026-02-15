#pragma once
#include "Engine/System/Screen/IGameScreen.h"
#include "MyScreenState.h"
class StartScreen : public IGameScreen
{
public:
    StartScreen(ScreenManager *sm);
    ~StartScreen();

    void OnEnter() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Update(float deltaTime) override;
    void Draw() override;
    void OnExit() override;
    ScreenState GetNextScreenState() const override;
    ScreenState GetScreenState() const override;

private:
    ScreenState m_nextScreenState;

    float m_elapsedTime;
    float m_alpha;

    const float DURATION = 3.0f;
    const float FADE_IN_TIME = 1.0f;
    const float FADE_OUT_TIME = 1.0f;
};