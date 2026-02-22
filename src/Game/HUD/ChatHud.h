#pragma once
#include "Engine/System/HUD/IGameHud.h"
#include <string>

class ScreenManager;
class InputManager;

class ChatHud : public IGameHud
{
public:
    ChatHud(ScreenManager *screenManager, InputManager *inputManager);
    ~ChatHud() override = default;

    void OnEnter() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Update(float deltaTime) override;
    void Draw() override;
    void OnExit() override;

    bool BlocksGameplayInput() const override { return m_chatActive; }
    bool WantsCursorVisible() const override { return m_chatActive; }
    bool ConsumeExitSuppressRequest() override;

private:
    void ActivateChat();
    void DeactivateChat();
    void PollChatUI();
    static std::string EscapeForJsStringLiteral(const std::string &text);

    ScreenManager *m_screenManager = nullptr;
    InputManager *m_inputManager = nullptr;

    bool m_chatActive = false;
    int m_framesInHud = 0;
    bool m_skipExitThisFrame = false;
};
