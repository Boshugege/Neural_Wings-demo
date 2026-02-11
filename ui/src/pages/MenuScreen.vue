<script setup>
defineProps({});

function requestStartGame() {
  window.vueAppState = window.vueAppState || {};
  window.vueAppState.nextScreen = "gameplay";
}
</script>

<template>
  <section class="menu-full pixel-font">
    <h1 class="splash-title big">Neural Wings</h1>
    <div class="actions">
      <button class="btn large" type="button" @click="requestStartGame">
        Start Game
      </button>
      <a class="btn large" href="#/options">Settings</a>
    </div>
  </section>
</template>
