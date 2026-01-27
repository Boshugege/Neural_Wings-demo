<script setup>
import { computed, onBeforeUnmount, onMounted, ref, watch } from "vue";

// Global state for C++ communication
window.vueAppState = window.vueAppState || {};
window.vueAppState.vueAppReady = false;
window.vueAppState.settingsSaveRequested = false;

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
  <section v-if="isStart" class="splash-only">
    <h1 class="splash-title splash-typewriter">{{ typedStartTitle }}</h1>
  </section>

  <div v-else class="root">
    <header v-if="!isMenu && !isOptions" class="topbar pixel-font">
      <div class="brand">Neural Wings</div>
      <nav class="menu-top">
        <a class="btn small nav-link" href="#/menu">Menu</a>
        <a class="btn small nav-link" href="#/options">Options</a>
      </nav>
    </header>

    <main class="content">
      <section v-if="isMenu" class="menu-full pixel-font">
        <h1 class="splash-title big">Neural Wings</h1>
        <div class="actions">
          <a class="btn large" href="#/gameplay">Start Game</a>
          <a class="btn large" href="#/options">Settings</a>
        </div>
      </section>

      <section v-else-if="isOptions" class="menu-full settings-full pixel-font">
        <h1 class="splash-title">Game Settings</h1>
        <div class="settings-panel">
          <div class="form">
            <div class="row">
              <span>Fullscreen</span>
              <div class="settings-control">
                <button class="chip control-chip" @click="toggleFullscreen" :class="{ active: fullscreen === true }">
                  {{ fullscreen === null ? "Not Set" : fullscreen ? "On" : "Off" }}
                </button>
              </div>
            </div>

            <div class="row">
              <span>Resolution</span>
              <div class="settings-control">
                <div class="dropdown" @click.stop>
                  <button class="chip dropdown-toggle" @click="toggleResolution">
                    {{ selectedResolution || "Select" }}
                    <span class="caret"></span>
                  </button>
                  <div v-if="isResolutionOpen" class="dropdown-menu">
                    <button v-for="res in resolutions" :key="res.value" class="dropdown-item"
                      @click="chooseResolution(res.value)">
                      {{ res.label }}
                    </button>
                  </div>
                </div>
              </div>
            </div>

            <div class="row">
              <span>FPS</span>
              <div class="settings-control">
                <div class="fps-control">
                  <input
                    class="fps-slider"
                    type="range"
                    v-model.number="targetFPS"
                    @change="changeFPS(targetFPS)"
                    min="30"
                    max="240"
                    step="1"
                    :disabled="targetFPS === null"
                  />
                  <span class="chip fps-value">
                    {{ targetFPS === null ? "--" : targetFPS }} FPS
                  </span>
                </div>
              </div>
            </div>
          </div>

          <div class="actions settings-actions">
            <button class="btn large primary" @click="saveSettings">Save Settings</button>
            <a class="btn large secondary" href="#/menu">Back to Menu</a>
          </div>
        </div>
      </section>

      <section v-else class="card">
        <h1>Page Not Found</h1>
        <a class="btn" href="#/start">Back to Start</a>
      </section>
    </main>
  </div>
</template>
