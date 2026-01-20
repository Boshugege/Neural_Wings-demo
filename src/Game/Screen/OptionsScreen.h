#pragma once
#include "Engine/System/Screen/GameScreen.h"
#include "ScreenState.h"

class OptionsScreen : public GameScreen
{
public:
    OptionsScreen();
    virtual ~OptionsScreen() = default;

    void OnEnter() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Update(float deltaTime) override;
    void Draw() override;
    void OnExit() override;
    int GetNextScreenState() const override;
    int GetScreenState() const override;

private:
    int m_nextScreenState;

    // Video settings
    int m_resolutionIndex;
    bool m_resolutionDropdownOpen;
    bool m_fullscreen;
    float m_targetFPS;
};
