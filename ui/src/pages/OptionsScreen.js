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
  checkServer: {
    type: Function,
    required: true,
  },
  saveSettings: {
    type: Function,
    required: true,
  },
});
