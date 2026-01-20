#pragma once
#include "Engine/System/Screen/GameScreen.h"
#include "ScreenState.h"
class MainMenuScreen : public GameScreen {
public:
    MainMenuScreen();
    ~MainMenuScreen(); 


    void OnEnter() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Update(float deltaTime) override;
    void Draw() override;
    void OnExit() override;
    int GetNextScreenState() const override;
    int GetScreenState() const override;
private:
    int m_nextScreenState;
};