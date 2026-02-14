#include "IGameScreen.h"
#include "ScreenManager.h"

IGameScreen::IGameScreen(ScreenManager *sm)
    : screenManager(sm), resourceManager(&sm->GetResourceManager()) {}
