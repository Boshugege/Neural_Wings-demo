#include "StartScreen.h"
#include "MyScreenState.h"
#include "Engine/System/Screen/ScreenManager.h"

StartScreen::StartScreen(ScreenManager *sm)
    : m_nextScreenState(SCREEN_STATE_NONE),
      m_elapsedTime(0.0f),
      m_alpha(0.0f),
      IGameScreen(sm)
{
}
StartScreen::~StartScreen()
{
    OnExit();
}
void StartScreen::OnEnter()
{
    m_elapsedTime = 0.0f;
    m_alpha = 0.0f;
    m_nextScreenState = SCREEN_STATE_NONE;

    if (screenManager && screenManager->GetUILayer())
    {
        screenManager->GetUILayer()->SetVisible(true);
        screenManager->GetUILayer()->LoadRoute(SCREEN_STATE_START);
    }
}

void StartScreen::FixedUpdate(float fixedDeltaTime)
{
}

void StartScreen::Update(float deltaTime)
{
    m_elapsedTime += deltaTime;

    // --- 时间动画 ---
    if (m_elapsedTime < FADE_IN_TIME)
    {
        m_alpha = m_elapsedTime / FADE_IN_TIME;
    }
    else if (m_elapsedTime < DURATION - FADE_OUT_TIME)
    {
        m_alpha = 1.0f;
    }
    else if (m_elapsedTime < DURATION)
    {
        float timeInFadeOut = m_elapsedTime - (DURATION - FADE_OUT_TIME);
        m_alpha = 1.0f - (timeInFadeOut / FADE_OUT_TIME);
    }
    else
    {
        m_alpha = 0.0f;
        m_nextScreenState = MAIN_MENU;
    }

    if (m_alpha < 0.0f)
        m_alpha = 0.0f;
    if (m_alpha > 1.0f)
        m_alpha = 1.0f;
}

// 每帧绘制
void StartScreen::Draw()
{
    if (screenManager && screenManager->GetUILayer())
    {
        screenManager->GetUILayer()->Draw();
    }
}

void StartScreen::OnExit()
{
    if (screenManager && screenManager->GetUILayer())
    {
        screenManager->GetUILayer()->SetVisible(false);
    }
}

ScreenState StartScreen::GetNextScreenState() const
{
    return m_nextScreenState;
}
ScreenState StartScreen::GetScreenState() const
{
    return SCREEN_STATE_START;
}
