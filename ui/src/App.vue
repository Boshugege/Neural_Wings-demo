<script setup>
import { computed, onBeforeUnmount, onMounted, ref, watch } from "vue";
import MenuScreen from "./pages/MenuScreen.vue";
import NotFoundScreen from "./pages/NotFoundScreen.vue";
import OptionsScreen from "./pages/OptionsScreen.vue";
import StartScreen from "./pages/StartScreen.vue";
import HudRoot from "./hud/HudRoot.vue";

// Global state for C++ communication
window.vueAppState = window.vueAppState || {};
window.vueAppState.vueAppReady = false;
window.vueAppState.settingsSaveRequested = false;
window.vueAppState.nextScreen = window.vueAppState.nextScreen || "";
window.vueAppState.nicknameApplyRequested = false;
window.vueAppState.nicknameStatus = window.vueAppState.nicknameStatus || "";

const resolutions = [
  { label: "1280x720", value: "1280x720" },
  { label: "1600x900", value: "1600x900" },
  { label: "1920x1080", value: "1920x1080" },
  { label: "2560x1440", value: "2560x1440" },
];

const route = ref(window.location.hash || "#/start");
const fullscreen = ref(null);
const selectedResolution = ref("");
const isResolutionOpen = ref(false);
const targetFPS = ref(null);
const serverIP = ref("");
const serverStatus = ref("");
const nickname = ref("");
const nicknameStatus = ref("");

const isStart = computed(() => route.value.startsWith("#/start"));
const isMenu = computed(() => route.value.startsWith("#/menu"));
const isOptions = computed(() => route.value.startsWith("#/options"));
const isGameplay = computed(() => route.value.startsWith("#/gameplay"));

const startTitleFull = "Neural Wings";
const typedStartTitle = ref("");
let typingTimerId = null;
let nicknameRequestInFlight = false;

function syncGameplayBodyClass() {
  if (route.value.startsWith("#/gameplay")) {
    document.body.classList.add("gameplay-overlay");
    return;
  }
  document.body.classList.remove("gameplay-overlay");
}

function syncRoute() {
  route.value = window.location.hash || "#/start";
  window.vueAppState.currentRoute = route.value;
  syncGameplayBodyClass();
}

function syncAppStateToWindow() {
  if (fullscreen.value !== null) {
    window.vueAppState.fullscreen = fullscreen.value;
  }
  if (selectedResolution.value) {
    window.vueAppState.resolution = selectedResolution.value;
  }
  if (targetFPS.value !== null) {
    window.vueAppState.targetFPS = targetFPS.value;
  }
  if (serverIP.value) {
    window.vueAppState.serverIP = serverIP.value;
  }
  window.vueAppState.nickname = nickname.value;
}

function applySettings(settings) {
  const state = settings || {};

  if (typeof state.fullscreen === "boolean") {
    fullscreen.value = state.fullscreen;
  }

  if (typeof state.resolution === "string" && state.resolution) {
    const exists = resolutions.some((r) => r.value === state.resolution);
    if (exists) {
      selectedResolution.value = state.resolution;
    }
  }

  if (state.targetFPS !== undefined && state.targetFPS !== null) {
    const fps = Number(state.targetFPS);
    if (!Number.isNaN(fps)) {
      targetFPS.value = fps;
    }
  }

  if (typeof state.serverIP === "string" && state.serverIP) {
    serverIP.value = state.serverIP;
  }
  if (typeof state.nickname === "string") {
    nickname.value = state.nickname;
  }

  syncAppStateToWindow();
}

function toggleFullscreen() {
  if (fullscreen.value === null) {
    return;
  }
  fullscreen.value = !fullscreen.value;
}

function toggleResolution() {
  isResolutionOpen.value = !isResolutionOpen.value;
}

function chooseResolution(value) {
  selectedResolution.value = value;
  isResolutionOpen.value = false;
}

function changeFPS(value) {
  targetFPS.value = value;
}

function changeServerIP(value) {
  serverIP.value = value;
}

function changeNickname(value) {
  nickname.value = value;
}

function syncNicknameStateFromWindow() {
  const state = window.vueAppState || {};

  if (typeof state.nicknameStatus === "string") {
    nicknameStatus.value = state.nicknameStatus;
  }
  if (typeof state.nicknameServerName === "string" && state.nicknameServerName) {
    nickname.value = state.nicknameServerName;
    state.nickname = state.nicknameServerName;
  }
}

function applyNicknameStateFromEngine(payload) {
  if (!payload || typeof payload !== "object") {
    return;
  }
  const status = typeof payload.status === "string" ? payload.status : "";
  if (status === "fetching" || status === "submitting") {
    nicknameRequestInFlight = true;
  }
  if (typeof payload.status === "string") {
    nicknameStatus.value = payload.status;
    window.vueAppState.nicknameStatus = payload.status;
  }
  if (
    typeof payload.nickname === "string" &&
    payload.nickname &&
    (nicknameRequestInFlight || !nickname.value.trim())
  ) {
    nickname.value = payload.nickname;
    window.vueAppState.nickname = payload.nickname;
    window.vueAppState.nicknameServerName = payload.nickname;
  }
  if (
    status === "accepted" ||
    status === "conflict" ||
    status === "invalid" ||
    status === "offline" ||
    status === "failed"
  ) {
    nicknameRequestInFlight = false;
  }
}

function applyNickname() {
  const nick = nickname.value.trim();
  if (!nick) return;

  nickname.value = nick;
  nicknameStatus.value = "submitting";
  nicknameRequestInFlight = true;
  window.vueAppState.nickname = nick;
  window.vueAppState.nicknameStatus = "submitting";
  window.vueAppState.nicknameApplyRequested = true;

  const pollInterval = setInterval(() => {
    const status = window.vueAppState.nicknameStatus;
    if (
      status === "accepted" ||
      status === "conflict" ||
      status === "invalid" ||
      status === "offline" ||
      status === "failed"
    ) {
      nicknameStatus.value = status;
      if (window.vueAppState.nicknameServerName) {
        nickname.value = window.vueAppState.nicknameServerName;
      }
      clearInterval(pollInterval);
    }
  }, 150);

  setTimeout(() => {
    clearInterval(pollInterval);
    if (nicknameStatus.value === "submitting") {
      nicknameStatus.value = "offline";
      nicknameRequestInFlight = false;
    }
  }, 5000);
}

function checkServer() {
  if (!serverIP.value) return;
  serverStatus.value = "checking";
  window.vueAppState.serverIP = serverIP.value;
  window.vueAppState.serverStatus = "checking";
  window.vueAppState.serverCheckRequested = true;

  // Poll for serverStatus changes from the C++ engine
  const pollInterval = setInterval(() => {
    const status = window.vueAppState.serverStatus;
    if (status === "online" || status === "offline") {
      serverStatus.value = status;
      clearInterval(pollInterval);
    }
  }, 200);

  // Auto-clear after timeout
  setTimeout(() => {
    clearInterval(pollInterval);
    if (serverStatus.value === "checking") {
      serverStatus.value = "offline";
    }
  }, 5000);
}

function saveSettings() {
  if (
    fullscreen.value === null ||
    !selectedResolution.value ||
    targetFPS.value === null
  ) {
    return;
  }

  window.vueAppState.fullscreen = fullscreen.value;
  window.vueAppState.resolution = selectedResolution.value;
  window.vueAppState.targetFPS = targetFPS.value;
  window.vueAppState.serverIP = serverIP.value || "127.0.0.1";
  window.vueAppState.settingsSaveRequested = true;

  console.log("Settings save requested:", {
    fullscreen: fullscreen.value,
    resolution: selectedResolution.value,
    fps: targetFPS.value,
    serverIP: serverIP.value,
  });
}

function handleDocumentClick() {
  isResolutionOpen.value = false;
}

function stopStartTyping() {
  if (typingTimerId !== null) {
    window.clearInterval(typingTimerId);
    typingTimerId = null;
  }
}

function startStartTyping() {
  stopStartTyping();
  typedStartTitle.value = "";

  let index = 0;
  typingTimerId = window.setInterval(() => {
    index += 1;
    typedStartTitle.value = startTitleFull.slice(0, index);
    if (index >= startTitleFull.length) {
      stopStartTyping();
    }
  }, 90);
}

watch(isStart, (nowIsStart) => {
  if (nowIsStart) {
    startStartTyping();
    return;
  }
  stopStartTyping();
});

watch(isOptions, (nowIsOptions) => {
  if (nowIsOptions) {
    syncNicknameStateFromWindow();
  }
});

onMounted(() => {
  window.addEventListener("hashchange", syncRoute);
  document.addEventListener("click", handleDocumentClick);

  syncRoute();
  if (isStart.value) {
    startStartTyping();
  }
  syncGameplayBodyClass();

  // Replace the early stub from main.js with the real handler.
  window.__applyEngineSettings = applySettings;
  window.__NW_APPLY_NICKNAME_STATE__ = applyNicknameStateFromEngine;

  // Apply settings that may have arrived before mount.
  if (window.__pendingEngineSettings) {
    applySettings(window.__pendingEngineSettings);
    window.__pendingEngineSettings = undefined;
  }

  if (isOptions.value) {
    syncNicknameStateFromWindow();
  }
  window.vueAppState.vueAppReady = true;
});

onBeforeUnmount(() => {
  stopStartTyping();
  delete window.__NW_APPLY_NICKNAME_STATE__;
  document.body.classList.remove("gameplay-overlay");
  window.removeEventListener("hashchange", syncRoute);
  document.removeEventListener("click", handleDocumentClick);
});
</script>

<template>
  <StartScreen v-if="isStart" :typedStartTitle="typedStartTitle" />

  <div v-else-if="!isGameplay" class="root">
    <header v-if="!isMenu && !isOptions" class="topbar pixel-font">
      <div class="brand">Neural Wings</div>
      <nav class="menu-top">
        <a class="btn small nav-link" href="#/menu">Menu</a>
        <a class="btn small nav-link" href="#/options">Options</a>
      </nav>
    </header>

    <main class="content">
      <MenuScreen v-if="isMenu" />

      <OptionsScreen
        v-else-if="isOptions"
        :fullscreen="fullscreen"
        :selectedResolution="selectedResolution"
        :isResolutionOpen="isResolutionOpen"
        :targetFPS="targetFPS"
        :serverIP="serverIP"
        :serverStatus="serverStatus"
        :nickname="nickname"
        :nicknameStatus="nicknameStatus"
        :resolutions="resolutions"
        :toggleFullscreen="toggleFullscreen"
        :toggleResolution="toggleResolution"
        :chooseResolution="chooseResolution"
        :changeFPS="changeFPS"
        :changeServerIP="changeServerIP"
        :changeNickname="changeNickname"
        :applyNickname="applyNickname"
        :checkServer="checkServer"
        :saveSettings="saveSettings"
      />

      <NotFoundScreen v-else />
    </main>
  </div>

  <!-- Global HUD overlay root -->
  <HudRoot />
</template>
