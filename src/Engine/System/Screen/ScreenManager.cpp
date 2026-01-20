#include "ScreenManager.h"
#include "raylib.h"     

ScreenManager::ScreenManager(const EngineConfig& config, std::unique_ptr<ScreenFactory> factory) 
    : m_factory(std::move(factory))
{
    InitWindow(config.screenWidth, config.screenHeight, config.windowTitle.c_str());
    SetTargetFPS(config.targetFPS);
    m_timeManager = TimeManager(static_cast<float>(config.targetFPS));
    m_accumulator = 0.0f;
    SetExitKey(KEY_NULL);
    m_currentScreen = m_factory->Create(config.initialScreen);
    m_currentScreen->OnEnter();
   
}

ScreenManager::~ScreenManager() {
    Shutdown();
}

bool ScreenManager::UpdateFrame() {
    if (WindowShouldClose() || m_currentScreen->GetNextScreenState() == SCREEN_STATE_EXIT)
    {
        return false;
    }
    m_timeManager.Tick();
    m_accumulator += m_timeManager.GetDeltaTime();

    while (m_accumulator >= m_timeManager.GetFixedDeltaTime())
    {
        m_currentScreen->FixedUpdate(m_timeManager.GetFixedDeltaTime());
        m_accumulator -= m_timeManager.GetFixedDeltaTime();
    }

    m_currentScreen->Update(m_timeManager.GetDeltaTime());

    BeginDrawing();
    ClearBackground(BLACK);
    m_currentScreen->Draw();
    EndDrawing();

    int nextState = m_currentScreen->GetNextScreenState();
    if (nextState != SCREEN_STATE_NONE)
    {
        ChangeScreen(nextState);
    }
    return true;
}
void ScreenManager::Shutdown() {
    if (m_currentScreen) {
        m_currentScreen->OnExit();
    }
    CloseWindow();
}
// // 主循环
// void ScreenManager::Run() {
//     if (!m_currentScreen) {
//         return;
//     }

//     TimeManager timeManager;
//     float accumulator = 0.0f;

//     while (!WindowShouldClose() && m_currentScreen->GetNextScreenState() != SCREEN_STATE_EXIT) {
        
//         // --- 1. 更新时间 ---
//         timeManager.Tick();
//         accumulator += timeManager.GetDeltaTime();

//         // --- 2. 固定更新循环 (FixedUpdate) ---
//         // 这个循环确保无论渲染帧率如何波动，物理和逻辑都以固定的频率执行
//         while (accumulator >= timeManager.GetFixedDeltaTime()) {
//             m_currentScreen->FixedUpdate(timeManager.GetFixedDeltaTime());
//             accumulator -= timeManager.GetFixedDeltaTime();
//         }

//         // --- 3. 可变更新 (Update) ---
//         m_currentScreen->Update(timeManager.GetDeltaTime());

//         // --- 4. 绘制 (Draw) ---
//         BeginDrawing();
//         ClearBackground(BLACK); // 默认黑色背景
//         m_currentScreen->Draw();
//         EndDrawing();

//         // --- 5. 在帧末尾检查是否需要切换屏幕 ---
//         int nextState = m_currentScreen->GetNextScreenState();
//         if (nextState != SCREEN_STATE_NONE) {
//             ChangeScreen(nextState);
//         }
//     }
// }

// 屏幕切换
void ScreenManager::ChangeScreen(int newState) {
    if (newState == SCREEN_STATE_EXIT || newState == SCREEN_STATE_NONE) {
        return;
    }
    // 1. 删除当前屏幕，释放资源
    if (m_currentScreen) {
        m_currentScreen.reset();
    }
    // 2. 使用工厂创建一个新的屏幕实例
    m_currentScreen = m_factory->Create(newState);

    // 3. 如果新屏幕创建成功，则通知它进入
    if (m_currentScreen) {
        m_currentScreen->OnEnter();
    } else {
        m_currentScreen = m_factory->Create(SCREEN_STATE_ERROR);
        if(m_currentScreen) m_currentScreen->OnEnter();
    }
}