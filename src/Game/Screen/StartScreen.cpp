#include "StartScreen.h"
#include "ScreenState.h" 
#include "raylib.h"

StartScreen::StartScreen() 
    : m_nextScreenState(SCREEN_STATE_NONE),
      m_elapsedTime(0.0f),
      m_alpha(0.0f)
{
}
StartScreen::~StartScreen() {
    OnExit();
}
void StartScreen::OnEnter() {
    m_elapsedTime = 0.0f;
    m_alpha = 0.0f;
    m_nextScreenState = SCREEN_STATE_NONE;
}

void StartScreen::FixedUpdate(float fixedDeltaTime) {
}

void StartScreen::Update(float deltaTime) {
    m_elapsedTime += deltaTime;

    // --- 动画 ---
    if (m_elapsedTime < FADE_IN_TIME) {
        m_alpha = m_elapsedTime / FADE_IN_TIME;
    } 
    else if (m_elapsedTime < DURATION - FADE_OUT_TIME) {
        m_alpha = 1.0f;
    } 
    else if (m_elapsedTime < DURATION) {
        float timeInFadeOut = m_elapsedTime - (DURATION - FADE_OUT_TIME);
        m_alpha = 1.0f - (timeInFadeOut / FADE_OUT_TIME);
    }
    else {
        m_alpha = 0.0f;
        m_nextScreenState = static_cast<int>(ScreenStateID::MAIN_MENU);
    }

    if (m_alpha < 0.0f) m_alpha = 0.0f;
    if (m_alpha > 1.0f) m_alpha = 1.0f;
}

// 每帧绘制
void StartScreen::Draw() {
    ClearBackground(BLACK);

    const char* logoText = "Neural Wings";
    int fontSize = 80;
    int textWidth = MeasureText(logoText, fontSize);
    
    DrawText(
        logoText, 
        GetScreenWidth() / 2 - textWidth / 2, 
        GetScreenHeight() / 2 - fontSize / 2, 
        fontSize, 
        Fade(RAYWHITE, m_alpha) 
    );
}

void StartScreen::OnExit() {
}

int StartScreen::GetNextScreenState() const {
    return m_nextScreenState;
}
int StartScreen::GetScreenState() const {
    return static_cast<int>(ScreenStateID::START);
}