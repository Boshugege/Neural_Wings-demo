#pragma once
#include "Config.h"
#include "Engine/System/Screen/GameScreen.h"
#include <string>

struct EngineConfig : public Config
{
    int screenWidth = 1280;
    int screenHeight = 720;
    std::string windowTitle = "Default Engine Window";

    float targetFPS = 60.0f;

    int initialScreen = SCREEN_STATE_NONE;

    bool fullScreen = false;

    void toJson(json &j) const
    {
        j = json{
            {"window", {{"width", screenWidth}, {"height", screenHeight}, {"title", windowTitle}, {"fullscreen", fullScreen}}},
            {"performance", {{"targetFPS", targetFPS}}},
        };
    }

protected:
    void ParseJson(const json &configJson) override
    {
        this->screenWidth = configJson.at("window").value("width", this->screenWidth);
        this->screenHeight = configJson.at("window").value("height", this->screenHeight);
        this->windowTitle = configJson.at("window").value("title", this->windowTitle);
        this->targetFPS = configJson.at("performance").value("targetFPS", this->targetFPS);
        if (configJson.contains("window"))
        {
            const auto &windowJson = configJson.at("window");
            this->fullScreen = windowJson.value("fullscreen", this->fullScreen);
        }
        this->initialScreen = SCREEN_STATE_START;
    }
};
