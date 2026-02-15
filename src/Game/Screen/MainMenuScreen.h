#pragma once
#include "Engine/System/Screen/IGameScreen.h"
#include "MyScreenState.h"
class MainMenuScreen : public IGameScreen
{
public:
    MainMenuScreen(ScreenManager *sm);
    ~MainMenuScreen();

    void OnEnter() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Update(float deltaTime) override;
    void Draw() override;
    void OnExit() override;
    ScreenState GetNextScreenState() const override;
    ScreenState GetScreenState() const override;

private:
    ScreenState m_nextScreenState;
};