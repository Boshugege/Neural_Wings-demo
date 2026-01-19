#include "OptionsScreen.h"
#include "raylib.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
// #define RAYGUI_IMPLEMENTATION
#include "raygui.h"

OptionsScreen::OptionsScreen()
    : m_nextScreenState(static_cast<int>(ScreenStateID::NONE)),
      m_resolutionIndex(1),
      m_resolutionDropdownOpen(false),
      m_fullscreen(false),
      m_targetFPS(144.0f) {}
void OptionsScreen::OnEnter() {}
void OptionsScreen::OnExit() {}
void OptionsScreen::FixedUpdate(float fixedDeltaTime) {}

void OptionsScreen::Update(float deltaTime)
{
    m_nextScreenState = static_cast<int>(ScreenStateID::NONE);

    if (IsKeyPressed(KEY_ESCAPE))
    {
        m_nextScreenState = static_cast<int>(ScreenStateID::MAIN_MENU);
    }

    // 检测全屏状态变化并应用
    bool currentFullscreen = IsWindowFullscreen();
    if (m_fullscreen != currentFullscreen)
    {
#if defined(PLATFORM_WEB)
        // Web平台：使用Emscripten的全屏API
        // 注意：Web端全屏需要用户手势触发，这里通过checkbox点击触发
        if (m_fullscreen)
        {
            // 请求全屏（已在Draw中通过GuiCheckBox的用户交互触发）
        }
#else
        // Desktop平台：使用Raylib的ToggleFullscreen
        ToggleFullscreen();
#endif
    }
}

void OptionsScreen::Draw()
{
    ClearBackground(BLACK);

    const char *title = "OPTIONS";
    int titleFontSize = 60;
    int titleWidth = MeasureText(title, titleFontSize);
    DrawText(title, GetScreenWidth() / 2 - titleWidth / 2, 100, titleFontSize, RAYWHITE);

    // Video Settings Section
    float centerX = GetScreenWidth() / 2.0f;
    float startY = 200.0f;
    float lineHeight = 50.0f;
    float labelWidth = 180.0f;
    float controlWidth = 250.0f;
    float totalWidth = labelWidth + controlWidth + 20;
    float leftX = centerX - totalWidth / 2.0f;

    // Section Title
    const char *sectionTitle = "VIDEO SETTINGS";
    int sectionTitleWidth = MeasureText(sectionTitle, 30);
    DrawText(sectionTitle, centerX - sectionTitleWidth / 2.0f, startY, 30, RAYWHITE);
    startY += 60.0f;

#if !defined(PLATFORM_WEB)
    DrawText("Resolution:", leftX, startY + 8, 20, LIGHTGRAY);
    const char *resolutions[] = {"1280x720", "1600x900", "1920x1080", "2560x1440"};
    static const int resWidths[] = {1280, 1600, 1920, 2560};
    static const int resHeights[] = {720, 900, 1080, 1440};

    Rectangle ddRect = {leftX + labelWidth + 20, startY, controlWidth, 30};
    const char *curText = resolutions[m_resolutionIndex];
    if (GuiButton(ddRect, curText))
    {
        m_resolutionDropdownOpen = !m_resolutionDropdownOpen;
    }

    // Draw popup options when open
    if (m_resolutionDropdownOpen)
    {
        bool selectionMade = false;
        for (int i = 0; i < 4; ++i)
        {
            Rectangle optRect = {ddRect.x, ddRect.y + (i + 1) * (ddRect.height + 4), ddRect.width, ddRect.height};
            if (GuiButton(optRect, resolutions[i]))
            {
                m_resolutionIndex = i;
                int w = resWidths[m_resolutionIndex];
                int h = resHeights[m_resolutionIndex];
                SetWindowSize(w, h);
                m_resolutionDropdownOpen = false;
                selectionMade = true;
            }
        }
        if (!selectionMade && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Vector2 mp = GetMousePosition();
            bool clickedInside = (mp.x >= ddRect.x && mp.x <= ddRect.x + ddRect.width && mp.y >= ddRect.y && mp.y <= ddRect.y + ddRect.height + 4 * (ddRect.height + 4));
            if (!clickedInside)
                m_resolutionDropdownOpen = false;
        }
    }

    startY += lineHeight + (m_resolutionDropdownOpen ? (4 * (ddRect.height + 4)) : 0);
#else
    // Web: skip resolution control
    startY += lineHeight;
#endif

    // Fullscreen
    DrawText("Fullscreen:", leftX, startY + 8, 20, LIGHTGRAY);
    bool prevFullscreen = m_fullscreen;
    GuiCheckBox({leftX + labelWidth + 20, startY, 30, 30}, "", &m_fullscreen);
    if (prevFullscreen != m_fullscreen)
    {
#if defined(PLATFORM_WEB)
        // Web: 使用浏览器 Fullscreen API via Emscripten
        if (m_fullscreen)
        {
            emscripten_run_script("if (!document.fullscreenElement) { document.documentElement.requestFullscreen().catch(function(e){ console.warn('requestFullscreen failed', e); }); }");
        }
        else
        {
            emscripten_run_script("if (document.fullscreenElement) { document.exitFullscreen().catch(function(e){ console.warn('exitFullscreen failed', e); }); }");
        }
        // 不要立即覆盖用户的 m_fullscreen（浏览器的 fullscreenchange 是异步的），保留用户的选择
#else
        // Desktop平台直接切换并同步实际窗口状态
        ToggleFullscreen();
        m_fullscreen = IsWindowFullscreen();
#endif
    }
    startY += lineHeight;

    // Target FPS
    DrawText("Target FPS:", leftX, startY + 8, 20, LIGHTGRAY);
    DrawText(TextFormat("%.0f", m_targetFPS), leftX + labelWidth + controlWidth + 40, startY + 8, 20, YELLOW);
    float prevFPS = m_targetFPS;
    GuiSlider({leftX + labelWidth + 20, startY, controlWidth, 30}, "", "", &m_targetFPS, 30.0f, 240.0f);
    if ((int)prevFPS != (int)m_targetFPS)
    {
        SetTargetFPS(static_cast<int>(m_targetFPS));
    }
    startY += lineHeight;

    DrawText("Press ESC to return.", 10, GetScreenHeight() - 30, 20, DARKGRAY);
}

int OptionsScreen::GetNextScreenState() const
{
    return m_nextScreenState;
}

int OptionsScreen::GetScreenState() const
{
    return static_cast<int>(ScreenStateID::OPTIONS);
}
