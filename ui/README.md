# Neural Wings UI (Vue + Vite)

This UI is now a proper Vue 3 + Vite project that still builds into the engine-friendly shape expected by the Ultralight runtime:

- `ui/dist/index.html`
- `ui/dist/assets/app.js`
- `ui/dist/assets/style.css`

The build config is in `ui/vite.config.js`.

## Commands

From `ui/`:

1. Install deps: `npm ci`
2. Dev server: `npm run dev`
3. Production build: `npm run build`
4. Preview build: `npm run preview`

## Web integration (Emscripten)

- When building the Web target (`PLATFORM=Web` via `emcmake`), the top-level CMake tries to run `npm ci` and `npm run build` automatically (if `npm` is available) and will copy `ui/dist` into the web build output (`build_web/ui/dist`).
- If your build environment doesn't have `npm`, run the UI build manually from `ui/` before configuring/building the Web target:
  - `npm ci`
  - `npm run build`

### Typical Web build flow

1. Configure: `emcmake cmake -B build_web -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release -G "Ninja"`
2. Build: `cmake --build build_web` (this will invoke the UI build if `npm` is present)
3. Serve: `python -m http.server 8000 --directory build_web` (or use the workspace task "Start Web Server")

## Engine integration contract

The engine communicates with the UI through globals on `window`:

- `window.vueAppState`
- `window.__applyEngineSettings(settings)`

These are defined in `ui/src/App.vue` and `ui/src/main.js`.
