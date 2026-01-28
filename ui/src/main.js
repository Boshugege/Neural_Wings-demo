import { createApp } from "vue";
import App from "./App.vue";
import "./style.css";

const root = document.getElementById("app");


function setFallback(title, message) {
  if (!root) {
    return;
  }

  root.innerHTML =
    '<div class="fallback">' +
    '<div class="fallback-title">' +
    title +
    "</div>" +
    '<div class="fallback-message">' +
    message +
    "</div>" +
    "</div>";
}

// Allow the engine to send settings even before Vue is ready.
window.__applyEngineSettings = (settings) => {
  window.__pendingEngineSettings = settings;
};

if (!root) {
  console.error("UI root element not found.");
} else {
  try {
    const app = createApp(App);

    app.config.errorHandler = (error) => {
      console.error(error);
      setFallback(
        "UI Initialization Failed",
        error && error.message ? error.message : String(error),
      );
      window.__NW_UI_ERROR__ = error;
    };

    app.mount(root);
    window.__NW_UI_MOUNTED__ = true;
  } catch (error) {
    console.error(error);
    setFallback(
      "UI Initialization Failed",
      error && error.message ? error.message : String(error),
    );
    window.__NW_UI_ERROR__ = error;
  }
}
