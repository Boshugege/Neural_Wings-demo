#include "MainMenuScreen.h"
#include "raylib.h"
#include "Engine/System/Screen/ScreenManager.h"
#include "Game/Screen/MyScreenState.h"
MainMenuScreen::MainMenuScreen()
    : m_nextScreenState(SCREEN_STATE_NONE)
{
}
MainMenuScreen::~MainMenuScreen()
{
    OnExit();
}
// 进入时调用
void MainMenuScreen::OnEnter()
{
    if (screenManager && screenManager->GetUILayer())
    {
        screenManager->GetUILayer()->SetVisible(true);
        screenManager->GetUILayer()->LoadRoute(MAIN_MENU);
        screenManager->GetUILayer()->ExecuteScript(
            "window.vueAppState = window.vueAppState || {};"
            "window.vueAppState.nextScreen = '';");
    }
}
void MainMenuScreen::OnExit()
{
    if (screenManager && screenManager->GetUILayer())
    {
        screenManager->GetUILayer()->SetVisible(false);
    }
}

void MainMenuScreen::FixedUpdate(float fixedDeltaTime)
{
}

// 每帧更新逻辑
void MainMenuScreen::Update(float deltaTime)
{
    // 检查 Vue 路由是否已变化
    if (screenManager && screenManager->GetUILayer())
    {
        std::string nextScreen = screenManager->GetUILayer()->GetAppState("nextScreen");
        if (nextScreen == GAMEPLAY.getName())
        {
            screenManager->GetUILayer()->ExecuteScript("window.vueAppState.nextScreen = '';");
            m_nextScreenState = GAMEPLAY;
            return;
        }

        std::string currentRoute = screenManager->GetUILayer()->GetCurrentRoute();

        if (currentRoute == "#/" + GAMEPLAY.getName())
        {
            m_nextScreenState = GAMEPLAY;
            return;
        }
        else if (currentRoute == "#/" + OPTIONS.getName())
        {
            m_nextScreenState = OPTIONS;
            return;
        }
    }
}

// 每帧绘制
void MainMenuScreen::Draw()
{
    if (screenManager && screenManager->GetUILayer())
    {
        screenManager->GetUILayer()->Draw();
    }
}

// 向 ScreenManager 报告下一个状态
ScreenState MainMenuScreen::GetNextScreenState() const
{
    return m_nextScreenState;
}
ScreenState MainMenuScreen::GetScreenState() const
{
    return MAIN_MENU;
}
