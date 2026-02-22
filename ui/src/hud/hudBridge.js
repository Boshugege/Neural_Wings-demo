const CHAT_QUEUE_MAX = 128;

function ensureAppState() {
  window.vueAppState = window.vueAppState || {};
  return window.vueAppState;
}

export function setChatActiveState(active) {
  const state = ensureAppState();
  state.chatActive = !!active;
}

export function setChatInputText(text) {
  const state = ensureAppState();
  state.chatInputText = String(text ?? "");
}

export function clearChatQueue() {
  const state = ensureAppState();
  state.chatSendQueue = [];
}

export function clearLegacyChatSend() {
  const state = ensureAppState();
  state.chatSendRequested = false;
  state.chatSendText = "";
}

export function resetChatState() {
  const state = ensureAppState();
  state.chatMessages = [];
  clearLegacyChatSend();
  clearChatQueue();
  setChatActiveState(false);
  setChatInputText("");

  if (window.__NW_CHAT_DEACTIVATE__) {
    window.__NW_CHAT_DEACTIVATE__();
  } else if (window.__NW_CHAT_CLEAR_INPUT__) {
    window.__NW_CHAT_CLEAR_INPUT__();
  }
}

export function setChatActive(active) {
  setChatActiveState(active);

  if (active) {
    if (window.__NW_CHAT_ACTIVATE__) window.__NW_CHAT_ACTIVATE__();
    return;
  }
  if (window.__NW_CHAT_DEACTIVATE__) window.__NW_CHAT_DEACTIVATE__();
}

export function enqueueChatSend(text) {
  const state = ensureAppState();
  const payload = String(text ?? "").trim();
  if (!payload) {
    return false;
  }

  if (!Array.isArray(state.chatSendQueue)) {
    clearChatQueue();
  }
  if (state.chatSendQueue.length >= CHAT_QUEUE_MAX) {
    return false;
  }

  state.chatSendQueue.push(payload);
  return true;
}

export function registerHudBridge() {
  window.__NW_HUD_RESET_CHAT__ = resetChatState;
  window.__NW_HUD_SET_CHAT_ACTIVE__ = setChatActive;
  window.__NW_HUD_ENQUEUE_CHAT__ = enqueueChatSend;
  window.__NW_HUD_CLEAR_CHAT_QUEUE__ = clearChatQueue;
  window.__NW_HUD_CLEAR_LEGACY_CHAT_SEND__ = clearLegacyChatSend;
}

export function unregisterHudBridge() {
  delete window.__NW_HUD_RESET_CHAT__;
  delete window.__NW_HUD_SET_CHAT_ACTIVE__;
  delete window.__NW_HUD_ENQUEUE_CHAT__;
  delete window.__NW_HUD_CLEAR_CHAT_QUEUE__;
  delete window.__NW_HUD_CLEAR_LEGACY_CHAT_SEND__;
}
