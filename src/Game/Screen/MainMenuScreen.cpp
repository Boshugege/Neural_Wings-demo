#include "MainMenuScreen.h"
#include "raylib.h"
#include "Engine/System/Screen/ScreenManager.h"

MainMenuScreen::MainMenuScreen()
    : m_nextScreenState(static_cast<int>(ScreenStateID::NONE))
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
        screenManager->GetUILayer()->LoadRoute("menu");
    }
}
void MainMenuScreen::OnExit()
{
    if (screenManager && screenManager->GetUILayer())
    {
        screenManager->GetUILayer()->SetVisible(false);
    }
}

// 主菜单通常没有物理或网络逻辑，所以 FixedUpdate 为空
void MainMenuScreen::FixedUpdate(float fixedDeltaTime)
{
    //
}

// 每帧更新逻辑
void MainMenuScreen::Update(float deltaTime)
{
    // 检查 Vue 路由是否已变化
    if (screenManager && screenManager->GetUILayer())
    {
        std::string currentRoute = screenManager->GetUILayer()->GetCurrentRoute();

        if (currentRoute == "#/gameplay")
        {
            m_nextScreenState = static_cast<int>(ScreenStateID::GAMEPlAY);
            return;
        }
        else if (currentRoute == "#/options")
        {
            m_nextScreenState = static_cast<int>(ScreenStateID::OPTIONS);
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
int MainMenuScreen::GetNextScreenState() const
{
    return m_nextScreenState;
}
int MainMenuScreen::GetScreenState() const
{
    return static_cast<int>(ScreenStateID::MAIN_MENU);
}