#include "Engine/Engine.h"

#include "Game/Screen.h"


#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static std::unique_ptr<ScreenManager> g_App = nullptr;
void UpdateDrawFrame() {
    g_App->UpdateFrame();
}
int main() {
    EngineConfig config;
    if(!config.load("assets/config/engine_config.json"))
    {
        printf("EngineConfig failed.\n");
        return -1;
    }

    auto factory = std::make_unique<ScreenFactory>();
    factory->Register((static_cast<int>(ScreenStateID::START)), []() { return std::make_unique<StartScreen>(); });
    factory->Register(static_cast<int>(ScreenStateID::MAIN_MENU), []() { return std::make_unique<MainMenuScreen>(); });
    factory->Register(static_cast<int>(ScreenStateID::GAMEPlAY), []() { return std::make_unique<GameplayScreen>(); });
    factory->Register(static_cast<int>(ScreenStateID::OPTIONS), []() { return std::make_unique<OptionsScreen>(); });
    
    g_App = std::make_unique<ScreenManager>(config, std::move(factory));


#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (g_App->UpdateFrame());
#endif

    //g_App->Shutdown();
    

    return 0;
}