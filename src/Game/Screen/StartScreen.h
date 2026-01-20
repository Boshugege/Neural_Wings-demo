#pragma once
#include "Engine/System/Screen/GameScreen.h"
#include "ScreenState.h"
class StartScreen : public GameScreen {
public:
    StartScreen();
    ~StartScreen();

    void OnEnter() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Update(float deltaTime) override;
    void Draw() override;
    void OnExit() override;
    int GetNextScreenState() const override;
    int GetScreenState() const override;
private:
    int m_nextScreenState;

    float m_elapsedTime;   
    float m_alpha;         
    
    const float DURATION = 3.0f; 
    const float FADE_IN_TIME = 1.0f; 
    const float FADE_OUT_TIME = 1.0f; 

};