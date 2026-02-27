<script setup>
defineProps({
  fullscreen: {
    default: null,
  },
  selectedResolution: {
    type: String,
    default: "",
  },
  isResolutionOpen: {
    type: Boolean,
    default: false,
  },
  targetFPS: {
    default: null,
  },
  serverIP: {
    type: String,
    default: "",
  },
  serverStatus: {
    type: String,
    default: "",
  },
  nickname: {
    type: String,
    default: "",
  },
  nicknameStatus: {
    type: String,
    default: "",
  },
  resolutions: {
    type: Array,
    default: () => [],
  },
  toggleFullscreen: {
    type: Function,
    required: true,
  },
  toggleResolution: {
    type: Function,
    required: true,
  },
  chooseResolution: {
    type: Function,
    required: true,
  },
  changeFPS: {
    type: Function,
    required: true,
  },
  changeServerIP: {
    type: Function,
    required: true,
  },
  changeNickname: {
    type: Function,
    required: true,
  },
  applyNickname: {
    type: Function,
    required: true,
  },
  checkServer: {
    type: Function,
    required: true,
  },
  saveSettings: {
    type: Function,
    required: true,
  },
});
</script>

<template>
  <section class="menu-full settings-full pixel-font">
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
              <input class="fps-slider" type="range" :value="targetFPS === null ? '' : targetFPS"
                @input="changeFPS(Number($event.target.value))" @change="changeFPS(Number($event.target.value))"
                min="30" max="240" step="1" :disabled="targetFPS === null" />
              <span class="chip fps-value">
                {{ targetFPS === null ? "--" : targetFPS }} FPS
              </span>
            </div>
          </div>
        </div>

        <div class="row">
          <span>Server</span>
          <div class="settings-control">
            <div class="server-ip-control">
              <input class="chip server-ip-input" type="text" :value="serverIP"
                @input="changeServerIP($event.target.value)"
                placeholder="127.0.0.1" />
              <button class="btn small server-check-btn"
                :class="{ 'status-online': serverStatus === 'online', 'status-offline': serverStatus === 'offline', 'status-checking': serverStatus === 'checking' }"
                @click="checkServer"
                :disabled="serverStatus === 'checking'">
                {{ serverStatus === 'checking' ? 'Checking...' : serverStatus === 'online' ? 'Online' : serverStatus === 'offline' ? 'Offline' : 'Check' }}
              </button>
            </div>
          </div>
        </div>

        <div class="row">
          <span>Nickname</span>
          <div class="settings-control">
            <div class="server-ip-control">
              <input class="chip server-ip-input" type="text" :value="nickname" @input="changeNickname($event.target.value)"
                placeholder="Enter nickname" maxlength="16" />
              <button class="btn small server-check-btn"
                :class="{ 'status-online': nicknameStatus === 'accepted', 'status-offline': nicknameStatus === 'conflict' || nicknameStatus === 'invalid' || nicknameStatus === 'offline' || nicknameStatus === 'failed', 'status-checking': nicknameStatus === 'submitting' || nicknameStatus === 'fetching' }"
                @click="applyNickname" :disabled="nicknameStatus === 'submitting' || nicknameStatus === 'fetching'">
                {{ nicknameStatus === 'fetching' ? 'Loading...' : nicknameStatus === 'submitting' ? 'Applying...' : nicknameStatus === 'accepted' ? 'Accepted' : nicknameStatus === 'conflict' ? 'Conflict' : nicknameStatus === 'invalid' ? 'Invalid' : nicknameStatus === 'offline' ? 'Offline' : nicknameStatus === 'failed' ? 'Failed' : 'Apply' }}
              </button>
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
</template>
