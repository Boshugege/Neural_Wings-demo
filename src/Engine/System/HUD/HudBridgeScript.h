#pragma once
#include <string>

namespace HudBridgeScript
{
    inline const char *ResetChatState()
    {
        return "if (window.__NW_HUD_RESET_CHAT__) window.__NW_HUD_RESET_CHAT__();"
               "else {"
               "window.vueAppState = window.vueAppState || {};"
               "window.vueAppState.chatMessages = [];"
               "window.vueAppState.chatSendRequested = false;"
               "window.vueAppState.chatSendText = '';"
               "window.vueAppState.chatSendQueue = [];"
               "window.vueAppState.chatActive = false;"
               "window.vueAppState.chatInputText = '';"
               "if (window.__NW_CHAT_DEACTIVATE__) window.__NW_CHAT_DEACTIVATE__();"
               "}";
    }

    inline const char *SetChatActive(bool active)
    {
        return active
                   ? "if (window.__NW_HUD_SET_CHAT_ACTIVE__) window.__NW_HUD_SET_CHAT_ACTIVE__(true);"
                     "else {"
                     "window.vueAppState = window.vueAppState || {};"
                     "window.vueAppState.chatActive = true;"
                     "if (window.__NW_CHAT_ACTIVATE__) window.__NW_CHAT_ACTIVATE__();"
                     "}"
                   : "if (window.__NW_HUD_SET_CHAT_ACTIVE__) window.__NW_HUD_SET_CHAT_ACTIVE__(false);"
                     "else {"
                     "window.vueAppState = window.vueAppState || {};"
                     "window.vueAppState.chatActive = false;"
                     "if (window.__NW_CHAT_DEACTIVATE__) window.__NW_CHAT_DEACTIVATE__();"
                     "}";
    }

    inline const char *ClearChatInput()
    {
        return "if (window.__NW_CHAT_CLEAR_INPUT__) window.__NW_CHAT_CLEAR_INPUT__();"
               "else if (window.vueAppState) window.vueAppState.chatInputText = '';";
    }

    inline std::string EnqueueChat(const std::string &escapedTextLiteral)
    {
        return "if (window.__NW_HUD_ENQUEUE_CHAT__) window.__NW_HUD_ENQUEUE_CHAT__(" + escapedTextLiteral + ");"
                                                                                                         "else {"
                                                                                                         "window.vueAppState = window.vueAppState || {};"
                                                                                                         "if (!Array.isArray(window.vueAppState.chatSendQueue))"
                                                                                                         "  window.vueAppState.chatSendQueue = [];"
                                                                                                         "window.vueAppState.chatSendQueue.push(" +
               escapedTextLiteral + ");"
                                    "}";
    }

    inline const char *ClearChatQueue()
    {
        return "if (window.__NW_HUD_CLEAR_CHAT_QUEUE__) window.__NW_HUD_CLEAR_CHAT_QUEUE__();"
               "else {"
               "window.vueAppState = window.vueAppState || {};"
               "window.vueAppState.chatSendQueue = [];"
               "}";
    }

    inline const char *ClearLegacyChatSend()
    {
        return "if (window.__NW_HUD_CLEAR_LEGACY_CHAT_SEND__) window.__NW_HUD_CLEAR_LEGACY_CHAT_SEND__();"
               "else {"
               "window.vueAppState = window.vueAppState || {};"
               "window.vueAppState.chatSendRequested = false;"
               "window.vueAppState.chatSendText = '';"
               "}";
    }
} // namespace HudBridgeScript
