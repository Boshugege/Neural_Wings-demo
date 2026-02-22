#include "ChatHud.h"
#include "raylib.h"

#include "Engine/System/Screen/ScreenManager.h"
#include "Engine/System/HUD/HudBridgeScript.h"
#include "Engine/UI/UILayer.h"
#include "Engine/System/Input/InputManager.h"

ChatHud::ChatHud(ScreenManager *screenManager, InputManager *inputManager)
    : m_screenManager(screenManager), m_inputManager(inputManager)
{
}

void ChatHud::OnEnter()
{
    m_chatActive = false;
    m_skipExitThisFrame = false;
    m_framesInHud = 0;

    if (!m_screenManager)
        return;

    auto *ui = m_screenManager->GetUILayer();
    if (!ui)
        return;

    // Gameplay HUD overlay entrypoint.
    ui->ExecuteScript("window.location.hash = '#/gameplay';");
    ui->SetVisible(true);
    ui->ExecuteScript(HudBridgeScript::ResetChatState());
}

void ChatHud::FixedUpdate(float fixedDeltaTime)
{
    (void)fixedDeltaTime;
}

void ChatHud::Update(float deltaTime)
{
    (void)deltaTime;
    ++m_framesInHud;

    if (m_chatActive)
    {
        PollChatUI();
    }
    else if (m_framesInHud > 2 && IsKeyPressed(KEY_ENTER))
    {
        ActivateChat();
    }
}

void ChatHud::Draw()
{
}

void ChatHud::OnExit()
{
    if (m_chatActive)
    {
        DeactivateChat();
    }
}

bool ChatHud::ConsumeExitSuppressRequest()
{
    if (!m_skipExitThisFrame)
        return false;
    m_skipExitThisFrame = false;
    return true;
}

void ChatHud::ActivateChat()
{
    if (m_chatActive)
        return;

    m_chatActive = true;
    if (m_inputManager)
    {
        m_inputManager->SetEnabled(false);
    }
    EnableCursor();

    if (!m_screenManager)
        return;

    auto *ui = m_screenManager->GetUILayer();
    if (!ui)
        return;

    ui->SetVisible(true);
    ui->ExecuteScript(HudBridgeScript::SetChatActive(true));
}

void ChatHud::DeactivateChat()
{
    if (!m_chatActive)
        return;

    m_chatActive = false;
    if (m_inputManager)
    {
        m_inputManager->SetEnabled(true);
    }
    DisableCursor();

    if (!m_screenManager)
        return;

    auto *ui = m_screenManager->GetUILayer();
    if (!ui)
        return;

    ui->ExecuteScript(HudBridgeScript::SetChatActive(false));
}

std::string ChatHud::EscapeForJsStringLiteral(const std::string &text)
{
    std::string escaped;
    escaped.reserve(text.size() + 8);
    escaped += '"';
    for (char c : text)
    {
        if (c == '\\')
            escaped += "\\\\";
        else if (c == '"')
            escaped += "\\\"";
        else if (c == '\n')
            escaped += "\\n";
        else if (c == '\r')
            escaped += "\\r";
        else
            escaped += c;
    }
    escaped += '"';
    return escaped;
}

void ChatHud::PollChatUI()
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        m_skipExitThisFrame = true;
        DeactivateChat();
        return;
    }

    if (!IsKeyPressed(KEY_ENTER))
        return;
    if (!m_screenManager)
        return;

    auto *ui = m_screenManager->GetUILayer();
    if (!ui)
        return;

    std::string text = ui->GetAppState("chatInputText");
    ui->ExecuteScript(HudBridgeScript::ClearChatInput());

    if (text.empty())
        return;

    const std::string escaped = EscapeForJsStringLiteral(text);
    std::string script = HudBridgeScript::EnqueueChat(escaped);
    ui->ExecuteScript(script);
}
