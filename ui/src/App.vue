<script setup>
import { computed, onBeforeUnmount, onMounted, ref, watch } from "vue";
import MenuScreen from "./pages/MenuScreen.vue";
import NotFoundScreen from "./pages/NotFoundScreen.vue";
import OptionsScreen from "./pages/OptionsScreen.vue";
import StartScreen from "./pages/StartScreen.vue";

// Global state for C++ communication
window.vueAppState = window.vueAppState || {};
window.vueAppState.vueAppReady = false;
window.vueAppState.settingsSaveRequested = false;
window.vueAppState.nextScreen = window.vueAppState.nextScreen || "";

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

const isStart = computed(() => route.value.startsWith("#/start"));
const isMenu = computed(() => route.value.startsWith("#/menu"));
const isOptions = computed(() => route.value.startsWith("#/options"));

const startTitleFull = "Neural Wings";
const typedStartTitle = ref("");
let typingTimerId = null;

function syncRoute() {
  route.value = window.location.hash || "#/start";
  window.vueAppState.currentRoute = route.value;
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
  window.vueAppState.settingsSaveRequested = true;

  console.log("Settings save requested:", {
    fullscreen: fullscreen.value,
    resolution: selectedResolution.value,
    fps: targetFPS.value,
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

onMounted(() => {
  window.addEventListener("hashchange", syncRoute);
  document.addEventListener("click", handleDocumentClick);

  syncRoute();
  if (isStart.value) {
    startStartTyping();
  }

  // Replace the early stub from main.js with the real handler.
  window.__applyEngineSettings = applySettings;

  // Apply settings that may have arrived before mount.
  if (window.__pendingEngineSettings) {
    applySettings(window.__pendingEngineSettings);
    window.__pendingEngineSettings = undefined;
  }

  window.vueAppState.vueAppReady = true;
});

onBeforeUnmount(() => {
  stopStartTyping();
  window.removeEventListener("hashchange", syncRoute);
  document.removeEventListener("click", handleDocumentClick);
});
</script>

<template>
  <StartScreen v-if="isStart" :typedStartTitle="typedStartTitle" />

  <div v-else class="root">
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
        :resolutions="resolutions"
        :toggleFullscreen="toggleFullscreen"
        :toggleResolution="toggleResolution"
        :chooseResolution="chooseResolution"
        :changeFPS="changeFPS"
        :saveSettings="saveSettings"
      />

      <NotFoundScreen v-else />
    </main>
  </div>
</template>
