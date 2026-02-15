#pragma once
#include "Config.h"
#include "Engine/System/Screen/IGameScreen.h"
#include <string>
#include "Engine/System/Screen/ScreenState.h"
#include "Engine/Network/NetTypes.h"

struct EngineConfig : public Config
{
    int screenWidth = 1280;
    int screenHeight = 720;
    std::string windowTitle = "Default Engine Window";

    float targetFPS = 60.0f;

    int initialScreen = SCREEN_STATE_NONE;

    bool fullScreen = false;

    std::string serverIP = DEFAULT_SERVER_HOST;
    uint16_t serverPort = DEFAULT_SERVER_PORT;

    void toJson(json &j) const
    {
        j = json{
            {"window", {{"width", screenWidth}, {"height", screenHeight}, {"title", windowTitle}, {"fullscreen", fullScreen}}},
            {"performance", {{"targetFPS", targetFPS}}},
            {"network", {{"serverIP", serverIP}, {"serverPort", serverPort}}},
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
        if (configJson.contains("network"))
        {
            const auto &networkJson = configJson.at("network");
            this->serverIP = networkJson.value("serverIP", this->serverIP);
            this->serverPort = networkJson.value("serverPort", this->serverPort);
        }
        this->initialScreen = SCREEN_STATE_START;
    }
};
