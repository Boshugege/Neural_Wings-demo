#include "MainMenuScreen.h"
#include "raylib.h"
// 确保这个定义只在整个项目的一个 .cpp 文件中出现
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"


MainMenuScreen::MainMenuScreen() 
    : m_nextScreenState(static_cast<int>(ScreenStateID::NONE))
{
}

// 进入时调用
void MainMenuScreen::OnEnter() {
    // GuiLoadStyle("path/to/my_style.rgs");
}
void MainMenuScreen::OnExit() {
    
}

// 主菜单通常没有物理或网络逻辑，所以 FixedUpdate 为空
void MainMenuScreen::FixedUpdate(float fixedDeltaTime) {
    // 
}

// 每帧更新逻辑
void MainMenuScreen::Update(float deltaTime) {
    // 在每一帧的开始，重置状态请求。
    // 只有当用户在本帧点击按钮时，它才会被改变。
    m_nextScreenState = static_cast<int>(ScreenStateID::NONE);
}

// 每帧绘制
void MainMenuScreen::Draw() {
    // 1. 背景
    ClearBackground(BLACK);

    // 2. 标题
    const char* title = "NEURAL WINGS";
    int titleFontSize = 60;
    int titleWidth = MeasureText(title, titleFontSize);
    DrawText(title, GetScreenWidth() / 2 - titleWidth / 2, 150, titleFontSize, RAYWHITE);
    
    // 3. 按钮
    float buttonWidth = 220;
    float buttonHeight = 50;
    float centerX = GetScreenWidth() / 2.0f;
    float spacing = 15.0f; 

    // 4. 点击事件
    if (GuiButton({ centerX - buttonWidth / 2, 300, buttonWidth, buttonHeight }, "Play Game")) {
        m_nextScreenState = static_cast<int>(ScreenStateID::GAMEPlAY);
    }
    
    if (GuiButton({ centerX - buttonWidth / 2, 300 + buttonHeight + spacing, buttonWidth, buttonHeight }, "Options")) {
        m_nextScreenState = static_cast<int>(ScreenStateID::OPTIONS);
    }

    if (GuiButton({ centerX - buttonWidth / 2, 300 + (buttonHeight + spacing) * 2, buttonWidth, buttonHeight }, "Exit")) {
        m_nextScreenState =static_cast<int>(ScreenStateID::EXIT);
    }
}

// 向 ScreenManager 报告下一个状态
int MainMenuScreen::GetNextScreenState() const {
    return m_nextScreenState;
}
int MainMenuScreen::GetScreenState() const {
    return static_cast<int>(ScreenStateID::MAIN_MENU);
}