<script setup>
import { onBeforeUnmount, onMounted } from "vue";
import ChatHUD from "./ChatHUD.vue";
import { registerHudBridge, unregisterHudBridge } from "./hudBridge";

// Centralized HUD registry. New gameplay overlays should be added here.
const hudRegistry = [
  { id: "chat", component: ChatHUD },
];

onMounted(() => {
  registerHudBridge();
});

onBeforeUnmount(() => {
  unregisterHudBridge();
});
</script>

<template>
  <component
    v-for="hud in hudRegistry"
    :key="hud.id"
    :is="hud.component"
  />
</template>
