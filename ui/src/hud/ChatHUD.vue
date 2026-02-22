<script setup>
import { ref, nextTick, onMounted, onBeforeUnmount } from "vue";
import {
  enqueueChatSend,
  resetChatState,
  setChatActiveState,
  setChatInputText,
} from "./hudBridge";

const messages = ref([]);
const inputText = ref("");
const isActive = ref(false);
const chatChannelMode = ref("public");
const chatWhisperTarget = ref("");
const gameplayMessageVisible = ref(false);
const inputRef = ref(null);
const messagesEndRef = ref(null);
const route = ref(window.location.hash || "");
const GAMEPLAY_MESSAGE_VISIBLE_MS = 5000;
let gameplayMessageHideTimer = null;

function syncRoute() {
  route.value = window.location.hash || "";
  if (!isVisibleRoute() && isActive.value) {
    deactivate();
  }
  if (!isGameplayRoute()) {
    gameplayMessageVisible.value = false;
    if (gameplayMessageHideTimer) {
      clearTimeout(gameplayMessageHideTimer);
      gameplayMessageHideTimer = null;
    }
  }
}

function isVisibleRoute() {
  const r = route.value;
  return (
    r.startsWith("#/menu") ||
    r.startsWith("#/gameplay")
  );
}

function isGameplayRoute() {
  return route.value.startsWith("#/gameplay");
}

function shouldHandleLocalHotkeys() {
  return route.value.startsWith("#/menu") || route.value.startsWith("#/options");
}

function scrollToBottom() {
  nextTick(() => {
    if (messagesEndRef.value) {
      messagesEndRef.value.scrollIntoView({ behavior: "auto" });
    }
  });
}

function showGameplayMessagesTemporarily() {
  if (!isGameplayRoute()) {
    return;
  }
  gameplayMessageVisible.value = true;
  if (gameplayMessageHideTimer) {
    clearTimeout(gameplayMessageHideTimer);
    gameplayMessageHideTimer = null;
  }
  gameplayMessageHideTimer = setTimeout(() => {
    gameplayMessageVisible.value = false;
    gameplayMessageHideTimer = null;
  }, GAMEPLAY_MESSAGE_VISIBLE_MS);
}

function applyChatModeMarker(msg) {
  if (!msg || typeof msg !== "object") {
    return msg;
  }
  if (msg.type !== "system" || typeof msg.text !== "string") {
    return msg;
  }

  const match = msg.text.match(/^\[CHAT_MODE:(PUBLIC|WHISPER)(?::([^\]]*))?\]\s*(.*)$/);
  if (!match) {
    return msg;
  }

  if (match[1] === "WHISPER") {
    chatChannelMode.value = "whisper";
    chatWhisperTarget.value = (match[2] || "").trim();
  } else {
    chatChannelMode.value = "public";
    chatWhisperTarget.value = "";
  }

  const plainText = (match[3] || "").trim();
  if (!plainText) {
    return null;
  }
  return { ...msg, text: plainText };
}

function pushMessage(msg) {
  const normalized = applyChatModeMarker(msg);
  if (!normalized) {
    return;
  }
  messages.value.push(normalized);
  if (!isActive.value) {
    showGameplayMessagesTemporarily();
  }
  if (messages.value.length > 300) {
    messages.value.shift();
  }
  scrollToBottom();
}

function pushBatch(batch) {
  if (!Array.isArray(batch) || batch.length === 0) {
    return;
  }
  for (const msg of batch) {
    const normalized = applyChatModeMarker(msg);
    if (normalized) {
      messages.value.push(normalized);
    }
  }
  if (!isActive.value) {
    showGameplayMessagesTemporarily();
  }
  if (messages.value.length > 300) {
    messages.value.splice(0, messages.value.length - 300);
  }
  scrollToBottom();
}

function activate() {
  isActive.value = true;
  setChatActiveState(true);
  nextTick(() => {
    if (inputRef.value) {
      inputRef.value.focus();
    }
  });
}

function deactivate() {
  isActive.value = false;
  inputText.value = "";
  setChatActiveState(false);
}

function clearInput() {
  inputText.value = "";
  syncInputToAppState();
}

function syncInputToAppState() {
  setChatInputText(inputText.value.trim());
}

function onInput() {
  // Keep vueAppState in sync whenever the user types
  syncInputToAppState();
}

/**
 * Push a message directly into the JS-side send queue (an array on
 * vueAppState). The C++ side drains this array atomically each frame,
 * so no messages can be lost between read and clear.
 */
function enqueueSend(text) {
  enqueueChatSend(text);
}

function isEnterKey(e) {
  return (
    e.key === "Enter" ||
    e.code === "Enter" ||
    e.code === "NumpadEnter" ||
    e.keyCode === 13
  );
}

function isEscapeKey(e) {
  return (
    e.key === "Escape" ||
    e.key === "Esc" ||
    e.code === "Escape" ||
    e.keyCode === 27
  );
}

function onInputKeydown(e) {
  const enter = isEnterKey(e);
  const escape = isEscapeKey(e);
  if (!enter && !escape) {
    return;
  }

  e.preventDefault();
  e.stopPropagation();

  if (shouldHandleLocalHotkeys()) {
    if (enter) {
      submitInput();
      return;
    }
    onEscapePressed();
  }
}

function submitInput() {
  const text = inputText.value.trim();
  if (!text) {
    return;
  }

  enqueueSend(text);
  clearInput();
}

function onEscapePressed() {
  deactivate();
}

function onGlobalKeydown(e) {
  if (!isVisibleRoute() || !shouldHandleLocalHotkeys()) {
    return;
  }

  if (isEnterKey(e)) {
    e.preventDefault();
    e.stopPropagation();

    if (!isActive.value) {
      activate();
      return;
    }
    submitInput();
    return;
  }

  if (isEscapeKey(e) && isActive.value) {
    e.preventDefault();
    e.stopPropagation();
    onEscapePressed();
  }
}

function getTypeClass(type) {
  switch (type) {
    case "system":
      return "msg-system";
    case "whisper":
      return "msg-whisper";
    default:
      return "msg-public";
  }
}

function getTypeLabel(type) {
  switch (type) {
    case "system":
      return "[SYSTEM]";
    case "whisper":
      return "[WHISPER]";
    default:
      return "";
  }
}

function getChannelStatusText() {
  if (chatChannelMode.value === "whisper") {
    if (chatWhisperTarget.value) {
      return `Whisper ${chatWhisperTarget.value}`;
    }
    return "Whisper";
  }
  return "Public";
}

function getInputPlaceholder() {
  if (chatChannelMode.value === "whisper") {
    if (chatWhisperTarget.value) {
      return `Whisper to ${chatWhisperTarget.value} (/a to public, /help for commands)`;
    }
    return "Whisper mode (/a to public, /help for commands)";
  }
  return "Public chat (/w <nickname> to whisper, /help for commands)";
}

onMounted(() => {
  // Register global hooks for C++ to call
  window.__NW_CHAT_PUSH__ = pushMessage;
  window.__NW_CHAT_PUSH_BATCH__ = pushBatch;
  window.__NW_CHAT_ACTIVATE__ = activate;
  window.__NW_CHAT_DEACTIVATE__ = deactivate;
  window.__NW_CHAT_CLEAR_INPUT__ = clearInput;

  // Init shared chat state via HUD bridge.
  resetChatState();
  syncRoute();
  window.addEventListener("hashchange", syncRoute);
  window.addEventListener("keydown", onGlobalKeydown);
});

onBeforeUnmount(() => {
  delete window.__NW_CHAT_PUSH__;
  delete window.__NW_CHAT_PUSH_BATCH__;
  delete window.__NW_CHAT_ACTIVATE__;
  delete window.__NW_CHAT_DEACTIVATE__;
  delete window.__NW_CHAT_CLEAR_INPUT__;
  if (gameplayMessageHideTimer) {
    clearTimeout(gameplayMessageHideTimer);
    gameplayMessageHideTimer = null;
  }
  window.removeEventListener("hashchange", syncRoute);
  window.removeEventListener("keydown", onGlobalKeydown);
});
</script>

<template>
  <div v-if="isVisibleRoute()" class="chat-hud" :class="{ active: isActive, gameplay: isGameplayRoute() }">
    <!-- Message history (always visible when active; in gameplay, auto-show for a short time on new messages) -->
    <div v-if="isActive || (isGameplayRoute() && gameplayMessageVisible)" class="chat-messages" :class="{ faded: !isActive && !isGameplayRoute() }">
      <div v-for="(msg, idx) in messages" :key="idx" class="chat-line" :class="getTypeClass(msg.type)">
        <span v-if="getTypeLabel(msg.type)" class="chat-type-label">{{
          getTypeLabel(msg.type)
        }}</span>
        <span v-if="msg.senderName && msg.type !== 'system'" class="chat-sender">{{ msg.senderName }}:</span>
        <span class="chat-text">{{ msg.text }}</span>
      </div>
      <div ref="messagesEndRef"></div>
    </div>

    <!-- Input bar (only when active) -->
    <div v-if="isActive" class="chat-input-bar">
      <div class="chat-input-row">
        <div class="chat-channel-status" :class="{ whisper: chatChannelMode === 'whisper' }">
          {{ getChannelStatusText() }}
        </div>
        <input ref="inputRef" v-model="inputText" type="text" class="chat-input" :placeholder="getInputPlaceholder()"
          maxlength="256" @keydown="onInputKeydown" @input="onInput" />
      </div>
    </div>
  </div>
</template>

<style scoped>
.chat-hud {
  position: fixed;
  bottom: 48px;
  left: 20px;
  width: 640px;
  max-width: 68vw;
  z-index: 9999;
  pointer-events: none;
  font-family: "Consolas", "Courier New", monospace;
  font-size: 16px;
}

.chat-hud.active {
  pointer-events: auto;
}

.chat-messages {
  max-height: 320px;
  overflow-y: auto;
  padding: 10px 12px;
  display: flex;
  flex-direction: column;
  gap: 6px;
  /* Hide scrollbar for cleaner look */
  scrollbar-width: thin;
  scrollbar-color: rgba(255, 255, 255, 0.15) transparent;
}

.chat-messages.faded {
  opacity: 0.5;
}

.chat-line {
  padding: 7px 10px;
  border-radius: 6px;
  background: rgba(0, 0, 0, 0.55);
  overflow-wrap: break-word;
  line-height: 1.5;
}

.chat-hud.gameplay .chat-line {
  background: rgba(0, 0, 0, 0.18);
}

.msg-system {
  color: #ffd740;
}

.msg-public {
  color: #dfe6dd;
}

.msg-whisper {
  color: #b388ff;
}

.chat-type-label {
  font-weight: bold;
  margin-right: 8px;
  font-size: 13px;
}

.chat-sender {
  font-weight: bold;
  margin-right: 8px;
  color: #8aff7f;
}

.msg-whisper .chat-sender {
  color: #ce93d8;
}

.msg-system .chat-sender {
  color: #ffd740;
}

.chat-text {
  /* Preserve server-sent line breaks such as /help multi-line output. */
  white-space: pre-wrap;
}

.chat-input-bar {
  margin-top: 8px;
}

.chat-input-row {
  display: flex;
  align-items: center;
  gap: 8px;
}

.chat-channel-status {
  flex: 0 0 auto;
  max-width: 40%;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  padding: 4px 8px;
  border-radius: 6px;
  background: rgba(0, 0, 0, 0.55);
  border: 1px solid rgba(138, 255, 127, 0.35);
  color: #8aff7f;
  font-size: 12px;
  letter-spacing: 0.3px;
}

.chat-channel-status.whisper {
  border-color: rgba(206, 147, 216, 0.55);
  color: #ce93d8;
}

.chat-input {
  width: auto;
  flex: 1 1 auto;
  padding: 12px 14px;
  background: rgba(0, 0, 0, 0.75);
  border: 1px solid rgba(138, 255, 127, 0.4);
  border-radius: 8px;
  color: #dfe6dd;
  font-family: inherit;
  font-size: 16px;
  outline: none;
}

.chat-hud.gameplay .chat-input {
  background: rgba(0, 0, 0, 0.28);
}

.chat-input:focus {
  border-color: rgba(138, 255, 127, 0.8);
  box-shadow: 0 0 6px rgba(138, 255, 127, 0.2);
}

.chat-input::placeholder {
  color: rgba(255, 255, 255, 0.3);
}

@media (max-width: 960px) {
  .chat-hud {
    left: 12px;
    right: 12px;
    bottom: 16px;
    width: auto;
    max-width: none;
    font-size: 14px;
  }

  .chat-messages {
    max-height: 220px;
    padding: 8px 10px;
    gap: 4px;
  }

  .chat-line {
    padding: 6px 8px;
  }

  .chat-input {
    padding: 10px 12px;
    font-size: 14px;
  }
}
</style>
