#include "OptionsScreen.h"
#include "raylib.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
// #define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

#include <cstdio>
using json = nlohmann::json;

// Resolution options
static constexpr int RESOLUTION_WIDTHS[4] = {1280, 1600, 1920, 2560};
static constexpr int RESOLUTION_HEIGHTS[4] = {720, 900, 1080, 1440};
static constexpr const char *RESOLUTION_OPTIONS = "1280x720;1600x900;1920x1080;2560x1440";

// UI state
static int s_resolutionIndex = 0;
static bool s_dropdownEdit = false;
static float s_targetFPS = 60.0f;
#if !defined(PLATFORM_WEB)
static bool s_fullscreen = false;
#endif

// Loaded baseline
static int s_loadedResolutionIndex = 0;
static float s_loadedTargetFPS = 60.0f;
#if !defined(PLATFORM_WEB)
static bool s_loadedFullscreen = false;
#endif

OptionsScreen::OptionsScreen()
    : m_nextScreenState(static_cast<int>(ScreenStateID::NONE)){}
OptionsScreen::~OptionsScreen() {}

void OptionsScreen::OnEnter() { LoadConfig(); }
void OptionsScreen::OnExit() {}
void OptionsScreen::FixedUpdate(float) {}

void OptionsScreen::Update(float deltaTime)
{
    m_nextScreenState = static_cast<int>(ScreenStateID::NONE);

    if (IsKeyPressed(KEY_ESCAPE))
    {
        // Return to main menu without saving/applying
        m_nextScreenState = static_cast<int>(ScreenStateID::MAIN_MENU);
    }
}

void OptionsScreen::Draw()
{
    ClearBackground(BLACK);

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawText("OPTIONS", screenW / 2 - MeasureText("OPTIONS", 60) / 2, 100, 60, RAYWHITE);

    float centerX = screenW / 2.0f;
    float startY = 260.0f;
    float lineHeight = 50.0f;
    float labelWidth = 180.0f;
    float controlWidth = 250.0f;
    float leftX = centerX - (labelWidth + controlWidth + 20) / 2.0f;

    DrawText("VIDEO SETTINGS", screenW / 2 - MeasureText("VIDEO SETTINGS", 30) / 2, 200, 30, RAYWHITE);

#if !defined(PLATFORM_WEB)
    float resolutionY = startY;
    DrawText("Resolution:", leftX, startY + 8, 20, LIGHTGRAY);
    startY += lineHeight;
#else
    startY += lineHeight;
#endif

    if (s_dropdownEdit)
        GuiDisable();

#if !defined(PLATFORM_WEB)
    DrawText("Fullscreen:", leftX, startY + 8, 20, LIGHTGRAY);
    GuiCheckBox({leftX + labelWidth + 20, startY, 30, 30}, "", &s_fullscreen);
#else
    DrawText("Fullscreen: Press F11", leftX, startY + 8, 20, LIGHTGRAY);
#endif
    startY += lineHeight;

    DrawText("Target FPS:", leftX, startY + 8, 20, LIGHTGRAY);
    DrawText(TextFormat("%.0f", s_targetFPS), leftX + labelWidth + controlWidth + 40, startY + 8, 20, YELLOW);
    GuiSlider({leftX + labelWidth + 20, startY, controlWidth, 30}, "", "", &s_targetFPS, 30.0f, 240.0f);
    startY += lineHeight;

    bool dirty = s_resolutionIndex != s_loadedResolutionIndex || (int)s_targetFPS != (int)s_loadedTargetFPS
#if !defined(PLATFORM_WEB)
                 || s_fullscreen != s_loadedFullscreen
#endif
        ;
    if (GuiButton({centerX - 100.0f, startY + 20.0f, 200.0f, 36.0f}, "Save & Apply"))
    {
        SaveConfig();
        ApplyCurrentSelection();
        s_loadedResolutionIndex = s_resolutionIndex;
        s_loadedTargetFPS = s_targetFPS;
#if !defined(PLATFORM_WEB)
        s_loadedFullscreen = s_fullscreen;
#endif
    }

    if (s_dropdownEdit)
        GuiEnable();

#if !defined(PLATFORM_WEB)
    if (GuiDropdownBox({leftX + labelWidth + 20, resolutionY, controlWidth, 30}, RESOLUTION_OPTIONS, &s_resolutionIndex, s_dropdownEdit))
        s_dropdownEdit = !s_dropdownEdit;
#endif

    DrawText("Press ESC to return.", 10, screenH - 30, 20, DARKGRAY);
}

int OptionsScreen::GetNextScreenState() const
{
    return m_nextScreenState;
}

int OptionsScreen::GetScreenState() const
{
    return static_cast<int>(ScreenStateID::OPTIONS);
}

// --- Private helpers ---
void OptionsScreen::LoadConfig()
{
    int cfgWidth = GetScreenWidth();
    int cfgHeight = GetScreenHeight();
    int cfgTargetFPS = 60;
    bool cfgFullscreen = false;

    std::ifstream file("assets/config/engine_config.json");
    if (file.is_open())
    {
        json j = json::parse(file, nullptr, false);
        if (!j.is_discarded())
        {
#if !defined(PLATFORM_WEB)
            cfgWidth = j.value("/window/width"_json_pointer, cfgWidth);
            cfgHeight = j.value("/window/height"_json_pointer, cfgHeight);
            cfgFullscreen = j.value("/window/fullscreen"_json_pointer, cfgFullscreen);
#endif
            cfgTargetFPS = j.value("/performance/targetFPS"_json_pointer, cfgTargetFPS);
        }
    }

    s_targetFPS = static_cast<float>(cfgTargetFPS);
    s_resolutionIndex = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (RESOLUTION_WIDTHS[i] == cfgWidth && RESOLUTION_HEIGHTS[i] == cfgHeight)
        {
            s_resolutionIndex = i;
            break;
        }
    }

    s_loadedResolutionIndex = s_resolutionIndex;
    s_loadedTargetFPS = s_targetFPS;
#if !defined(PLATFORM_WEB)
    s_fullscreen = cfgFullscreen;
    s_loadedFullscreen = cfgFullscreen;
#endif
}

void OptionsScreen::SaveConfig() const
{
    json j;
    std::ifstream in("assets/config/engine_config.json");
    if (in.is_open())
    {
        j = json::parse(in, nullptr, false);
        if (j.is_discarded())
            j = json::object();
    }
    else
    {
        j = json::object();
    }

#if !defined(PLATFORM_WEB)
    j["window"]["width"] = RESOLUTION_WIDTHS[s_resolutionIndex];
    j["window"]["height"] = RESOLUTION_HEIGHTS[s_resolutionIndex];
    j["window"]["fullscreen"] = s_fullscreen;
#endif
    j["performance"]["targetFPS"] = static_cast<int>(s_targetFPS);

    std::ofstream out("assets/config/engine_config.json");
    if (out.is_open())
        out << j.dump(4);
}

void OptionsScreen::ApplyCurrentSelection() const
{
#if !defined(PLATFORM_WEB)
    SetWindowSize(RESOLUTION_WIDTHS[s_resolutionIndex], RESOLUTION_HEIGHTS[s_resolutionIndex]);
    if (s_fullscreen != IsWindowFullscreen())
        ToggleFullscreen();
#endif
    SetTargetFPS(static_cast<int>(s_targetFPS));
}
